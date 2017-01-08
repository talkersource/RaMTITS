#ifndef _CHESS_C
  #define EXTERN extern
#else
  #define EXTERN
#endif

EXTERN void game_summary(User);
EXTERN void view_pieces(User);
EXTERN void tell_chess(Chess,char *);
EXTERN void init_board(User,Chess);
EXTERN void invalid(Chess,int,int,int,int);
EXTERN int move_white_pawn(Chess,int,int,int,int);
EXTERN int move_black_pawn(Chess,int,int,int,int);
EXTERN int move_white_knight(Chess,int,int,int,int);
EXTERN int move_black_knight(Chess,int,int,int,int);
EXTERN int move_white_rook(Chess,int,int,int,int);
EXTERN int move_black_rook(Chess,int,int,int,int);
EXTERN int move_white_bishop(Chess,int,int,int,int);
EXTERN int move_black_bishop(Chess,int,int,int,int);
EXTERN int move_white_queen(Chess,int,int,int,int);
EXTERN int move_black_queen(Chess,int,int,int,int);
EXTERN int move_white_king(Chess,int,int,int,int);
EXTERN int move_black_king(Chess,int,int,int,int);
EXTERN void ViewBoard(User,char *);
EXTERN void chess_com(User,char *,int);
EXTERN void take_back(User,char *);
EXTERN void move_pieces(User,char *,int);
EXTERN void kill_chess(User);
EXTERN int white_in_check(Chess);
EXTERN int black_in_check(Chess);
EXTERN int white_stalemate(Chess);
EXTERN int black_stalemate(Chess);
EXTERN int white_is_checkmate(Chess);
EXTERN int black_is_checkmate(Chess);
#undef EXTERN
