#include <stdio.h>
#include <stdlib.h> // For rand() functions.
#include <string.h> // For string operations.
#include <stdbool.h> // For boolean variables, to save some space.
#include <time.h> // For time() function.
#include <termios.h> // To set terminal properties.
#include <unistd.h> // For FILENO definitions.
#include <ctype.h> // For checks of characters.
#include "include/actors.h" // Defining the actors struct and its properties.
#include "include/general.h" // General-purpose definitions.
#include "include/printtui.h" // Defining the properties of a printf tui.

// main() is at: 140

/* Not implemented yet.
#define SPELL_COST 5
#define PROJECTILE_CHAR 'o'
*/

// Starting positions of the user.

#define STARTX (WIDTH-1 -2)
#define STARTY (HEIGHT/2)

#define NUM_OF_LEVELS 4
#define NUM_OF_MOBS 4

// Defining indexes for all mobs.

#define USER 0 // self-explanatory
#define MOB1 1 // Goblin
#define MOB2 2 // Kobold
#define MOB3 3 // Human Warrior

Actor Mobs[NUM_OF_MOBS]; // Creating an array of these structs, so I can loop through them.

/*
 * Secondary frame, meant for saving parts of the main map in case that I need to print a
 * window to the main frame. (e.g. an inventory window covering half of the map) The part
 * That would be ovewriten by a window (e.g. inventory) will be saved here and then pulled
 * back. (Currently no advanced features like inventory are implemented but it still serves
 * as a saving place) At the beginning there is the main map.
 */

// Y coordinates need to be the first dimension, so that I can use each row as a string.

map second_frame = { 
	"################################################################################",
	"#                                                                         #....#",
	"#                                                                         #....#",
	"#                                                                         #....#",
	"#        ############################                                     #....#",
	"#        #.............##         ####                     ########       #....#",
	"#        ##...........##           ####                    #......#       #....#",
	"#         #############             ####                   #......#       #....#",
	"#                                    #############       ###......#       #....#",
	"#                                         ##.....#       #........#       ######",
	"#                                          ##....#       #........#            #",
	"##################################          ######       #######..#            #",
	"#  $  #                        #.##         ##                 ##.#            #",
	"#     #                        #..##        ##                  ################",
	"###                            #...#        ##                                ##",
	"#                              #...#        ##                                 #",
	"#                              #...#        ##                  #########      #",
	"#                              #..##        ##############################     #",
	"#                              ####         #...........##        #######      #",
	"#                                           #..........##                      #",
	"#                                           #.........##                      ##",
	"################################################################################"
};

/*
 * The main frame containing the thing that will be printed to the screen after an action happens.
 * At the beginning there is a greeting screen that immediately gets ovewriten after displaying.
 * (by restoring the main map from the secondary frame)
 */

map main_frame = {
	"################################################################################",
	"#                                                                              #",
	"#                                                                              #",
	"#                                                                              #",
	"#        __        __   _                                _                     #",
	"#        \\ \\      / /__| | ___ ___  _ __ ___   ___      | |_ ___               #",
	"#         \\ \\ /\\ / / _ \\ |/ __/ _ \\| '_ ` _ \\ / _ \\     | __/ _ \\              #",
	"#          \\ V  V /  __/ | (_| (_) | | | | | |  __/     | || (_) |             #",
	"#           \\_/\\_/ \\___|_|\\___\\___/|_| |_| |_|\\___|      \\__\\___/              #",
	"#                                                                              #",
	"#                                                                              #",
	"#                                                                              #",
	"#                  _       _        ____                                       #",
	"#                 / \\   __| |_   __/ ___| __ _ _ __ ___   ___                  #",
	"#                / _ \\ / _` \\ \\ / / |  _ / _` | '_ ` _ \\ / _ \\                 #",
	"#               / ___ \\ (_| |\\ V /| |_| | (_| | | | | | |  __/                 #",
	"#              /_/   \\_\\__,_| \\_/  \\____|\\__,_|_| |_| |_|\\___|                 #",
	"#                                                                              #",
	"#                                                                              #",
	"#                                   V 1.2             Made by: ShinyJonny      #",
	"#                                                                              #",
	"################################################################################"
};

char status_msg[STATUS_MSG_LEN+1]; // The message part of the status line.

//                    Map chars    newlines   Escapes before and after
char print_buffer[(HEIGHT * WIDTH + HEIGHT) * (HTML_ESC_LEN * 2)]; // The buffer for printing onto the screen.
long int pb_index = 0; // Index into print_buffer.

bool win_lvl = false; // Determines whether the level is beaten.
bool win_status = false; // Determines whether the whole game is beaten. Currently serves no purpose.

int dlvl; // Holding the Dungeon Level number (currently serves no purpose)
bool color_mode = true;

struct termios default_term; // Original state of the terminal.

// Prototypes

void init_Mobs();
void greet();
void print_main_frame();
void print_status();
void spawn(int Actor_num, int x_position, int y_position);
void restore_main_frame(int baseX, int baseY, int x_portion, int y_portion);
void save_main_frame(int baseX, int baseY, int x_portion, int y_portion);
char interactive_getchar();
void do_action(char action_char);
void move(int Actor_id, char direction, int count);
void get_coord_adjustment(int *x_coord, int *y_coord, char direction);
void flush_status_msg();
void set_status_msg(const char *line);
void attack(int attacker, int x_of_enemy, int y_of_enemy);
void die(int dead_actor);
//void hitback(int attacker, int victim);
void print_help();
void load_map(FILE *map_file);
//void follow_actor(int follower, int followed_one); // no body yet
void random_spawn(int spawned_actor, int min_x, int max_x, int min_y, int max_y);
void random_move(int Actor_id, int count);
void hilight_actor(int actor_id);
//void teleport();

int main(void)
{
	// Saving the initial terminal state.

	tcgetattr(STDIN_FILENO, &default_term);

	// Initialising the status message with spaces. 

	flush_status_msg();

	// Greeting the user and getting the name for the avatar.

	greet();

	interactive_getchar(); // Getting rid of a trailing newline.

	// Pulling the map to the main frame. (From 0:0 and spanning the whole width and height)

	restore_main_frame(0, 0, WIDTH, HEIGHT);

	// Initialising all the actors with their default values. (including USER)

	init_Mobs();

	// Displaying the help.txt file.

	print_help();

	// Spawning the player and the mobs.

	spawn(USER, STARTX, STARTY);
	random_spawn(MOB1, 40, 58, 1, 6); // Goblin
	random_spawn(MOB2, 46, 62, 12, 16); // Kobold
	random_spawn(MOB3, 1, 29, 12, 20); // Human Warrior

	/* The main loop.
	 * 
	 * The main frame is refreshed, the status line is displayed under it.
	 * Then the program waits for input.
	 * An action is performed according to the input.
	 * Then, if they are alive, the mobs move in a random direction.
	 * The function exits if the user dies or wins.
	 */

	while(Mobs[USER].hp > 0 && win_lvl == false)
	{
		// Printing the main frame and the status-line, and flushing the status-line.

		print_main_frame();
		print_status();
		flush_status_msg();

		// Getting the action from the user and performing it.

		char action = interactive_getchar();
		do_action(action);

		// Looping through all the non-user actors (1-3) and moving them randomly.

		for(int mob = 1; mob < NUM_OF_MOBS; mob++)
		{
			if (Mobs[mob].hp > 0) {
				random_move(mob, 1);
			}
		}
	}

	// Opening the victory or the loss file.

	FILE *bye_user_file;

	if (win_lvl == true) { // Needs to be changes in multilevel!!!
		bye_user_file = fopen("maps/victory.txt", "r");
	}
	else {
		bye_user_file = fopen("maps/loss.txt", "r");
	}

	char error_read[] = " Error, couldn't read loss.txt or victory.txt";
	char exit_msg[] = " Press any key to EXIT.";

	// Loading the bye file to the main frame, closing it, printing it, updating and
	// printing the status, and waiting for a key press.

	if (bye_user_file == NULL) {
		set_status_msg(error_read);
	}
	else {

		load_map(bye_user_file);
		fclose(bye_user_file);
	
		set_status_msg(exit_msg);
	}

	print_main_frame();
	print_status();

	interactive_getchar();
	
	// Printing the missing \n, so that the terminal prompt is on a new line.

	printf("\n");

	// Finish.

	return 0;
}

// A function to initialise the actors with default values.

void init_Mobs()
{
	// Initialising the user.

	Mobs[USER].symbol = '@';
	// Name will be initialised later.
	Mobs[USER].power = 10;
	Mobs[USER].hp = 24;
	Mobs[USER].mp = 20;
	// Starting positions
	// not initialised.
	Mobs[USER].actor_type = INTERACTIVE; // Only the user is not a bot and is interactive.

	// Initialising the Goblin.

	Mobs[MOB1].symbol = 'G';
	strcpy(Mobs[MOB1].actor_name, "Goblin\0");
	Mobs[MOB1].power = 3;
	Mobs[MOB1].hp = 10;
	Mobs[MOB1].mp = 0;
	// Starting positions
	// not initialised.
	Mobs[MOB1].actor_type = NONINTERACTIVE;

	// Initialising the Kobold.

	Mobs[MOB2].symbol = 'K';
	strcpy(Mobs[MOB2].actor_name, "Kobold\0");
	Mobs[MOB2].power = 5;
	Mobs[MOB2].hp = 14;
	Mobs[MOB2].mp = 0;
	// Starting positions
	// not initialised.
	Mobs[MOB2].actor_type = NONINTERACTIVE;

	// Initialising the Human Warrior.

	Mobs[MOB3].symbol = 'W';
	strcpy(Mobs[MOB3].actor_name, "Human Warrior\0");
	Mobs[MOB3].power = 7;
	Mobs[MOB3].hp = 30;
	Mobs[MOB3].mp = 20;
	// Starting positions
	// not initialised.
	Mobs[MOB3].actor_type = NONINTERACTIVE;

	return;
}

// A function to print the greeting and to get the hero's name.

void greet()
{
	char user_name[NAME_LEN+1]; // Temporary name storage.

	// Create a space for the window frames, so that no previous
	// output is overwritten.

	for(int i = 0; i < TERM_HEIGHT; i++)
	{
		printf("\n");
	}

	print_main_frame();

	printf("What is your name, hero?\n");
	printf("> ");
	for(int i = 0; i < NAME_LEN; i++) {
		printf("_");
	}
	for(int i = 0; i < NAME_LEN; i++) {
		printf("\b");
	}

	scanf("%" NAME_LEN_STR "s", user_name);

	// Adjustment to get to the desired line.
	
	printf(CURSOR_UP, 1);

	// Saving int user's actor_name.

	strcpy(Mobs[USER].actor_name, user_name);

	return;
}

// A function to print the main frame.

void print_main_frame()
{
	printf("\n" CURSOR_UP, TERM_HEIGHT);

	//pb_index = 0;

	for(int row = 0; row < HEIGHT; row++)
	{
		for(int column = 0; column < WIDTH; column++)
		{
			if (color_mode == true) {
				switch(main_frame[row][column]) {
				case '@':
					//strcpy(print_buffer + pb_index, GREY "@" FG);
					//pb_index += (ESC_LEN * 2) + 1;
					printf(GREY "@" FG);
					break;
				case '$':
					//strcpy(print_buffer + pb_index, GOLD "$" FG);
					//pb_index += (ESC_LEN * 2) + 1;
					printf(GOLD "$" FG);
					break;
				case 'W':
					//strcpy(print_buffer + pb_index, B_RED "W" FG);
					//pb_index += (ESC_LEN * 2) + 1;
					printf(B_RED "W" FG);
					break;
				case 'G':
					//strcpy(print_buffer + pb_index, B_GREEN "G" FG);
					//pb_index += (ESC_LEN * 2) + 1;
					printf(B_GREEN "G" FG);
					break;
				case 'K':
					//strcpy(print_buffer + pb_index, B_BLUE "K" FG);
					//pb_index += (ESC_LEN * 2) + 1;
					printf(B_BLUE "K" FG);
					break;
				default:
					//print_buffer[pb_index] = main_frame[row][column];
					//pb_index++;
					printf("%c", main_frame[row][column]);
					break;
				}
			}
			else {
				//print_buffer[pb_index] = main_frame[row][column];
				//pb_index++;
				printf("%c", main_frame[row][column]);
			}
		}

		//print_buffer[pb_index] = '\n';
		//pb_index++;
		printf("\n");
	}

	// Terminating the string in the buffer and printing it.

	//print_buffer[pb_index] = '\0';

	//printf("%s", print_buffer);

	return;
}

// A function to print the stats of the user and the status-line.

void print_status()
{
	printf("# HP:%2d| MP:%2d| DLV:%2d|%s#\n", Mobs[USER].hp, Mobs[USER].mp, dlvl, status_msg);

	int i;
	for(i = 0; i < WIDTH; i++)
	{
		print_buffer[i] = '#';
	}
	print_buffer[i] = '\0';
	
	printf("%s", print_buffer);

	// Moving the cursor vertically to last position of the given TERM_WIDTH.

	printf(CURSOR_BACK, TERM_WIDTH);
	printf(CURSOR_FORWARD, TERM_WIDTH -1);

	return;
}

// A function to set an actor's X and Y coordinates and to print the actor to the map.

void spawn(int Actor_num, int x_position, int y_position)
{
	// Printing the actor to the map.

	main_frame[y_position][x_position] = Mobs[Actor_num].symbol;

	// Updating the actor's position variables.

	Mobs[Actor_num].xpos = x_position;
	Mobs[Actor_num].ypos = y_position;

	return;
}

/*
 * A function to pull a portion of the secondary frame (back) to the main frame.
 * baseX - The lowest X coordinate where the desired frame starts.
 * x_portion - the range that the frame spans. (on X)
 * The same is for Y.
 */


void restore_main_frame(int baseX, int baseY, int x_portion, int y_portion)
{
	// The functions are looping from 0 for the whole portion of the window
	// but the pulling back is offset with the base values.

	for(int row = 0; row < y_portion; row++)
	{
		for(int column = 0; column < x_portion; column++)
		{// The values are offset:                                    |here|       |here|
			main_frame[baseY + row][baseX + column] = second_frame[baseY + row][baseX + column];
		}
	}

	return;
}

/*
 * The same function as restore_main_frame() but to do the opposite.
 */

void save_main_frame(int baseX, int baseY, int x_portion, int y_portion)
{
	for(int row = 0; row < y_portion; row++)
	{
		for(int column = 0; column < x_portion; column++)
		{
			second_frame[baseY + row][baseX + column] = main_frame[baseY + row][baseX + column];
		}
	}

	return;
}

/*
 * This function sets the terminal settings to raw and turns off echoeing, so that it can
 * recieve input as soon as the user hits a key and so that the character is not displayed.
 * Then it turns the settings to normal values.
 */

char interactive_getchar()
{
	// Copying the initial state.

	struct termios raw_term = default_term;

	// Disabling echoeing and buffering of the input.

	raw_term.c_lflag &= ~(ECHO | ICANON);

	// Setting the properties, getting the char and restoring.

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_term);
	char return_char = getchar();
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &default_term);

	return return_char;
}

// A function to fill the status-line with zeroes and to terminate it with NULL again.

void flush_status_msg()
{
	for(int i = 0; i < STATUS_MSG_LEN; i++)
	{
		status_msg[i] = ' ';
	}
	status_msg[STATUS_MSG_LEN] = '\0';

	return;
}

// A function to flush the status-line and fill it with line.

void set_status_msg(const char *line)
{
	for(int i = 0; i < STATUS_MSG_LEN; i++)
	{
		status_msg[i] = ' ';
	}
	status_msg[STATUS_MSG_LEN] = '\0';

	strcpy(status_msg, line);
	status_msg[strlen(line)] = ' ';
}

// This function evaluates what to do according to the character typed ty the user.

void do_action(char action_char)
{
	// The status_msg for the default action.

	char invalid[] = " Invalid command!";

	switch(action_char)
	{
		// Movement to all the 8 directions.

		case LEFT_C:
		case LEFT_NUM: 
			move(USER, action_char, 1);
			break;
		case DOWN_C:
		case DOWN_NUM:
			move(USER, action_char, 1);
			break;
		case UP_C:
		case UP_NUM:
			move(USER, action_char, 1);
			break;
		case RIGHT_C:
		case RIGHT_NUM:
			move(USER, action_char, 1);
			break;
		case UPLEFT_C:
		case UPLEFT_NUM:
			move(USER, action_char, 1);
			break;
		case UPRIGHT_C:
		case UPRIGHT_NUM:
			move(USER, action_char, 1);
			break;
		case DOWNLEFT_C:
		case DOWNLEFT_NUM:
			move(USER, action_char, 1);
			break;
		case DOWNRIGHT_C:
		case DOWNRIGHT_NUM:
			move(USER, action_char, 1);
			break;

		// Wait

		case '.':
			break;

		/*
		// Teleport

		case 'T':
			teleport();
			break;
		*/

		// Display help

		case 'H':
			print_help();
			break;

		// Quit

		case 'Q':
			die(USER);
			break;

		// The default action - did not understand.
			
		default:
			set_status_msg(invalid);
			/* [debug]
			memset(status_msg, ' ', STATUS_MSG_LEN);
			status_msg[STATUS_MSG_LEN] = '\0';

			sprintf(status_msg, "%s %02x", invalid, action_char);
			status_msg[strlen(status_msg)] = ' ';
			*/

	}

	return;
}

// A function that performs a movement of an actor in a given direction by a given count of
// steps.

void move(int Actor_id, char direction, int count)
{
	char invalid[] = " Cannot move there."; // Self-explanatory msg.

	int x_adjust = 0;
	int y_adjust = 0;

	// Getting the coordinate adjustment.

	get_coord_adjustment(&x_adjust, &y_adjust, direction);

	// Multiplying the adjustment by the count of steps.

	x_adjust *= count;
	y_adjust *= count;

	// Adding it to the original coordinates.

	int destination_x = Mobs[Actor_id].xpos + x_adjust;
	int destination_y = Mobs[Actor_id].ypos + y_adjust;

	// The character that is currently at the desired destination is saved.

	char dest_char = main_frame[destination_y][destination_x];

	/*
	 * If the there is a space at the destination, the actor is free to move in.
	 * If there is an actor, (upper-case letter or '@') then it is attacked.
	 * If there is the exit from the level and the actor is the user, then
	 * win_lvl is set to true.
	 * Else an error status_msg is displayed. (but only if the actor is the user, we
	 * don't want the mobs to trigger "You cannot move there." status_msgs.)
	 */

	if (dest_char == ' ') {

		// Clearing the former position of the actor and writing the actor's symbol
		// to the destination.

		main_frame[Mobs[Actor_id].ypos][Mobs[Actor_id].xpos] = ' ';
		main_frame[destination_y][destination_x] = Mobs[Actor_id].symbol;

		// Updating the actor's X and Y positions values.

		Mobs[Actor_id].xpos = destination_x;
		Mobs[Actor_id].ypos = destination_y;
	}

	// In boundaries of uppercase character or '@' causes attack. (friendly-fire enabled)

	//else if ((((int) dest_char >= UPPER_CASE_LOW && (int) dest_char <= UPPER_CASE_HIGH) || dest_char == '@')) {
	else if (isupper(dest_char) || dest_char == '@') {
		attack(Actor_id, destination_x, destination_y); // self-explanatory
	}
	else if (dest_char == '$' && Actor_id == USER) {

		// Even though we won the level, we still want to move there because of
		// the following levels.

		Mobs[Actor_id].xpos = destination_x;
		Mobs[Actor_id].ypos = destination_y;
		win_lvl = true;
	}
	else if (Actor_id == USER) {
		set_status_msg(invalid);
	}

	return;
}

// A function to adjust x coordinates when an actor steps in a direction.

void get_coord_adjustment(int *x_adjust, int *y_adjust, char direction)
{

	// All 8 directions - the adjustments are either incremented, decremented,
	// or left untouched.

	switch(direction)
	{
		case LEFT_C:
		case LEFT_NUM: 
			*x_adjust = -1;
			break;
		case DOWN_C:
		case DOWN_NUM:
			*y_adjust = 1;
			break;
		case UP_C:
		case UP_NUM:
			*y_adjust = -1;
			break;
		case RIGHT_C:
		case RIGHT_NUM:
			*x_adjust = 1;
			break;
		case UPLEFT_C:
		case UPLEFT_NUM:
			*x_adjust = -1;
			*y_adjust = -1;
			break;
		case UPRIGHT_C:
		case UPRIGHT_NUM:
			*x_adjust = 1;
			*y_adjust = -1;
			break;
		case DOWNLEFT_C:
		case DOWNLEFT_NUM:
			*x_adjust = -1;
			*y_adjust = 1;
			break;
		case DOWNRIGHT_C:
		case DOWNRIGHT_NUM:
			*x_adjust = 1;
			*y_adjust = 1;
			break;
	}

	return;
}

// A function that sorts attacking and, if the victim is non-interactive, it also hits back.

void attack(int attacker, int x_of_enemy, int y_of_enemy)
{
	char attack_msg[STATUS_MSG_LEN] = " "; // The status-line string.

	// The string is different for the user and for mobs.

	if (attacker == USER) {
		strcat(attack_msg, "You hit ");
	}
	else {
		strcat(attack_msg, Mobs[attacker].actor_name);
		strcat(attack_msg, " hits ");
		// E.g. " Goblin hits "
	}

	// This loop loops through all the actors and the one that is at the attacked coordinate is hit.

	for(int cur_actor = 0; cur_actor < NUM_OF_MOBS; cur_actor++)
	{
		if (Mobs[cur_actor].xpos == x_of_enemy && Mobs[cur_actor].ypos == y_of_enemy) {

			// Victims HP minus Attackers power.

			Mobs[cur_actor].hp -= Mobs[attacker].power;

			// Adding the victim's name to the status_msg and pushing it to the status-line.

			strcat(attack_msg, Mobs[cur_actor].actor_name);
			strcat(attack_msg, "!");

			set_status_msg(attack_msg);

			// If the victim's hp drops below/to 0, it dies.

			if (Mobs[cur_actor].hp <= 0) {
				die(cur_actor);
			}
			/*else if (Mobs[cur_actor].actor_type == NONINTERACTIVE) {
				hitback(cur_actor, attacker);
			}*/
		}
	}
}

// A function that attacks back the former attacker.

/*
void hitback(int attacker, int victim)
{
	// Creating the status_msg (attacker's name + " hits back!" + ' ')
	// It overwrites the attack() status_msg.

	char fin_msg[STATUS_MSG_LEN] = " ";
	strcat(fin_msg, Mobs[attacker].actor_name);
	strcat(fin_msg, " hits back!");

	set_status_msg(fin_msg);

	// Victim's HP - attacker's power.

	Mobs[victim].hp -= Mobs[attacker].power;

	// Again, death if the victim drops HP below/to 0.

	if (Mobs[victim].hp <= 0) {
		die(victim);
	}
}
*/

// A function that makes a character die: clears it from the map and sets its HP to 0
// to make sure it is dead.

void die(int dead_actor)
{
	// Setting the HP to 0.

	Mobs[dead_actor].hp = 0;

	// Setting up the status_msg. (dead actor's name " dies." + ' ')
	// It overwrites the attack() or hitback() status_msg.

	char fin_msg[STATUS_MSG_LEN] = " ";
	strcat(fin_msg, Mobs[dead_actor].actor_name);
	strcat(fin_msg, " dies.");

	set_status_msg(fin_msg);

	// Clearing the actor from the map.

	main_frame[Mobs[dead_actor].ypos][Mobs[dead_actor].xpos] = ' ';
	return;
}

// A function to print the in-game help.

void print_help()
{
	// Messages for two different scenarios.

	char error_msg[] = " Error, couldn't read help.txt.";
	char valid_msg[] = " Press any key to continue.";

	// Attempt to open the "help.txt" file.

	FILE *help_file = fopen("maps/help.txt", "r");

	// If failed, set up the status status_msg accordingly and return.

	if (help_file == NULL) {
		set_status_msg(error_msg);
		return;
	}

	// We need to save the map. (From 0:0 and spanning the whole width and height)

	save_main_frame(0, 0, WIDTH, HEIGHT);

	// Loading the help file and closing it.

	load_map(help_file);
	fclose(help_file);

	// Setting up the status status_msg with " Press any key to continue."

	set_status_msg(valid_msg);

	// Printing the frame and status and immediately clear the status line.

	color_mode = false; // Disabling color.
	print_main_frame();
	color_mode = true; // Enabling color.

	print_status();
	flush_status_msg();

	interactive_getchar(); // Needed for the status_msg to stay until the user presses a key.

	// Finally restoring the map (From 0:0 and spanning the whole width and height)

	restore_main_frame(0, 0, WIDTH, HEIGHT);

	return;
}

// A function to read a file's contents into the main frame.

void load_map(FILE *map_file)
{
	char cur_char; // Holding the current character.

	// Go through the whole width and height of the main frame and read a character from the file
	// to the main frame at the same coordinates.

	for(int row = 0; row < HEIGHT; row++)
	{
		for(int column = 0; column < WIDTH; column++)
		{
			fscanf(map_file, "%c", &cur_char);
			main_frame[row][column] = cur_char;
		}

		fscanf(map_file, "%c", &cur_char); // Scanning away the \n.
	}

	return;
}

// A function to randomly spawn an actor in a limited area.
// The function is given a minimum X value, a minimum Y value and also maximum X and Y values.

void random_spawn(int spawned_actor, int min_x, int max_x, int min_y, int max_y)
{
	int final_x; // Variables to hold the final X and Y positions of the actor to be spawned at.
	int final_y;

	// This loop gets random coordinates in the given area and repeats if the spot is not free
	// i.e. it is not ' '.

	do
	{
		/*
		 * Using clock() instead of time(NULL) because it returns milliseconds.
		 * At first I was using time() but it returns seconds and the random number stays the
		 * same until the current second passes. This caused that the random number was same
		 * for all the mobs because they all executed before a second passed.
		 * This was especially annoying for random movement because all mobs moves in the same
		 * direction.
		 */

		// [2] [reference]

		// Seeding the random number.

		long long int x_milis = clock();
		srand(x_milis);

		// Subtracting the minimal value from the maximal, so that I can get the pure range, and
		// getting a random number in that range. Then adding the minimal value.

		final_x = rand() % (max_x - min_x +1);
		final_x += min_x;

		// The same here for Y value.

		long long int y_milis = clock();
		srand(y_milis);

		final_y = rand() % (max_y - min_y +1);
		final_y += min_y;
	}
	while(main_frame[final_y][final_x] != ' '); // While it is not a free space.

	// Spawn at those coordinates.

	spawn(spawned_actor, final_x, final_y);

	return;
}

// A function to move a given actor in a rangom direction.

void random_move(int Actor_id, int count)
{
	// Getting a random number of 8 possible values.

	long long int milis = clock();
	srand(milis);
	int direction/* = rand() % 8*/;

	do
	{
		direction = rand() % 10;
	}
	while(direction == 5 || direction == 0);

	// Making it an ascii number character

	direction += 0x30;

	// Moving in a direction according to the random number.

	move(Actor_id, direction, count);

	return;
}

// A function to select set the cursor to a given actor.

void hilight_actor(int actor_id)
{
	printf(CURSOR_UP, TERM_HEIGHT - Mobs[actor_id].ypos -1);
	printf(CURSOR_BACK, TERM_WIDTH - Mobs[actor_id].xpos -1);

	return;
}

// A function to interactively teleport the user. [unfinished]

/*
void teleport()
{
	int cursor_x, cursor_y; // Vars to keep track of the pos. of the cursor.

	// Set cursor to user position, and update the cursor trackers.

	hilight_actor(USER);
	cursor_x = Mobs[USER].xpos;
	cursor_y = Mobs[USER].ypos;

	// main code

	interactive_getchar();

	SET_CURSOR_END;
	printf(CURSOR_DOWN, TERM_HEIGHT - Mobs[USER].ypos -1);

	return;
}
*/

void open_inventory()
{
	// Saving at the main frame portion in the place of the invwntory window. (centered)

	save_main_frame((WIDTH - WINDOW_WIDTH) / 2 -1, (HEIGHT - WINDOW_HEIGHT) / 2 -1, WINDOW_WIDTH, WINDOW_HEIGHT);



	// Restoring the saved frame back.

	restore_main_frame((WIDTH - WINDOW_WIDTH) / 2 -1, (HEIGHT - WINDOW_HEIGHT) / 2 -1, WINDOW_WIDTH, WINDOW_HEIGHT);
}

/* Not finished yet, coming soon :)
void cast_projectile(int caster, int distance,int spell_cost, int damage, char *status_msg)
{
	char direct_question[] = " In direction do you want to cast?";
	char strike_msg[] = " WWHAMMM!!!";
	char direct_failed[] = " Aborting ...";


	char fail_string[] = " You don't have enough mana."; // This function won't care about spell cost. That will
	// be decided by cast(). (These instructions will be removed.)

	if (Mobs[caster].mp - spell_cost < 0) {
		set_status_msg(fail_string);
		return;
	}

	Mobs[caster].mp -= spell_cost; // Up until here the stuff will be removed.


	int x_adjust = 0;
	int y_adjust = 0;

	int successful = get_location_adjustment(&x_adjust, &y_adjust);

	if (successful = -1) {
		set_status_msg(direct_failed);
		return;
	}

	char save_char = ' ';
	int projectile_x = Mobs[caster].xpos;
	int projectile_y = Mobs[caster].ypos;

	for(int neg_proximity = 1; neg_proximity <= distance; neg_proximity++)
	{
		projectile_x_dest = projectile_x + x_adjust;
		projectile_y_dest = projectile_y + y_adjust;

		char dest_char = main_frame[projectile_y_dest][projectile_x_dest];

		if (dest_char == ' ') {
			main_frame[projectile_y][projectile_x] = save_char; // Recover the current spot.
			save_char = main_frame[projectile_y_dest][projectile_x_dest]; // Save the char we are about to move into.
			main_frame[projectile_y][projectile_x] = PROJECTILE_CHAR; // Patch the destination with the projectile char.
			sleep(0.1);
		}
		//else if (dest_char == '@' || (dest_char <= UPPER_CASE_LOW && dest_char >= UPPER_CASE_HIGH)) {
		else if (isupper(dest_char) || dest_char == '@') {
		}
	}
}
*/

/* References
 * [2] https://stackoverflow.com/questions/10192903/time-in-milliseconds-in-c
 */
