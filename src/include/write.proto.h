#ifndef _WRITE_SOURCE
  #define EXTERN extern
#else
  #define EXTERN
#endif
EXTERN int write_window(User,char *,int,int);
EXTERN int write_socket(int,char *,int);
EXTERN void get_pane_size(User,int,int *,int *);
EXTERN void set_pane(int,int,int);
EXTERN void save_cursor(int);
EXTERN void restore_cursor(int);
EXTERN void set_cursor(int,int,int);
EXTERN void splitscreen_on(User);
EXTERN void splitscreen_off(User);
EXTERN void show_prompt(User);
EXTERN void prompt(User);
EXTERN void echo_off(User);
EXTERN void echo_on(User);
EXTERN void write_user(User,char *);
EXTERN void write_sock(int,char *);
EXTERN int write_sock_nr(User,char *);
EXTERN void writeus(User,char *, ...);
EXTERN void WriteRoom(Room,char *, ...);
EXTERN void WriteRoomExcept(Room,User,char *, ...);
EXTERN void WriteLevel(int,User,char *, ...);
EXTERN void write_level(int,User,char *);
EXTERN void write_room(Room,char *);
EXTERN void write_room_except(Room,User,char *);
EXTERN void write_room_except2(Room,User,User,char *);
EXTERN void write_monitor(User,int,Room);
EXTERN void write_log(int,int,const char *, ...);
EXTERN void write_record(User,int,const char *, ...);
EXTERN void write_friends(User,char *);
EXTERN int more(User,int,char *,int);
EXTERN void fill_pages(User,char *);
EXTERN int show_screen(User);
EXTERN int view_file(User,char *);
EXTERN void will_time_mark(User);
EXTERN void init_inputbuffer(User);
EXTERN int add_char_internal(User,char);
EXTERN int addcharto_inputbuff(User,char);
EXTERN int addstring_tobuff(User,char *,int);
EXTERN void remove_from_inputbuff(User,int);
EXTERN void getbuffer_char_fromend(User,int,char *);
EXTERN int getline_from_inputbuff(User,char *);
EXTERN int newline_inbuff(int);
EXTERN void release_inputbuff(User);
EXTERN void clear_inputbuff(User);
EXTERN void log_telnetseq(User);
EXTERN Convbuff newconv_buffer(void);
EXTERN void clear_convbuff(Convbuff);
EXTERN void addto_convbuff(Convbuff,char *);
EXTERN void write_convbuff(User,Convbuff);
EXTERN void filewrite_convbuff(FILE *,Convbuff);
EXTERN int count_dir(char *);
EXTERN void write_dir(User,char *);
#undef EXTERN
