#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <sys/nearptr.h>

#define VIDEO_INT			0x10
#define WRITE_DOT			0x0c
#define SET_MODE			0x00
#define VGA_256_COLOR_MODE	0x13
#define TEXT_MODE			0x03

#define SCREEN_WIDTH		320
#define SCREEN_HEIGHT		200
#define NUM_COLORS			256

typedef unsigned char byte;
typedef unsigned short word;

byte *VGA = (byte *)0xA0000;
word *my_clock = (word *)0x46c;

void set_mode(byte mode) {
	union REGS regs;
	regs.h.ah = SET_MODE;
	regs.h.al = mode;
	int86(VIDEO_INT, &regs, &regs);
}

void plot_pixel_slow(int x, int y, byte color) {
	union REGS regs;
	regs.h.ah = WRITE_DOT;
	regs.h.al = color;
	regs.x.cx = x;
	regs.x.dx = y;
	int86(VIDEO_INT, &regs, &regs);
}

void plot_pixel_fast(int x, int y, byte color) {
	VGA[y * SCREEN_WIDTH + x] = color;
}

int main() {

	int x, y, color;
	float t1, t2;
	word i, start;

	if(__djgpp_nearptr_enable() == 0) {
		printf("Can get access to the first 640k of memory.\n");
		exit(-1);
	}

	VGA+= __djgpp_conventional_base;
	my_clock = (void *)my_clock + __djgpp_conventional_base;

	srand(*my_clock);
	set_mode(VGA_256_COLOR_MODE);
	start = *my_clock;

	for(i = 0; i < 5000L; i++) {
		x = rand() % SCREEN_WIDTH;
		y = rand() % SCREEN_HEIGHT;
		color = rand() % NUM_COLORS;
		plot_pixel_slow(x, y, color);
	}

	t1 = (*my_clock - start) / 18.2;

	set_mode(VGA_256_COLOR_MODE);

	start = *my_clock;
	for(i = 0; i < 5000L; i++) {
		x = rand() % SCREEN_WIDTH;
		y = rand() % SCREEN_HEIGHT;
		color = rand() %NUM_COLORS;
		plot_pixel_fast(x, y, color);
	}

	t2 = (*my_clock - start) / 18.2;
	set_mode(TEXT_MODE);

	printf("Slow pixel plotting took %f seconds.\n", t1);
	printf("Fast pixel plotting took %f seconds.\n", t2);

	if(t2 != 0) {
		printf("Fast pixel was %f times faster.\n", t1 / t2);

		__djgpp_nearptr_disable();
	}

	return 0;
}