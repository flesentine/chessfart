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
    int file, rank, count = 0;
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
    int f, r, i, df, dr;
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
                    if (ray_attacks(board, f, r, file, rank, df > 0 ? 1 : -1, dr > 0 ? 1 : -1)) return 1;
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
                if (df >= -1 && df <= 1 && dr >= -1 && dr <= 1 && !(df == 0 && dr == 0)) return 1;
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
    int file, rank;
    if (board == 0) return 0;
    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            if (board->squares[rank][file].type == CF_PIECE_KING && board->squares[rank][file].color == color)
                return board_square_is_attacked(board, file, rank, board_other_color(color));
        }
    }
    return 1;
}

static void add_move(CfMoveList *list, int ff, int fr, int tf, int tr, CfPiece moved, CfPiece captured)
{
    CfMove *move;
    if (list == 0 || list->count >= CF_MAX_MOVES) return;
    move = &list->moves[list->count++];
    move->from_file = ff; move->from_rank = fr;
    move->to_file = tf; move->to_rank = tr;
    move->moved = moved; move->captured = captured;
}

static void add_if_pseudo_legal(const CfBoard *board, CfMoveList *list,
                                int ff, int fr, int tf, int tr)
{
    CfPiece moved, captured;
    if (!in_bounds(tf, tr)) return;
    moved = board->squares[fr][ff];
    captured = board->squares[tr][tf];
    if (captured.color == moved.color) return;
    if (captured.type == CF_PIECE_KING) return;
    add_move(list, ff, fr, tf, tr, moved, captured);
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
            add_move(list, ff, fr, tf, tr, moved, target);
        } else {
            if (target.color != moved.color && target.type != CF_PIECE_KING)
                add_move(list, ff, fr, tf, tr, moved, target);
            break;
        }
        tf += df;
        tr += dr;
    }
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
    int i, dir, start_rank, next_rank;
    list->count = 0;
    if (board == 0 || !in_bounds(file, rank)) return;
    piece = board->squares[rank][file];
    if (piece.type == CF_PIECE_NONE) return;

    switch (piece.type) {
    case CF_PIECE_PAWN:
        dir = piece.color == CF_COLOR_WHITE ? 1 : -1;
        start_rank = piece.color == CF_COLOR_WHITE ? 1 : 6;
        next_rank = rank + dir;
        /* Build 4 owns promotion, so Build 3 stops pawns short of the last rank. */
        if (next_rank > 0 && next_rank < 7 && board->squares[next_rank][file].type == CF_PIECE_NONE) {
            add_move(list, file, rank, file, next_rank, piece, board->squares[next_rank][file]);
            if (rank == start_rank && board->squares[rank + dir * 2][file].type == CF_PIECE_NONE)
                add_move(list, file, rank, file, rank + dir * 2, piece, board->squares[rank + dir * 2][file]);
        }
        if (next_rank > 0 && next_rank < 7) {
            for (i = -1; i <= 1; i += 2) {
                int tf = file + i;
                if (in_bounds(tf, next_rank) && board->squares[next_rank][tf].type != CF_PIECE_NONE &&
                    board->squares[next_rank][tf].color != piece.color &&
                    board->squares[next_rank][tf].type != CF_PIECE_KING)
                    add_move(list, file, rank, tf, next_rank, piece, board->squares[next_rank][tf]);
            }
        }
        break;
    case CF_PIECE_KNIGHT:
        for (i = 0; i < 8; ++i)
            add_if_pseudo_legal(board, list, file, rank,
                                file + knight_offsets[i][0], rank + knight_offsets[i][1]);
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
            add_if_pseudo_legal(board, list, file, rank,
                                file + king_offsets[i][0], rank + king_offsets[i][1]);
        break;
    default:
        break;
    }
}

static void apply_raw(CfBoard *board, const CfMove *move)
{
    board->squares[move->to_rank][move->to_file] = move->moved;
    board->squares[move->from_rank][move->from_file] = make_piece(CF_PIECE_NONE, CF_COLOR_NONE);
}

void board_unmake_move(CfBoard *board, const CfMove *move)
{
    if (board == 0 || move == 0) return;
    board->squares[move->from_rank][move->from_file] = move->moved;
    board->squares[move->to_rank][move->to_file] = move->captured;
    board->side_to_move = board_other_color(board->side_to_move);
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
        apply_raw(&scratch, &pseudo.moves[i]);
        if (!board_is_in_check(&scratch, piece.color))
            add_move(list, pseudo.moves[i].from_file, pseudo.moves[i].from_rank,
                     pseudo.moves[i].to_file, pseudo.moves[i].to_rank,
                     pseudo.moves[i].moved, pseudo.moves[i].captured);
    }
}

int board_move_is_legal(const CfBoard *board, int from_file, int from_rank, int to_file, int to_rank)
{
    CfMoveList list;
    int i;
    board_generate_legal_moves(board, from_file, from_rank, &list);
    for (i = 0; i < list.count; ++i)
        if (list.moves[i].to_file == to_file && list.moves[i].to_rank == to_rank) return 1;
    return 0;
}

int board_make_move(CfBoard *board, int from_file, int from_rank, int to_file, int to_rank, CfMove *made_move)
{
    CfMoveList list;
    int i;
    if (board == 0) return 0;
    board_generate_legal_moves(board, from_file, from_rank, &list);
    for (i = 0; i < list.count; ++i) {
        if (list.moves[i].to_file == to_file && list.moves[i].to_rank == to_rank) {
            apply_raw(board, &list.moves[i]);
            if (made_move != 0) *made_move = list.moves[i];
            board->side_to_move = board_other_color(board->side_to_move);
            return 1;
        }
    }
    return 0;
}

int board_count_legal_moves(const CfBoard *board, CfPieceColor color)
{
    CfBoard scratch;
    CfMoveList list;
    int file, rank, total = 0;
    if (board == 0) return 0;
    scratch = *board;
    scratch.side_to_move = color;
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
