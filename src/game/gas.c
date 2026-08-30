#include <string.h>

#include "gas.h"

static int in_bounds(int file, int rank)
{
    return file >= 0 && file < 8 && rank >= 0 && rank < 8;
}

static CfPiece empty_piece(void)
{
    CfPiece piece;
    piece.type = CF_PIECE_NONE;
    piece.color = CF_COLOR_NONE;
    return piece;
}

static cf_u8 clamp_gas(unsigned value)
{
    return (cf_u8)(value > CF_GAS_MAX ? CF_GAS_MAX : value);
}

static int is_promotion_choice(CfPieceType type)
{
    return type == CF_PIECE_QUEEN || type == CF_PIECE_ROOK ||
           type == CF_PIECE_BISHOP || type == CF_PIECE_KNIGHT;
}

void gas_init(CfGasState *gas)
{
    if (gas == 0) return;
    memset(gas, 0, sizeof(*gas));
}

cf_u8 gas_at(const CfGasState *gas, int file, int rank)
{
    if (gas == 0 || !in_bounds(file, rank)) return 0;
    return gas->squares[rank][file];
}

void gas_set(CfGasState *gas, int file, int rank, cf_u8 value)
{
    if (gas == 0 || !in_bounds(file, rank)) return;
    gas->squares[rank][file] = clamp_gas(value);
}

static void apply_move_gas(CfGasState *gas, const CfGasState *before,
                           const CfMove *move)
{
    unsigned gain;
    int rank;
    int rook_from;
    int rook_to;

    *gas = *before;
    gas->squares[move->from_rank][move->from_file] = 0;
    if ((move->flags & CF_MOVE_EN_PASSANT) != 0U)
        gas->squares[move->captured_rank][move->captured_file] = 0;

    gain = move->captured.type == CF_PIECE_NONE ? 1U : 2U;
    gas->squares[move->to_rank][move->to_file] =
        clamp_gas((unsigned)before->squares[move->from_rank][move->from_file] + gain);

    if ((move->flags & CF_MOVE_CASTLE_KING) != 0U ||
        (move->flags & CF_MOVE_CASTLE_QUEEN) != 0U) {
        rank = move->from_rank;
        if ((move->flags & CF_MOVE_CASTLE_KING) != 0U) {
            rook_from = 7;
            rook_to = 5;
        } else {
            rook_from = 0;
            rook_to = 3;
        }
        gas->squares[rank][rook_to] =
            clamp_gas((unsigned)before->squares[rank][rook_from] + 1U);
        gas->squares[rank][rook_from] = 0;
    }
}

int gas_make_move_ex(CfBoard *board, CfGasState *gas,
                     int from_file, int from_rank, int to_file, int to_rank,
                     CfPieceType promotion, CfGasMove *made_move)
{
    CfGasMove local;
    if (board == 0 || gas == 0) return 0;
    local.previous_gas = *gas;
    if (!board_make_move_ex(board, from_file, from_rank, to_file, to_rank,
                            promotion, &local.chess_move)) return 0;
    apply_move_gas(gas, &local.previous_gas, &local.chess_move);
    if (made_move != 0) *made_move = local;
    return 1;
}

int gas_make_move(CfBoard *board, CfGasState *gas,
                  int from_file, int from_rank, int to_file, int to_rank,
                  CfGasMove *made_move)
{
    return gas_make_move_ex(board, gas, from_file, from_rank, to_file, to_rank,
                            CF_PIECE_QUEEN, made_move);
}

void gas_unmake_move(CfBoard *board, CfGasState *gas, const CfGasMove *move)
{
    if (board == 0 || gas == 0 || move == 0) return;
    board_unmake_move(board, &move->chess_move);
    *gas = move->previous_gas;
}

static void direction_delta(CfFartDirection direction, int *df, int *dr)
{
    static const int deltas[8][2] = {
        {0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}
    };
    int index = (int)direction;
    if (index < 0 || index > 7) {
        *df = 0;
        *dr = 0;
        return;
    }
    *df = deltas[index][0];
    *dr = deltas[index][1];
}

const char *gas_direction_name(CfFartDirection direction)
{
    static const char *names[8] = {
        "N", "NE", "E", "SE", "S", "SW", "W", "NW"
    };
    int index = (int)direction;
    if (index < 0 || index > 7) return "?";
    return names[index];
}

const char *gas_fart_preview_name(CfFartPreview preview)
{
    switch (preview) {
    case CF_FART_PUFF: return "PUFF";
    case CF_FART_PUSH: return "PUSH";
    case CF_FART_BLOCKED: return "BLOCKED";
    case CF_FART_PROMOTION: return "PROMOTE";
    default: return "INVALID";
    }
}

int gas_piece_can_fart(const CfBoard *board, const CfGasState *gas,
                       int file, int rank)
{
    const CfPiece *piece;
    if (board == 0 || gas == 0) return 0;
    piece = board_piece_at(board, file, rank);
    return piece != 0 && piece->type != CF_PIECE_NONE &&
           piece->color == board->side_to_move &&
           gas_at(gas, file, rank) >= CF_GAS_FART_COST;
}

static void push_scratch(CfBoard *scratch, int target_file, int target_rank,
                         int destination_file, int destination_rank,
                         CfPieceType promotion)
{
    CfPiece pushed = scratch->squares[target_rank][target_file];
    if (promotion != CF_PIECE_NONE) pushed.type = promotion;
    scratch->squares[destination_rank][destination_file] = pushed;
    scratch->squares[target_rank][target_file] = empty_piece();
}

static int push_keeps_actor_safe(const CfBoard *board,
                                 int target_file, int target_rank,
                                 int destination_file, int destination_rank,
                                 CfPieceType promotion)
{
    CfBoard scratch;
    scratch = *board;
    push_scratch(&scratch, target_file, target_rank,
                 destination_file, destination_rank, promotion);
    return !board_is_in_check(&scratch, board->side_to_move);
}

static int fart_geometry(int file, int rank,
                         CfFartDirection direction,
                         int *target_file, int *target_rank,
                         int *destination_file, int *destination_rank)
{
    int df;
    int dr;
    direction_delta(direction, &df, &dr);
    if (df == 0 && dr == 0) return 0;
    *target_file = file + df;
    *target_rank = rank + dr;
    *destination_file = *target_file + df;
    *destination_rank = *target_rank + dr;
    if (!in_bounds(*target_file, *target_rank)) return 0;
    return 1;
}

static int push_requires_promotion(CfPiece piece, int destination_rank)
{
    if (piece.type != CF_PIECE_PAWN) return 0;
    if (piece.color == CF_COLOR_WHITE && destination_rank == 7) return 1;
    if (piece.color == CF_COLOR_BLACK && destination_rank == 0) return 1;
    return 0;
}

int gas_fart_promotion_choice_legal(const CfBoard *board, const CfGasState *gas,
                                    int file, int rank, CfFartDirection direction,
                                    CfPieceType promotion)
{
    int tf;
    int tr;
    int df;
    int dr;
    CfPiece target;
    if (!gas_piece_can_fart(board, gas, file, rank)) return 0;
    if (!is_promotion_choice(promotion)) return 0;
    if (!fart_geometry(file, rank, direction, &tf, &tr, &df, &dr)) return 0;
    target = board->squares[tr][tf];
    if (target.type == CF_PIECE_NONE) return 0;
    if (!in_bounds(df, dr)) return 0;
    if (board->squares[dr][df].type != CF_PIECE_NONE) return 0;
    if (!push_requires_promotion(target, dr)) return 0;
    return push_keeps_actor_safe(board, tf, tr, df, dr, promotion);
}

CfFartPreview gas_preview_fart(const CfBoard *board, const CfGasState *gas,
                               int file, int rank, CfFartDirection direction)
{
    static const CfPieceType promotions[4] = {
        CF_PIECE_QUEEN, CF_PIECE_ROOK, CF_PIECE_BISHOP, CF_PIECE_KNIGHT
    };
    int tf;
    int tr;
    int df;
    int dr;
    int i;
    CfPiece target;

    if (!gas_piece_can_fart(board, gas, file, rank)) return CF_FART_INVALID;
    if (!fart_geometry(file, rank, direction, &tf, &tr, &df, &dr))
        return CF_FART_INVALID;

    target = board->squares[tr][tf];
    if (target.type == CF_PIECE_NONE)
        return board_is_in_check(board, board->side_to_move) ?
               CF_FART_INVALID : CF_FART_PUFF;

    if (!in_bounds(df, dr) || board->squares[dr][df].type != CF_PIECE_NONE)
        return board_is_in_check(board, board->side_to_move) ?
               CF_FART_INVALID : CF_FART_BLOCKED;

    if (push_requires_promotion(target, dr)) {
        for (i = 0; i < 4; ++i)
            if (push_keeps_actor_safe(board, tf, tr, df, dr, promotions[i]))
                return CF_FART_PROMOTION;
        return CF_FART_INVALID;
    }

    if (!push_keeps_actor_safe(board, tf, tr, df, dr, CF_PIECE_NONE))
        return CF_FART_INVALID;
    return CF_FART_PUSH;
}

static void lose_castling_rights_for_displacement(CfBoard *board,
                                                   CfPiece piece,
                                                   int from_file,
                                                   int from_rank)
{
    if (piece.type == CF_PIECE_KING) {
        if (piece.color == CF_COLOR_WHITE && from_file == 4 && from_rank == 0)
            board->castling_rights &= ~(CF_CASTLE_WHITE_KING | CF_CASTLE_WHITE_QUEEN);
        else if (piece.color == CF_COLOR_BLACK && from_file == 4 && from_rank == 7)
            board->castling_rights &= ~(CF_CASTLE_BLACK_KING | CF_CASTLE_BLACK_QUEEN);
    } else if (piece.type == CF_PIECE_ROOK) {
        if (piece.color == CF_COLOR_WHITE && from_rank == 0) {
            if (from_file == 0) board->castling_rights &= ~CF_CASTLE_WHITE_QUEEN;
            if (from_file == 7) board->castling_rights &= ~CF_CASTLE_WHITE_KING;
        } else if (piece.color == CF_COLOR_BLACK && from_rank == 7) {
            if (from_file == 0) board->castling_rights &= ~CF_CASTLE_BLACK_QUEEN;
            if (from_file == 7) board->castling_rights &= ~CF_CASTLE_BLACK_KING;
        }
    }
}

int gas_make_fart(CfBoard *board, CfGasState *gas,
                  int file, int rank, CfFartDirection direction,
                  CfPieceType promotion, CfFartAction *action)
{
    CfFartAction local;
    CfFartPreview preview;
    int tf;
    int tr;
    int df;
    int dr;
    CfPiece pushed;

    if (board == 0 || gas == 0) return 0;
    preview = gas_preview_fart(board, gas, file, rank, direction);
    if (preview == CF_FART_INVALID) return 0;
    if (preview == CF_FART_PROMOTION &&
        !gas_fart_promotion_choice_legal(board, gas, file, rank,
                                         direction, promotion)) return 0;

    memset(&local, 0, sizeof(local));
    local.actor_file = file;
    local.actor_rank = rank;
    local.direction = direction;
    local.result = preview;
    local.target_file = -1;
    local.target_rank = -1;
    local.destination_file = -1;
    local.destination_rank = -1;
    local.previous_actor_gas = gas_at(gas, file, rank);
    local.previous_side = board->side_to_move;
    local.previous_castling_rights = board->castling_rights;
    local.previous_ep_file = board->en_passant_file;
    local.previous_ep_rank = board->en_passant_rank;
    local.previous_halfmove = board->halfmove_clock;
    local.previous_fullmove = board->fullmove_number;
    local.promotion = CF_PIECE_NONE;

    if (!fart_geometry(file, rank, direction, &tf, &tr, &df, &dr))
        return 0;
    local.target_file = tf;
    local.target_rank = tr;
    local.previous_target_piece = board->squares[tr][tf];
    local.previous_target_gas = gas_at(gas, tf, tr);

    if (in_bounds(df, dr)) {
        local.destination_file = df;
        local.destination_rank = dr;
        local.previous_destination_piece = board->squares[dr][df];
        local.previous_destination_gas = gas_at(gas, df, dr);
    }

    gas->squares[rank][file] =
        (cf_u8)(gas->squares[rank][file] - CF_GAS_FART_COST);

    if (preview == CF_FART_PUSH || preview == CF_FART_PROMOTION) {
        pushed = board->squares[tr][tf];
        lose_castling_rights_for_displacement(board, pushed, tf, tr);
        board->squares[tr][tf] = empty_piece();
        gas->squares[tr][tf] = 0;
        if (preview == CF_FART_PROMOTION) {
            pushed.type = promotion;
            local.promotion = promotion;
        }
        board->squares[dr][df] = pushed;
        gas->squares[dr][df] = local.previous_target_gas;
    }

    board->en_passant_file = -1;
    board->en_passant_rank = -1;
    ++board->halfmove_clock;
    if (board->side_to_move == CF_COLOR_BLACK) ++board->fullmove_number;
    board->side_to_move = board_other_color(board->side_to_move);

    if (action != 0) *action = local;
    return 1;
}

void gas_unmake_fart(CfBoard *board, CfGasState *gas,
                     const CfFartAction *action)
{
    if (board == 0 || gas == 0 || action == 0) return;

    gas_set(gas, action->actor_file, action->actor_rank,
            action->previous_actor_gas);

    if (action->target_file >= 0 && action->target_rank >= 0) {
        board->squares[action->target_rank][action->target_file] =
            action->previous_target_piece;
        gas->squares[action->target_rank][action->target_file] =
            action->previous_target_gas;
    }
    if (action->destination_file >= 0 && action->destination_rank >= 0) {
        board->squares[action->destination_rank][action->destination_file] =
            action->previous_destination_piece;
        gas->squares[action->destination_rank][action->destination_file] =
            action->previous_destination_gas;
    }

    board->side_to_move = action->previous_side;
    board->castling_rights = action->previous_castling_rights;
    board->en_passant_file = action->previous_ep_file;
    board->en_passant_rank = action->previous_ep_rank;
    board->halfmove_clock = action->previous_halfmove;
    board->fullmove_number = action->previous_fullmove;
}

int gas_make_puff(CfBoard *board, CfGasState *gas,
                  int file, int rank, CfFartDirection direction,
                  CfFartAction *action)
{
    CfFartAction local;
    if (gas_preview_fart(board, gas, file, rank, direction) != CF_FART_PUFF)
        return 0;
    if (!gas_make_fart(board, gas, file, rank, direction,
                       CF_PIECE_QUEEN, &local)) return 0;
    if (action != 0) *action = local;
    return 1;
}

void gas_unmake_puff(CfBoard *board, CfGasState *gas,
                     const CfFartAction *action)
{
    gas_unmake_fart(board, gas, action);
}

static int effective_en_passant_file(const CfBoard *board)
{
    CfMoveList list;
    int pawn_rank;
    int file;
    int i;
    if (board->en_passant_file < 0 || board->en_passant_rank < 0) return -1;
    pawn_rank = board->side_to_move == CF_COLOR_WHITE ?
                board->en_passant_rank - 1 : board->en_passant_rank + 1;
    for (file = board->en_passant_file - 1;
         file <= board->en_passant_file + 1; file += 2) {
        if (!in_bounds(file, pawn_rank)) continue;
        if (board->squares[pawn_rank][file].type != CF_PIECE_PAWN ||
            board->squares[pawn_rank][file].color != board->side_to_move) continue;
        board_generate_legal_moves(board, file, pawn_rank, &list);
        for (i = 0; i < list.count; ++i)
            if ((list.moves[i].flags & CF_MOVE_EN_PASSANT) != 0U)
                return board->en_passant_file;
    }
    return -1;
}

static cf_u8 piece_code(CfPiece piece)
{
    int code;
    if (piece.type == CF_PIECE_NONE) return 0;
    code = (int)piece.type;
    if (piece.color == CF_COLOR_BLACK) code += 6;
    return (cf_u8)code;
}

static void make_key(const CfBoard *board, const CfGasState *gas,
                     CfGasPositionKey *key)
{
    int square;
    int rank;
    int file;
    int ep_file;
    cf_u8 code;
    memset(key, 0, sizeof(*key));
    for (square = 0; square < 64; ++square) {
        rank = square / 8;
        file = square % 8;
        code = piece_code(board->squares[rank][file]);
        if (code != 0)
            code = (cf_u8)(code |
                   (cf_u8)((gas_at(gas, file, rank) & 3U) << 4));
        key->squares[square] = code;
    }
    key->state = (cf_u8)((board->side_to_move == CF_COLOR_BLACK ? 1U : 0U) |
                         ((board->castling_rights & CF_CASTLE_ALL) << 1));
    ep_file = effective_en_passant_file(board);
    key->en_passant_file = (cf_u8)(ep_file >= 0 ? ep_file : 8);
}

static int key_equal(const CfGasPositionKey *a, const CfGasPositionKey *b)
{
    return a->state == b->state &&
           a->en_passant_file == b->en_passant_file &&
           memcmp(a->squares, b->squares, sizeof(a->squares)) == 0;
}

void gas_history_record(CfGasHistory *history, const CfBoard *board,
                        const CfGasState *gas)
{
    CfGasPositionKey key;
    int i;
    if (history == 0 || board == 0 || gas == 0) return;
    make_key(board, gas, &key);
    if (history->count >= CF_GAS_HISTORY) {
        for (i = 1; i < CF_GAS_HISTORY; ++i)
            history->keys[i - 1] = history->keys[i];
        history->count = CF_GAS_HISTORY - 1;
    }
    history->keys[history->count++] = key;
}

void gas_history_init(CfGasHistory *history, const CfBoard *board,
                      const CfGasState *gas)
{
    if (history == 0) return;
    history->count = 0;
    if (board != 0 && gas != 0) gas_history_record(history, board, gas);
}

int gas_history_repetition_count(const CfGasHistory *history,
                                 const CfBoard *board, const CfGasState *gas)
{
    CfGasPositionKey key;
    int i;
    int count = 0;
    if (history == 0 || board == 0 || gas == 0) return 0;
    make_key(board, gas, &key);
    for (i = 0; i < history->count; ++i)
        if (key_equal(&key, &history->keys[i])) ++count;
    return count;
}

static int has_legal_fart(const CfBoard *board, const CfGasState *gas)
{
    int file;
    int rank;
    int direction;
    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            if (!gas_piece_can_fart(board, gas, file, rank)) continue;
            for (direction = 0; direction < 8; ++direction)
                if (gas_preview_fart(board, gas, file, rank,
                                     (CfFartDirection)direction) != CF_FART_INVALID)
                    return 1;
        }
    }
    return 0;
}

CfGameStatus gas_game_status(const CfBoard *board, const CfGasState *gas,
                             const CfGasHistory *history)
{
    int moves;
    int check;
    int fart;
    if (board == 0 || gas == 0) return CF_GAME_ONGOING;
    moves = board_count_legal_moves(board, board->side_to_move);
    check = board_is_in_check(board, board->side_to_move);
    fart = has_legal_fart(board, gas);
    if (moves == 0 && !fart)
        return check ? CF_GAME_CHECKMATE : CF_GAME_STALEMATE;
    if (board_is_insufficient_material(board)) return CF_GAME_DRAW_INSUFFICIENT;
    if (history != 0 && gas_history_repetition_count(history, board, gas) >= 3)
        return CF_GAME_DRAW_THREEFOLD;
    if (board->halfmove_clock >= 100U) return CF_GAME_DRAW_FIFTY_MOVE;
    if (check) return CF_GAME_CHECK;
    return CF_GAME_ONGOING;
}
