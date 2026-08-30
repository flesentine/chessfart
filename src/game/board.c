#include <string.h>

#include "board.h"

static CfPiece make_piece(CfPieceType type, CfPieceColor color)
{
    CfPiece piece;
    piece.type = type;
    piece.color = color;
    return piece;
}

static int in_bounds(int file, int rank)
{
    return file >= 0 && file < 8 && rank >= 0 && rank < 8;
}

CfPieceColor board_other_color(CfPieceColor color)
{
    if (color == CF_COLOR_WHITE) return CF_COLOR_BLACK;
    if (color == CF_COLOR_BLACK) return CF_COLOR_WHITE;
    return CF_COLOR_NONE;
}

void board_clear(CfBoard *board)
{
    if (board == 0) return;
    memset(board, 0, sizeof(*board));
    board->side_to_move = CF_COLOR_WHITE;
    board->castling_rights = 0U;
    board->en_passant_file = -1;
    board->en_passant_rank = -1;
    board->halfmove_clock = 0U;
    board->fullmove_number = 1U;
}

void board_init_starting_position(CfBoard *board)
{
    static const CfPieceType back_rank[8] = {
        CF_PIECE_ROOK, CF_PIECE_KNIGHT, CF_PIECE_BISHOP, CF_PIECE_QUEEN,
        CF_PIECE_KING, CF_PIECE_BISHOP, CF_PIECE_KNIGHT, CF_PIECE_ROOK
    };
    int file;
    if (board == 0) return;
    board_clear(board);
    for (file = 0; file < 8; ++file) {
        board->squares[0][file] = make_piece(back_rank[file], CF_COLOR_WHITE);
        board->squares[1][file] = make_piece(CF_PIECE_PAWN, CF_COLOR_WHITE);
        board->squares[6][file] = make_piece(CF_PIECE_PAWN, CF_COLOR_BLACK);
        board->squares[7][file] = make_piece(back_rank[file], CF_COLOR_BLACK);
    }
    board->castling_rights = CF_CASTLE_ALL;
}

const CfPiece *board_piece_at(const CfBoard *board, int file, int rank)
{
    if (board == 0 || !in_bounds(file, rank)) return 0;
    return &board->squares[rank][file];
}

void board_set_piece(CfBoard *board, int file, int rank, CfPieceType type, CfPieceColor color)
{
    if (board == 0 || !in_bounds(file, rank)) return;
    board->squares[rank][file] = make_piece(type, color);
}

int board_piece_count(const CfBoard *board)
{
    int file;
    int rank;
    int count = 0;
    if (board == 0) return 0;
    for (rank = 0; rank < 8; ++rank)
        for (file = 0; file < 8; ++file)
            if (board->squares[rank][file].type != CF_PIECE_NONE) ++count;
    return count;
}

const char *board_piece_type_name(CfPieceType type)
{
    switch (type) {
    case CF_PIECE_PAWN: return "PAWN";
    case CF_PIECE_KNIGHT: return "KNIGHT";
    case CF_PIECE_BISHOP: return "BISHOP";
    case CF_PIECE_ROOK: return "ROOK";
    case CF_PIECE_QUEEN: return "QUEEN";
    case CF_PIECE_KING: return "KING";
    default: return "EMPTY";
    }
}

const char *board_piece_color_name(CfPieceColor color)
{
    switch (color) {
    case CF_COLOR_WHITE: return "WHITE";
    case CF_COLOR_BLACK: return "BLACK";
    default: return "NONE";
    }
}

char board_piece_letter(CfPieceType type)
{
    switch (type) {
    case CF_PIECE_PAWN: return 'P';
    case CF_PIECE_KNIGHT: return 'N';
    case CF_PIECE_BISHOP: return 'B';
    case CF_PIECE_ROOK: return 'R';
    case CF_PIECE_QUEEN: return 'Q';
    case CF_PIECE_KING: return 'K';
    default: return ' ';
    }
}

static int ray_attacks(const CfBoard *board, int from_file, int from_rank,
                       int target_file, int target_rank, int df, int dr)
{
    int file = from_file + df;
    int rank = from_rank + dr;
    while (in_bounds(file, rank)) {
        if (file == target_file && rank == target_rank) return 1;
        if (board->squares[rank][file].type != CF_PIECE_NONE) return 0;
        file += df;
        rank += dr;
    }
    return 0;
}

int board_square_is_attacked(const CfBoard *board, int file, int rank, CfPieceColor by_color)
{
    static const int knight_offsets[8][2] = {
        {1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}
    };
    int f;
    int r;
    int i;
    int df;
    int dr;
    const CfPiece *piece;
    if (board == 0 || !in_bounds(file, rank)) return 0;

    for (r = 0; r < 8; ++r) {
        for (f = 0; f < 8; ++f) {
            piece = &board->squares[r][f];
            if (piece->color != by_color) continue;
            df = file - f;
            dr = rank - r;
            switch (piece->type) {
            case CF_PIECE_PAWN:
                if (by_color == CF_COLOR_WHITE && dr == 1 && (df == 1 || df == -1)) return 1;
                if (by_color == CF_COLOR_BLACK && dr == -1 && (df == 1 || df == -1)) return 1;
                break;
            case CF_PIECE_KNIGHT:
                for (i = 0; i < 8; ++i)
                    if (df == knight_offsets[i][0] && dr == knight_offsets[i][1]) return 1;
                break;
            case CF_PIECE_BISHOP:
                if (df != 0 && (df == dr || df == -dr))
                    if (ray_attacks(board, f, r, file, rank,
                                    df > 0 ? 1 : -1, dr > 0 ? 1 : -1)) return 1;
                break;
            case CF_PIECE_ROOK:
                if ((df == 0) != (dr == 0))
                    if (ray_attacks(board, f, r, file, rank,
                                    df == 0 ? 0 : (df > 0 ? 1 : -1),
                                    dr == 0 ? 0 : (dr > 0 ? 1 : -1))) return 1;
                break;
            case CF_PIECE_QUEEN:
                if (df == 0 || dr == 0 || df == dr || df == -dr)
                    if (!(df == 0 && dr == 0) &&
                        ray_attacks(board, f, r, file, rank,
                                    df == 0 ? 0 : (df > 0 ? 1 : -1),
                                    dr == 0 ? 0 : (dr > 0 ? 1 : -1))) return 1;
                break;
            case CF_PIECE_KING:
                if (df >= -1 && df <= 1 && dr >= -1 && dr <= 1 &&
                    !(df == 0 && dr == 0)) return 1;
                break;
            default:
                break;
            }
        }
    }
    return 0;
}

int board_is_in_check(const CfBoard *board, CfPieceColor color)
{
    int file;
    int rank;
    if (board == 0) return 0;
    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            if (board->squares[rank][file].type == CF_PIECE_KING &&
                board->squares[rank][file].color == color)
                return board_square_is_attacked(board, file, rank, board_other_color(color));
        }
    }
    return 1;
}

static void add_move(CfMoveList *list, int ff, int fr, int tf, int tr,
                     CfPiece moved, CfPiece captured, int captured_file,
                     int captured_rank, unsigned flags, CfPieceType promotion)
{
    CfMove *move;
    if (list == 0 || list->count >= CF_MAX_MOVES) return;
    move = &list->moves[list->count++];
    memset(move, 0, sizeof(*move));
    move->from_file = ff;
    move->from_rank = fr;
    move->to_file = tf;
    move->to_rank = tr;
    move->captured_file = captured_file;
    move->captured_rank = captured_rank;
    move->moved = moved;
    move->captured = captured;
    move->flags = flags;
    move->promotion = promotion;
}

static void add_regular_move(const CfBoard *board, CfMoveList *list,
                             int ff, int fr, int tf, int tr, unsigned flags)
{
    CfPiece moved;
    CfPiece captured;
    if (!in_bounds(tf, tr)) return;
    moved = board->squares[fr][ff];
    captured = board->squares[tr][tf];
    if (captured.color == moved.color) return;
    if (captured.type == CF_PIECE_KING) return;
    add_move(list, ff, fr, tf, tr, moved, captured, tf, tr, flags, CF_PIECE_NONE);
}

static void add_promotion_moves(const CfBoard *board, CfMoveList *list,
                                int ff, int fr, int tf, int tr, CfPiece captured)
{
    static const CfPieceType choices[4] = {
        CF_PIECE_QUEEN, CF_PIECE_ROOK, CF_PIECE_BISHOP, CF_PIECE_KNIGHT
    };
    CfPiece moved = board->squares[fr][ff];
    int i;
    for (i = 0; i < 4; ++i)
        add_move(list, ff, fr, tf, tr, moved, captured, tf, tr,
                 CF_MOVE_PROMOTION, choices[i]);
}

static void add_ray_moves(const CfBoard *board, CfMoveList *list,
                          int ff, int fr, int df, int dr)
{
    int tf = ff + df;
    int tr = fr + dr;
    CfPiece moved = board->squares[fr][ff];
    while (in_bounds(tf, tr)) {
        CfPiece target = board->squares[tr][tf];
        if (target.type == CF_PIECE_NONE) {
            add_move(list, ff, fr, tf, tr, moved, target, tf, tr, 0U, CF_PIECE_NONE);
        } else {
            if (target.color != moved.color && target.type != CF_PIECE_KING)
                add_move(list, ff, fr, tf, tr, moved, target, tf, tr, 0U, CF_PIECE_NONE);
            break;
        }
        tf += df;
        tr += dr;
    }
}

static void add_castle_moves(const CfBoard *board, int file, int rank,
                             CfPiece piece, CfMoveList *list)
{
    int home_rank;
    CfPiece empty = make_piece(CF_PIECE_NONE, CF_COLOR_NONE);
    CfPieceColor enemy;
    unsigned king_right;
    unsigned queen_right;

    home_rank = piece.color == CF_COLOR_WHITE ? 0 : 7;
    if (file != 4 || rank != home_rank) return;
    enemy = board_other_color(piece.color);
    king_right = piece.color == CF_COLOR_WHITE ? CF_CASTLE_WHITE_KING : CF_CASTLE_BLACK_KING;
    queen_right = piece.color == CF_COLOR_WHITE ? CF_CASTLE_WHITE_QUEEN : CF_CASTLE_BLACK_QUEEN;

    if ((board->castling_rights & king_right) != 0U &&
        board->squares[home_rank][5].type == CF_PIECE_NONE &&
        board->squares[home_rank][6].type == CF_PIECE_NONE &&
        board->squares[home_rank][7].type == CF_PIECE_ROOK &&
        board->squares[home_rank][7].color == piece.color &&
        !board_square_is_attacked(board, 4, home_rank, enemy) &&
        !board_square_is_attacked(board, 5, home_rank, enemy) &&
        !board_square_is_attacked(board, 6, home_rank, enemy))
        add_move(list, 4, home_rank, 6, home_rank, piece, empty, 6, home_rank,
                 CF_MOVE_CASTLE_KING, CF_PIECE_NONE);

    if ((board->castling_rights & queen_right) != 0U &&
        board->squares[home_rank][1].type == CF_PIECE_NONE &&
        board->squares[home_rank][2].type == CF_PIECE_NONE &&
        board->squares[home_rank][3].type == CF_PIECE_NONE &&
        board->squares[home_rank][0].type == CF_PIECE_ROOK &&
        board->squares[home_rank][0].color == piece.color &&
        !board_square_is_attacked(board, 4, home_rank, enemy) &&
        !board_square_is_attacked(board, 3, home_rank, enemy) &&
        !board_square_is_attacked(board, 2, home_rank, enemy))
        add_move(list, 4, home_rank, 2, home_rank, piece, empty, 2, home_rank,
                 CF_MOVE_CASTLE_QUEEN, CF_PIECE_NONE);
}

static void generate_pseudo_moves(const CfBoard *board, int file, int rank, CfMoveList *list)
{
    static const int knight_offsets[8][2] = {
        {1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}
    };
    static const int king_offsets[8][2] = {
        {1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1}
    };
    CfPiece piece;
    int i;
    int dir;
    int start_rank;
    int next_rank;
    int promotion_rank;
    int tf;
    CfPiece target;
    CfPiece ep_captured;

    list->count = 0;
    if (board == 0 || !in_bounds(file, rank)) return;
    piece = board->squares[rank][file];
    if (piece.type == CF_PIECE_NONE) return;

    switch (piece.type) {
    case CF_PIECE_PAWN:
        dir = piece.color == CF_COLOR_WHITE ? 1 : -1;
        start_rank = piece.color == CF_COLOR_WHITE ? 1 : 6;
        promotion_rank = piece.color == CF_COLOR_WHITE ? 7 : 0;
        next_rank = rank + dir;
        if (in_bounds(file, next_rank) && board->squares[next_rank][file].type == CF_PIECE_NONE) {
            if (next_rank == promotion_rank)
                add_promotion_moves(board, list, file, rank, file, next_rank,
                                    board->squares[next_rank][file]);
            else {
                add_move(list, file, rank, file, next_rank, piece,
                         board->squares[next_rank][file], file, next_rank, 0U, CF_PIECE_NONE);
                if (rank == start_rank &&
                    board->squares[rank + dir * 2][file].type == CF_PIECE_NONE)
                    add_move(list, file, rank, file, rank + dir * 2, piece,
                             board->squares[rank + dir * 2][file], file, rank + dir * 2,
                             CF_MOVE_PAWN_DOUBLE, CF_PIECE_NONE);
            }
        }
        for (i = -1; i <= 1; i += 2) {
            tf = file + i;
            if (!in_bounds(tf, next_rank)) continue;
            target = board->squares[next_rank][tf];
            if (target.type != CF_PIECE_NONE && target.color != piece.color &&
                target.type != CF_PIECE_KING) {
                if (next_rank == promotion_rank)
                    add_promotion_moves(board, list, file, rank, tf, next_rank, target);
                else
                    add_move(list, file, rank, tf, next_rank, piece, target,
                             tf, next_rank, 0U, CF_PIECE_NONE);
            } else if (target.type == CF_PIECE_NONE &&
                       board->en_passant_file == tf && board->en_passant_rank == next_rank) {
                ep_captured = board->squares[rank][tf];
                if (ep_captured.type == CF_PIECE_PAWN && ep_captured.color == board_other_color(piece.color))
                    add_move(list, file, rank, tf, next_rank, piece, ep_captured,
                             tf, rank, CF_MOVE_EN_PASSANT, CF_PIECE_NONE);
            }
        }
        break;
    case CF_PIECE_KNIGHT:
        for (i = 0; i < 8; ++i)
            add_regular_move(board, list, file, rank,
                             file + knight_offsets[i][0], rank + knight_offsets[i][1], 0U);
        break;
    case CF_PIECE_BISHOP:
        add_ray_moves(board, list, file, rank, 1, 1);
        add_ray_moves(board, list, file, rank, 1, -1);
        add_ray_moves(board, list, file, rank, -1, 1);
        add_ray_moves(board, list, file, rank, -1, -1);
        break;
    case CF_PIECE_ROOK:
        add_ray_moves(board, list, file, rank, 1, 0);
        add_ray_moves(board, list, file, rank, -1, 0);
        add_ray_moves(board, list, file, rank, 0, 1);
        add_ray_moves(board, list, file, rank, 0, -1);
        break;
    case CF_PIECE_QUEEN:
        add_ray_moves(board, list, file, rank, 1, 0);
        add_ray_moves(board, list, file, rank, -1, 0);
        add_ray_moves(board, list, file, rank, 0, 1);
        add_ray_moves(board, list, file, rank, 0, -1);
        add_ray_moves(board, list, file, rank, 1, 1);
        add_ray_moves(board, list, file, rank, 1, -1);
        add_ray_moves(board, list, file, rank, -1, 1);
        add_ray_moves(board, list, file, rank, -1, -1);
        break;
    case CF_PIECE_KING:
        for (i = 0; i < 8; ++i)
            add_regular_move(board, list, file, rank,
                             file + king_offsets[i][0], rank + king_offsets[i][1], 0U);
        add_castle_moves(board, file, rank, piece, list);
        break;
    default:
        break;
    }
}

static void apply_position_only(CfBoard *board, const CfMove *move)
{
    CfPiece empty = make_piece(CF_PIECE_NONE, CF_COLOR_NONE);
    CfPiece placed = move->moved;
    int home_rank;

    board->squares[move->from_rank][move->from_file] = empty;
    if ((move->flags & CF_MOVE_EN_PASSANT) != 0U)
        board->squares[move->captured_rank][move->captured_file] = empty;

    if ((move->flags & CF_MOVE_PROMOTION) != 0U)
        placed.type = move->promotion;
    board->squares[move->to_rank][move->to_file] = placed;

    if ((move->flags & CF_MOVE_CASTLE_KING) != 0U) {
        home_rank = move->from_rank;
        board->squares[home_rank][5] = board->squares[home_rank][7];
        board->squares[home_rank][7] = empty;
    } else if ((move->flags & CF_MOVE_CASTLE_QUEEN) != 0U) {
        home_rank = move->from_rank;
        board->squares[home_rank][3] = board->squares[home_rank][0];
        board->squares[home_rank][0] = empty;
    }
}

static void clear_rook_right(CfBoard *board, CfPieceColor color, int file, int rank)
{
    if (color == CF_COLOR_WHITE && rank == 0) {
        if (file == 0) board->castling_rights &= ~CF_CASTLE_WHITE_QUEEN;
        if (file == 7) board->castling_rights &= ~CF_CASTLE_WHITE_KING;
    } else if (color == CF_COLOR_BLACK && rank == 7) {
        if (file == 0) board->castling_rights &= ~CF_CASTLE_BLACK_QUEEN;
        if (file == 7) board->castling_rights &= ~CF_CASTLE_BLACK_KING;
    }
}

static void apply_stateful(CfBoard *board, CfMove *move)
{
    move->prev_side_to_move = board->side_to_move;
    move->prev_castling_rights = board->castling_rights;
    move->prev_en_passant_file = board->en_passant_file;
    move->prev_en_passant_rank = board->en_passant_rank;
    move->prev_halfmove_clock = board->halfmove_clock;
    move->prev_fullmove_number = board->fullmove_number;

    apply_position_only(board, move);

    if (move->moved.type == CF_PIECE_KING) {
        if (move->moved.color == CF_COLOR_WHITE)
            board->castling_rights &= ~(CF_CASTLE_WHITE_KING | CF_CASTLE_WHITE_QUEEN);
        else
            board->castling_rights &= ~(CF_CASTLE_BLACK_KING | CF_CASTLE_BLACK_QUEEN);
    } else if (move->moved.type == CF_PIECE_ROOK) {
        clear_rook_right(board, move->moved.color, move->from_file, move->from_rank);
    }
    if (move->captured.type == CF_PIECE_ROOK)
        clear_rook_right(board, move->captured.color, move->captured_file, move->captured_rank);

    board->en_passant_file = -1;
    board->en_passant_rank = -1;
    if ((move->flags & CF_MOVE_PAWN_DOUBLE) != 0U) {
        board->en_passant_file = move->from_file;
        board->en_passant_rank = (move->from_rank + move->to_rank) / 2;
    }

    if (move->moved.type == CF_PIECE_PAWN || move->captured.type != CF_PIECE_NONE)
        board->halfmove_clock = 0U;
    else
        ++board->halfmove_clock;

    if (board->side_to_move == CF_COLOR_BLACK)
        ++board->fullmove_number;
    board->side_to_move = board_other_color(board->side_to_move);
}

void board_unmake_move(CfBoard *board, const CfMove *move)
{
    CfPiece empty = make_piece(CF_PIECE_NONE, CF_COLOR_NONE);
    int home_rank;
    if (board == 0 || move == 0) return;

    board->squares[move->from_rank][move->from_file] = move->moved;
    if ((move->flags & CF_MOVE_EN_PASSANT) != 0U) {
        board->squares[move->to_rank][move->to_file] = empty;
        board->squares[move->captured_rank][move->captured_file] = move->captured;
    } else {
        board->squares[move->to_rank][move->to_file] = move->captured;
    }

    if ((move->flags & CF_MOVE_CASTLE_KING) != 0U) {
        home_rank = move->from_rank;
        board->squares[home_rank][7] = board->squares[home_rank][5];
        board->squares[home_rank][5] = empty;
    } else if ((move->flags & CF_MOVE_CASTLE_QUEEN) != 0U) {
        home_rank = move->from_rank;
        board->squares[home_rank][0] = board->squares[home_rank][3];
        board->squares[home_rank][3] = empty;
    }

    board->side_to_move = move->prev_side_to_move;
    board->castling_rights = move->prev_castling_rights;
    board->en_passant_file = move->prev_en_passant_file;
    board->en_passant_rank = move->prev_en_passant_rank;
    board->halfmove_clock = move->prev_halfmove_clock;
    board->fullmove_number = move->prev_fullmove_number;
}

void board_generate_legal_moves(const CfBoard *board, int file, int rank, CfMoveList *list)
{
    CfMoveList pseudo;
    CfBoard scratch;
    CfPiece piece;
    int i;
    if (list == 0) return;
    list->count = 0;
    if (board == 0 || !in_bounds(file, rank)) return;
    piece = board->squares[rank][file];
    if (piece.type == CF_PIECE_NONE || piece.color != board->side_to_move) return;
    generate_pseudo_moves(board, file, rank, &pseudo);
    for (i = 0; i < pseudo.count; ++i) {
        scratch = *board;
        apply_position_only(&scratch, &pseudo.moves[i]);
        if (!board_is_in_check(&scratch, piece.color) && list->count < CF_MAX_MOVES)
            list->moves[list->count++] = pseudo.moves[i];
    }
}

int board_move_is_legal(const CfBoard *board, int from_file, int from_rank,
                        int to_file, int to_rank)
{
    CfMoveList list;
    int i;
    board_generate_legal_moves(board, from_file, from_rank, &list);
    for (i = 0; i < list.count; ++i)
        if (list.moves[i].to_file == to_file && list.moves[i].to_rank == to_rank) return 1;
    return 0;
}

int board_make_move_ex(CfBoard *board, int from_file, int from_rank,
                       int to_file, int to_rank, CfPieceType promotion,
                       CfMove *made_move)
{
    CfMoveList list;
    CfMove move;
    int i;
    if (board == 0) return 0;
    board_generate_legal_moves(board, from_file, from_rank, &list);
    if (promotion == CF_PIECE_NONE) promotion = CF_PIECE_QUEEN;
    for (i = 0; i < list.count; ++i) {
        if (list.moves[i].to_file != to_file || list.moves[i].to_rank != to_rank) continue;
        if ((list.moves[i].flags & CF_MOVE_PROMOTION) != 0U && list.moves[i].promotion != promotion) continue;
        move = list.moves[i];
        apply_stateful(board, &move);
        if (made_move != 0) *made_move = move;
        return 1;
    }
    return 0;
}

int board_make_move(CfBoard *board, int from_file, int from_rank,
                    int to_file, int to_rank, CfMove *made_move)
{
    return board_make_move_ex(board, from_file, from_rank, to_file, to_rank,
                              CF_PIECE_QUEEN, made_move);
}

int board_count_legal_moves(const CfBoard *board, CfPieceColor color)
{
    CfBoard scratch;
    CfMoveList list;
    int file;
    int rank;
    int total = 0;
    if (board == 0) return 0;
    scratch = *board;
    if (color != board->side_to_move) {
        scratch.side_to_move = color;
        scratch.en_passant_file = -1;
        scratch.en_passant_rank = -1;
    }
    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            if (scratch.squares[rank][file].color == color) {
                board_generate_legal_moves(&scratch, file, rank, &list);
                total += list.count;
            }
        }
    }
    return total;
}

static cf_u8 piece_key(CfPiece piece)
{
    int code;
    if (piece.type == CF_PIECE_NONE) return 0;
    code = (int)piece.type;
    if (piece.color == CF_COLOR_BLACK) code += 6;
    return (cf_u8)code;
}

static int effective_en_passant_file(const CfBoard *board)
{
    CfMoveList list;
    int pawn_rank;
    int file;
    int i;
    if (board->en_passant_file < 0 || board->en_passant_rank < 0) return -1;
    pawn_rank = board->side_to_move == CF_COLOR_WHITE ? board->en_passant_rank - 1 : board->en_passant_rank + 1;
    for (file = board->en_passant_file - 1; file <= board->en_passant_file + 1; file += 2) {
        if (!in_bounds(file, pawn_rank)) continue;
        if (board->squares[pawn_rank][file].type != CF_PIECE_PAWN ||
            board->squares[pawn_rank][file].color != board->side_to_move) continue;
        board_generate_legal_moves(board, file, pawn_rank, &list);
        for (i = 0; i < list.count; ++i)
            if ((list.moves[i].flags & CF_MOVE_EN_PASSANT) != 0U) return board->en_passant_file;
    }
    return -1;
}

static void position_key(const CfBoard *board, CfPositionKey *key)
{
    int square;
    int rank;
    int file;
    int ep_file;
    cf_u8 low;
    cf_u8 high;
    memset(key, 0, sizeof(*key));
    for (square = 0; square < 64; square += 2) {
        rank = square / 8;
        file = square % 8;
        low = piece_key(board->squares[rank][file]);
        rank = (square + 1) / 8;
        file = (square + 1) % 8;
        high = piece_key(board->squares[rank][file]);
        key->packed_squares[square / 2] = (cf_u8)(low | (cf_u8)(high << 4));
    }
    key->state = (cf_u8)((board->side_to_move == CF_COLOR_BLACK ? 1U : 0U) |
                         ((board->castling_rights & CF_CASTLE_ALL) << 1));
    ep_file = effective_en_passant_file(board);
    key->en_passant_file = (cf_u8)(ep_file >= 0 ? ep_file : 8);
}

static int key_equal(const CfPositionKey *a, const CfPositionKey *b)
{
    return a->state == b->state && a->en_passant_file == b->en_passant_file &&
           memcmp(a->packed_squares, b->packed_squares, sizeof(a->packed_squares)) == 0;
}

static void history_push(CfHistory *history, const CfBoard *board)
{
    CfPositionKey key;
    int i;
    if (history == 0 || board == 0) return;
    position_key(board, &key);
    if (history->count >= CF_MAX_HISTORY) {
        for (i = 1; i < CF_MAX_HISTORY; ++i)
            history->keys[i - 1] = history->keys[i];
        history->count = CF_MAX_HISTORY - 1;
    }
    history->keys[history->count++] = key;
}

void board_history_init(CfHistory *history, const CfBoard *board)
{
    if (history == 0) return;
    history->count = 0;
    if (board != 0) history_push(history, board);
}

void board_history_record_move(CfHistory *history, const CfBoard *board, const CfMove *move)
{
    if (history == 0 || board == 0 || move == 0) return;
    if (move->moved.type == CF_PIECE_PAWN || move->captured.type != CF_PIECE_NONE ||
        move->prev_castling_rights != board->castling_rights)
        history->count = 0;
    history_push(history, board);
}

int board_history_repetition_count(const CfHistory *history, const CfBoard *board)
{
    CfPositionKey key;
    int i;
    int count = 0;
    if (history == 0 || board == 0) return 0;
    position_key(board, &key);
    for (i = 0; i < history->count; ++i)
        if (key_equal(&key, &history->keys[i])) ++count;
    return count;
}

int board_is_insufficient_material(const CfBoard *board)
{
    int file;
    int rank;
    int minors = 0;
    int bishops = 0;
    int knights = 0;
    int bishop_color = -1;
    int square_color;
    CfPiece piece;
    if (board == 0) return 0;
    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            piece = board->squares[rank][file];
            if (piece.type == CF_PIECE_NONE || piece.type == CF_PIECE_KING) continue;
            if (piece.type == CF_PIECE_PAWN || piece.type == CF_PIECE_ROOK || piece.type == CF_PIECE_QUEEN)
                return 0;
            if (piece.type == CF_PIECE_KNIGHT) {
                ++knights;
                ++minors;
            } else if (piece.type == CF_PIECE_BISHOP) {
                ++bishops;
                ++minors;
                square_color = (file + rank) & 1;
                if (bishop_color < 0) bishop_color = square_color;
                else if (bishop_color != square_color) bishop_color = 2;
            }
        }
    }
    if (minors == 0) return 1;
    if (minors == 1) return 1;
    if (knights == 0 && bishops == minors && bishop_color >= 0 && bishop_color < 2) return 1;
    return 0;
}

CfGameStatus board_game_status(const CfBoard *board, const CfHistory *history)
{
    int moves;
    int check;
    if (board == 0) return CF_GAME_ONGOING;
    moves = board_count_legal_moves(board, board->side_to_move);
    check = board_is_in_check(board, board->side_to_move);
    if (moves == 0) return check ? CF_GAME_CHECKMATE : CF_GAME_STALEMATE;
    if (board_is_insufficient_material(board)) return CF_GAME_DRAW_INSUFFICIENT;
    if (history != 0 && board_history_repetition_count(history, board) >= 3)
        return CF_GAME_DRAW_THREEFOLD;
    if (board->halfmove_clock >= 100U) return CF_GAME_DRAW_FIFTY_MOVE;
    if (check) return CF_GAME_CHECK;
    return CF_GAME_ONGOING;
}

const char *board_game_status_name(CfGameStatus status)
{
    switch (status) {
    case CF_GAME_CHECK: return "CHECK";
    case CF_GAME_CHECKMATE: return "CHECKMATE";
    case CF_GAME_STALEMATE: return "STALEMATE";
    case CF_GAME_DRAW_FIFTY_MOVE: return "DRAW 50 MOVE";
    case CF_GAME_DRAW_THREEFOLD: return "DRAW THREEFOLD";
    case CF_GAME_DRAW_INSUFFICIENT: return "DRAW MATERIAL";
    default: return "PLAYING";
    }
}
