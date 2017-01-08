/* Some fixed-length VT100 control code strings, with their lengths. */
#define VT100_CURSOR_SAVE		"\0337"
#define VT100_CURSOR_SAVE_LEN		2
#define VT100_CURSOR_RESTORE		"\0338"
#define VT100_CURSOR_RESTORE_LEN	2
#define VT100_CURSOR_ON			"\033[?25h"
#define VT100_CURSOR_OFF		"\033[?25l"

/* Some VT100 control code strings that require extra parameters */
#define VT100_SET_REGION		"\033[%d;%dr"
#define VT100_CURSOR_SET		"\033[%d;%df"
#define VT100_XY			"\033[%d;%dH"

/* A few Telnet control codes, fixed length strings, with their lengths */
#define TELNET_ECHO_ON			(char *)"\255\252\1"
#define TELNET_ECHO_ON_LEN		3
#define TELNET_ECHO_OFF			(char *)"\255\251\1"
#define TELNET_ECHO_OFF_LEN		3
#define TELNET_LINEMODE_ON		(char *)"\777\774\42"
#define TELNET_LINEMODE_ON_LEN		3
#define TELNET_LINEMODE_OFF		(char *)"\777\773\42"
#define TELNET_LINEMODE_OFF_LEN		3

/* Some other defines... */
#define CACHE_SIZE	2048
#define CDIVIDOR	"\033[0;32m+-= \033[0;36m%-13.13s \033[0;32m=-= \033[0;36mOn: \033[0;35m%-3d \033[0;32m- \033[0;36mIdle: \033[0;35m%-2d \033[0;32m=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+"
#define DIVIDOR		"\033[0;47m\033[30m+-= %-13.13s =-= On: %-3d - Idle: %-2d =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+"
#define CEDDIVIDOR	"\033[0;32m+-= \033[0;36m%-13.13s \033[0;32m=-= \033[0;36mEditing: \033[0;35m%-15s \033[0;32m=-=-=-=-=-=-=-=-=-=-=-=-=-=-+"
#define EDDIVIDOR	"\033[0;47m\033[30m+-= %-13.13s =-= Editing: %-15s =-=-=-=-=-=-=-=-=-=-=-=-=-=-+"
