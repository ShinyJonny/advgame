// The pre-set terminal size

#define TERM_WIDTH 80
#define TERM_HEIGHT 24

// The width of the map window.

#define WIDTH TERM_WIDTH
#define HEIGHT (TERM_HEIGHT-2)

// The popup window size.

#define WINDOW_WIDTH ((int) (WIDTH * 0.8f))
#define WINDOW_HEIGHT ((int) (HEIGHT * 0.85f))

// The map frame buffer

typedef char map[HEIGHT][WIDTH];

// Status line for messages to the user.

#define STATUS_MSG_LEN ((WIDTH-2) - 22)

// Directions

#define LEFT_C 'h'
#define LEFT_NUM '4'
#define RIGHT_C 'l'
#define RIGHT_NUM '6'
#define UP_C 'k'
#define UP_NUM '8'
#define DOWN_C 'j'
#define DOWN_NUM '2'
#define UPLEFT_C 'y'
#define UPLEFT_NUM '7'
#define UPRIGHT_C 'u'
#define UPRIGHT_NUM '9'
#define DOWNLEFT_C 'b'
#define DOWNLEFT_NUM '1'
#define DOWNRIGHT_C 'n'
#define DOWNRIGHT_NUM '3'

// Escape sequences

#define CURSOR_UP "\033[%dA"
#define CURSOR_DOWN "\033[%dB"
#define CURSOR_FORWARD "\033[%dC"
#define CURSOR_BACK "\033[%dD"
#define CURSOR_SET "\033[%d;%dH"

// Cursor movement macros

#define SET_CURSOR_END { printf(CURSOR_BACK, TERM_WIDTH); printf(CURSOR_FORWARD, TERM_WIDTH -1); }

// Colors

//#define MYBLUE "\033[38;2;070;122;255m"
#define BLUE		"\033[0034m"
#define BROWN		"\033[0033m"
#define GOLD		"\033[1;33m"
#define GREY		"\033[1;30m"
#define RED		"\033[0031m"
#define B_RED		"\033[1;31m"
#define B_BLUE		"\033[1;34m"
#define B_GREEN		"\033[1;32m"
#define FG		"\033[0000m"

// Lengths

#define HTML_ESC_LEN 19
#define ESC_LEN 7
//#define FG_LEN 5
//#define BG_LEN 5
//#define FG_BG_LEN 8
//#define BOLD_FG_LEN 7
//#define BOLD_BG_LEN 7
//#define BOLD_FG_BG_LEN 10
