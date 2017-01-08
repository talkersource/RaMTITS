#ifndef _CASINO_C
  #define EXTERN extern
#else
  #define EXTERN
#endif

EXTERN Pgame create_pgame(void);
EXTERN void destruct_pgame(Pgame);
EXTERN Pplayer create_pplayer(Pgame);
EXTERN void destruct_pplayer(Pplayer);
EXTERN Pgame get_pgame(char *);
EXTERN Pgame get_pgame_here(Room);
EXTERN void shuffle_cards(int []);
EXTERN void hand_poker(User,char *,int);
EXTERN void print_hand(User,int []);
EXTERN void room_print_hand(User,int []);
EXTERN void start_poker(User,char *,int);
EXTERN void join_poker(User,char *,int);
EXTERN void leave_poker(User,char *,int);
EXTERN void list_games(User,char *,int);
EXTERN void deal_poker(User,char *,int);
EXTERN void next_poker_player(Pgame);
EXTERN void next_in_player(Pgame);
EXTERN void bet_poker(User,char *,int);
EXTERN void bet_poker_aux(User,int);
EXTERN void raise_poker(User,char *,int);
EXTERN void see_poker(User,char *,int);
EXTERN void all_called_check(Pgame);
EXTERN void pass_the_deal(Pgame);
EXTERN void fold_poker(User,char *,int);
EXTERN void check_poker(User,char *,int);
EXTERN void bet_message(Pgame);
EXTERN void discard_poker(User,char *,int);
EXTERN void showdown_poker(Pgame);
EXTERN void swap_cards(int [],int,int);
EXTERN void show_players(User,char *,int);
EXTERN int myRand(int);
EXTERN void slots(User,char *,int);
EXTERN void black_jack(User user,char *inpstr,int rt);
EXTERN Bjack create_bjgame(void);
EXTERN void shuffle_bjcards(Bjack);
EXTERN void destruct_bjgame(User);
EXTERN void show_bjack_cards(User,int,int);
EXTERN int check_bjack_total(User,int);
EXTERN void show_bjack_status(User);
EXTERN void PlayHighDice(User,char *,int);
EXTERN void play_triples(User,char *,int);
EXTERN void play_seven(User,char *,int);
EXTERN void play_craps(User,char *,int);
EXTERN int add_ticket(char *,int,int,int,int,int,int);
EXTERN int remove_ticket(char *);
EXTERN void clear_tickets(void);
EXTERN int save_lotto(void);
EXTERN int load_lotto(void);
EXTERN int already_has_numbers(User,int,int,int,int,int,int);
#undef EXTERN
