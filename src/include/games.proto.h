#ifndef _GAMES_C
  #define EXTERN extern
#else
  #define EXTERN
#endif
EXTERN int game_ops(User,char *);
EXTERN void tictac(User,char *,int);
EXTERN int legal_tic(char *,int);
EXTERN int win_tic(char *);
EXTERN void print_tic(User);
EXTERN void reset_game(User,int);
EXTERN void connect_four(User,char *,int);
EXTERN void print_board(User);
EXTERN int connect_win(User);
EXTERN void hangman(User,char *,int);
EXTERN void fight_another(User,char *,int);
EXTERN void issue_challenge(User,User);
EXTERN void accept_challenge(User);
EXTERN void trivia(User,char *,int);
EXTERN void flip_coin(User,char *,int);
EXTERN void play_puzzle(User,char *,int);
EXTERN void init_table(User);
EXTERN void puzzle_status(User);
EXTERN void puzzle_swap(User,int,int,int,int);
EXTERN void puzzle_quit(User);
EXTERN void puzzle_play(User,int);
EXTERN int game_solved(User);
EXTERN char *get_word(void);
#undef EXTERN
