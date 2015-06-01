#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <sys/nearptr.h>

typedef struct Coord{
	unsigned int x;
	unsigned int y;
	struct Coord *next;
} Coord;

typedef struct {
	Coord *start;
	Coord *end;
	float slope;
} Line;

float slope(Line *line);
void plot(Line *line);
void print(Line *line);
Coord *create();

int main() {

	Line *line 		= malloc(sizeof(Line));
	line->start 	= create();
	line->end 		= create();

	line->start->x = 10;
	line->start->y = 10;
	line->end->x = 20;
	line->end->y = 120;
	line->slope = slope(line);

	plot(line);

	print(line);

	return 0;
}

void plot(Line *line) {
	
	int i;
	Coord *marker = create();

	for(i = line->start->x + 1; i < line->end->x; i++) {
		float point_slope = (float)(line->slope * (i - line->start->x)) + line->start->y;
		
		if(line->start->next == NULL) {
			printf("Line next coord is NULL\n");
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
			printf("Line coord is not NULL\n");
		}

		// printf("slope: %0.2f x:%i y:%0.2f yrounded:%i\n", line->slope, i, point_slope, (int)point_slope);
	}
}

void print(Line *line) {

	Coord *item = line->start;

	while(item != NULL) {
		printf("Plotted: %i %i\n", item->x, item->y);
		item = item->next;
	}
}

float slope(Line *line) {

	float numerator = (float)(line->end->y - line->start->y);
	float denomimator = (float)(line->end->x - line->start->x);
	return (float)(numerator / denomimator);
}

Coord *create() {
	Coord *coord = malloc(sizeof(Coord));
	coord->next = NULL;
	return coord;
}