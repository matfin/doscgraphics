#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <sys/nearptr.h>

/**
 *	Mapping to video memory and setting up instructions
 */
#define VIDEO_MODE_INT			0x10
#define VGA_256_COLOR_MODE		0x13
#define TEXT_MODE				0x03
#define PIX_PLOT				0x0c
#define SET_MODE				0x00

/**
 *	Screen characteristics
 */
#define SCREEN_WIDTH			320
#define SCREEN_HEIGHT			200
#define NUMBER_OF_COLORS		256

/**
 *	Structs for storing a list of coordinates
 */
typedef struct Coord{
	unsigned int x;
	unsigned int y;
	struct Coord *next;
} Coord;

/**
 *	Struct defining a line
 */
typedef struct {
	Coord *start;
	Coord *end;
	float slope;
} Line;

/**
 *	Other simple type definitions
 */
typedef unsigned char byte;
typedef unsigned short word;

/**
 *	Function prototypes for plotting a line
 */
float slope(Line *line);
void plot(Line *line);
void print(Line *line);
Coord *create();

/**
 *	Function prototypes for drawing a line
 */
void set_mode(byte mode);
void plot_pixel(int x, int y, byte color);
void draw(Line *line);

/**
 *	Hardware function offsets for video memory and the system clock
 */
byte *VGA 	= (byte *)0xA0000;
word *clock = (word *)0x46c; 

/**
 *	Main function, creates a plotted line given start and end 
 *	points and then draws it to the screen once the video
 *	mode has been set.
 */
int main() {

	Line *line 		= malloc(sizeof(Line));
	line->start 	= create();
	line->end 		= create();

	line->start->x = 100;
	line->start->y = 100;
	line->end->x = 120;
	line->end->y = 120;
	line->slope = slope(line);

	plot(line);
	draw(line);

	return 0;
}

/**
 *	Plotting the corrdinates for the line given the starting 
 *	and ending points.
 */
void plot(Line *line) {
	
	int i;
	Coord *marker = create();

	for(i = line->start->x + 1; i < line->end->x; i++) {
		float point_slope = (float)(line->slope * (i - line->start->x)) + line->start->y;
		
		if(line->start->next == NULL) {
			line->start->next = create();
			line->start->next->x = i;
			line->start->next->y = (int)point_slope;
			marker = line->start->next;
		}
		else {
			marker->next = create();
			marker->next->x = i;
			marker->next->y = (int)point_slope;
			marker = marker->next;
		}
	}
}

/**
 *	Loop through the coordinates for a line printing them out,
 *	something useful for debug purposes.
 */
void print(Line *line) {

	Coord *item = line->start;

	while(item != NULL) {
		printf("Plotted: %i %i\n", item->x, item->y);
		item = item->next;
	}
}

/**
 *	Calculate the slope of a line
 */
float slope(Line *line) {

	float numerator = (float)(line->end->y - line->start->y);
	float denomimator = (float)(line->end->x - line->start->x);
	return (float)(numerator / denomimator);
}

/**
 *	Creating a new Coord, allocating space for it in memory
 */
Coord *create() {
	Coord *coord = malloc(sizeof(Coord));
	coord->next = NULL;
	return coord;
}

/**
 *	Instruction to set the video mode
 */
void set_mode(byte mode) {
	union REGS regs;
	regs.h.ah = SET_MODE; 	//Upper part of the AX register
	regs.h.al = mode;		//Lower part of the AX register
	int86(VIDEO_MODE_INT, &regs, &regs); // Call DOS interrupt
}

/**
 *	Plot the pixel directly to video memory from the offset
 *	- 	this is the fast way of doing it. Access the correct 
 *		location in video memory and write the value for the 
 *		color to it.
 */
void plot_pixel(int x, int y, byte color) {
	VGA[y * SCREEN_WIDTH + x] = color;
}

/**
 *	Grab the coordinates for the line and draw
 *	each of them
 */
void draw(Line *line) {
	if(__djgpp_nearptr_enable() == 0) {
		printf("Error accessing 64k of memory. Exiting");
		exit(-1);
	}

	VGA += __djgpp_conventional_base;
	set_mode(VGA_256_COLOR_MODE);

	Coord *item = line->start;
	while(item != NULL) {
		plot_pixel(item->x, item->y, 100);
		item = item->next;
	}

	while(1 == 1) {

	}

	__djgpp_nearptr_disable();
	set_mode(TEXT_MODE);
}