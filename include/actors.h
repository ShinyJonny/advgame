#define NAME_LEN 15
#define NAME_LEN_STR "15"

// Struct variable type holding all the needed values for an in-game character.

typedef struct Actors { // [1] [reference]
	char symbol; // Ascii symbol
	char actor_name[NAME_LEN+1]; // String holding its name.
	int power; // Damage of the character.
	int hp; // HP
	int mp; // MP - not used currently :)
	int xpos; // Its X coordinate
	int ypos; // Its Y coordinate
	int actor_type; // Is it interactive or a bot?
} Actor;

// Are they bots or not?

#define INTERACTIVE 1
#define NONINTERACTIVE 0


/* References
 * [1] https://stackoverflow.com/questions/10468128/how-do-you-make-an-array-of-structs-in-c
 */
