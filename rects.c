#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <sys/nearptr.h>

/**
 *	Memory locations for DOS instructions
 */
#define VIDEO_INTERRUPT 		0x10
#define SET_MODE				0x00
#define VGA_256_COLOR_MODE		0x13
#define TEXT_MODE				0x03

/**
 *	Video parameters
 */
#define SCREEN_WIDTH			320
#define SCREEN_HEIGHT			200
#define NUM_COLORS				256

/**
 *	Checking to see if an int is signed
 */
#define sign(x)					((x < 0) ? -1:((x > 0) ? 1:0))

/**
 *	Type definitions
 */
typedef unsigned char 	byte;
typedef unsigned short 	word;

/**
 *	Access the video and clock hardware memory locations
 */
byte *VGA 	= (byte *)0xA0000;
word *clock = (word *)0x046c; 

/**
 *	Function prototypes - like in a header file
 */
void set_mode(byte mode);
void plot_pixel(int x, int y, byte color);
void line(int x1, int y1, int x2, int y2, byte color);
void rect_outline(int left, int top, int right, int bottom, byte color);
void rect_fill(int left, int top, int right, int bottom, byte color);

/**
 *	Main function
 */
void main() {

	int x1, y1, x2, y2, color;
	word i;

	if(__djgpp_nearptr_enable() == 0) {
		printf("Could not allocate memory - exiting");
		exit(-1);
	}

	VGA 	+= __djgpp_conventional_base;
	set_mode(VGA_256_COLOR_MODE);

	clock 	= (void *)clock + __djgpp_conventional_base;
	srand(*clock);

	for(i = 0; i < 2500; i++) {
		x1 = rand() % SCREEN_WIDTH;
		y1 = rand() % SCREEN_HEIGHT;
		x2 = rand() % SCREEN_WIDTH;
		y2 = rand() % SCREEN_HEIGHT;
		color = rand() % NUM_COLORS;
		rect_outline(x1, y1, x2, y2, color);
	}

	set_mode(TEXT_MODE);

	return;
}

/**
 *	Setting the video mode by doing the following
 *	- Put the set mode instruction in the higher AX register (AH)
 *	- put the mode in the lower AX register (AL)
 *	- call the dos interrupt with the correct params
 */
void set_mode(byte mode) {
	union REGS regs;
	regs.h.ah = SET_MODE;
	regs.h.al = mode;
	int86(VIDEO_INTERRUPT, &regs, &regs); 
}

/**
 *	Plot pixel and color parameters and write them to the 
 *	video memory offset. Here we are using bit shifting 
 *	which is faster than multiplication. To get the memory
 *	location for the y coordinate, we need to 'multiply' it
 *	by 320, or in this case, use bit shifting to achieve the
 *	same result in a more efficient way.
 */
void plot_pixel(int x, int y, byte color) {
	VGA[(y << 8) + (y << 6) + x] = color;
}

/**
 *	Generate the plotted pixels for a line using Bresenham's 
 *	line plotting algorithm.
 */
void line(int x1, int y1, int x2, int y2, byte color) {

	int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

	dx = x2 - x1; // Grab the difference on the x coordinate
	dy = y2 - y1; // Grab the difference on the y coordinate

	dxabs = abs(dx); //make this an absolute value
	dyabs = abs(dy); //same with this one

	sdx = sign(dx); //Is this value signed?
	sdy = sign(dy); //Or this one?

	x = dxabs >> 1; //Shift this to the right
	y = dyabs >> 1; //This one too

	VGA[(py << 8) + (py << 6) + px] = color; //Write the color to the buffer

	if(dxabs >= dyabs) {
		for(i = 0; i < dxabs; i++) {
			y += dyabs;
			if(y >= dxabs) {
				y -= dxabs;
				py += sdy;
			}
			px += sdx;
			plot_pixel(px, py, color);
		}
	}
	else {
		for(i = 0; i < dyabs; i++) {
			x += dxabs;
			if(x >= dyabs) {
				x -= dyabs;
				px += sdx;
			}
		}
		py += sdy;
		plot_pixel(px, py, color);
	}
}

void rect_outline(int left, int top, int right, int bottom, byte color) {

	word top_offset, bottom_offset, i, temp;

	if(top > bottom) {
		temp = top;
		top = bottom;
		bottom = temp;
	}
	if(left > right) {
		temp = left;
		left = right;
		right = temp;
	}

	top_offset 		= (top << 8) + (top << 6);
	bottom_offset	= (bottom << 8) + (bottom << 6);

	for(i = left; i <= right; i++) {
		VGA[top_offset + i] = color;
		VGA[bottom_offset + i] = color;
	}

	for(i = top_offset; i <= bottom_offset; i += SCREEN_WIDTH) {
		VGA[left + i] = color;
		VGA[right + i] = color;
	}
}

void rect_fill(int left, int top, int right, int bottom, byte color) {
	
	word top_offset, bottom_offset, i, temp, width;

	if(top > bottom) {
		temp = top;
		top = bottom;
		bottom = temp;
	}
	if(left > right) {
		temp = left;
		left = right; 
		right = temp;
	}

	top_offset = (top << 8) + (top << 6) + left;
	bottom_offset = (bottom << 8) + (bottom << 6) + left;
	width = right - left + 1;


	for(i = top_offset; i <= bottom_offset; i += SCREEN_WIDTH) {
		memset(&VGA[i], color, width);
	}
}