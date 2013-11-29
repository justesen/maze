/* maze - a maze generator and solver
 *
 * This software is licensed under the MIT License, see LICENSE for more info
 * Find the most recent version at http://justesen.zxq.net/src/maze
 *
 * Usage: maze [options]
 */

#include <cairo.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define PRG_NAME "maze" /* program name */
#define VERSION  "0.2"  /* version number */

#define UVISIT 0 /* cell is unvisited */
#define VISIT  1 /* cell is visited */
#define WALL   2 /* cell is a wall */
#define PATH   3 /* cell is a part of the path through the maze */
#define UPATH  4 /* cell is not a part of the path through the maze */

#define UP    0
#define DOWN  1
#define LEFT  2
#define RIGHT 3

#define FILENAME_SIZE 64


/* Options */
struct conf {
	char image[FILENAME_SIZE]; /* filename of image */
	int height;                /* height of maze (in cells) */
	int width;                 /* width of maze (in cells) */
	int pixpercell;            /* height and width of cells */
	int solve;                 /* solve maze (bool) */
};


/* Stack data structure for maze backtracking */
struct stack {
	int x, y;           /* maze coordinates */
	struct stack *next; /* mother cell */
};


/* Add new item to the stack
 *
 * Parameter(s): top - item to be pushed onto the stack
 *               x   - item data (x-coordinate)
 *               y   - item data (y-coordinate)
 *
 * Return: 0 on allocation error
 *         1 otherwise
 */
int push(struct stack **top, const int x, const int y)
{
	struct stack *node = malloc(sizeof *node);

	if (node == NULL) {
		return 0;
	}
	node->x = x;
	node->y = y;
	node->next = *top;
	*top = node;

	return 1;
}


/* Remove top item from the stack
 *
 * Parameter(s): top - top item of the stack
 */
void pop(struct stack **top)
{
	struct stack *tmp;

	if (*top == NULL) {
		return;
	}
	tmp = *top;
	*top = tmp->next;

	free(tmp);
}


/* Free the stack from memory
 *
 * Parameter(s): top - top item of the stack
 */
void free_stack(struct stack **top)
{
	while(*top) {
		pop(top);
	}
}


/* Print the maze (solved or unsolved) to a PNG image
 *
 * Draw a black square for each wall cell
 * Draw a red square if the cell is a part of the path out of the maze
 * Otherwise white
 *
 * Parameter(s): maze - maze cells
 *               opts - options
 */
void maze_to_png(int **maze, struct conf *opts)
{
	cairo_surface_t *surf;
	cairo_t *cr;
	int x, y;

	surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
	                                  opts->width * opts->pixpercell,
	                                  opts->height * opts->pixpercell);
	cr = cairo_create(surf);

	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_rectangle(cr,
	                0,
	                0,
	                opts->width * opts->pixpercell,
	                opts->height * opts->pixpercell);
	cairo_fill(cr);

	for (y = 0; y < opts->height; ++y) {
		for (x = 0; x < opts->width; ++x) {
			if (maze[y][x] == WALL) {
				cairo_rectangle(cr,
				                x * opts->pixpercell,
				                y * opts->pixpercell,
				                opts->pixpercell,
				                opts->pixpercell);
				cairo_set_source_rgb(cr, 0, 0, 0);
				cairo_fill(cr);
			} else if (maze[y][x] == PATH) {
				cairo_rectangle(cr,
				                x * opts->pixpercell,
				                y * opts->pixpercell,
				                opts->pixpercell,
				                opts->pixpercell);
				cairo_set_source_rgb(cr, 1, 0, 0);
				cairo_fill(cr);
			}
		}
	}
	cairo_destroy(cr);
	cairo_surface_write_to_png(surf, opts->image);
	cairo_surface_destroy(surf);
}


/* Free the maze from memory
 *
 * Parameter(s): maze - maze cells
 *               h    - maze height
 */
void free_maze(int **maze, const int h)
{
	int y;

	for (y = 0; y < h; ++y) {
		free(maze[y]);
	}
	free(maze);
}


/* Get coordinates to random visited neighbour cell
 *
 * Parameter(s): maze - maze cells
 *               x    - cell's x-coordinate
 *               y    - cell's y-coordinate
 *               w    - maze width
 *               h    - maze height
 *
 * Return: 0 if no available cell was found
 *         1 otherwise
 */
int solve_rand_neighbour(int **maze, int *const x, int *const y, const int w, const int h)
{
	int up, down, left, right;
	int rand_direction;

	up = down = left = right = 1;

	while (up || down || left || right) {
		rand_direction = rand() % 4;

		switch (rand_direction) {
			case UP:
				if (*y - 1 <= 0
				 || maze[*y - 1][*x] != VISIT) {
					up = 0;
				} else {
					*y -= 1;

					return 1;
				}
				break;
			case DOWN:
				if (*y + 1 >= h - 1
				 || maze[*y + 1][*x] != VISIT) {
					down = 0;
				} else {
					*y += 1;

					return 1;
				}
				break;
			case LEFT:
				if (*x - 1 <= 0
				 || maze[*y][*x - 1] != VISIT) {
					left = 0;
				} else {
					*x -= 1;

					return 1;
				}
				break;
			case RIGHT:
				if (*x + 1 >= w - 1
				 || maze[*y][*x + 1] != VISIT) {
					right = 0;
				} else {
					*x += 1;

					return 1;
				}
				break;
		}
	}
	return 0;
}


/* Get coordinates to random unvisited neighbour cell and knock down wall
 * in between
 *
 * Parameter(s): maze - maze cells
 *               x    - cell's x-coordinate
 *               y    - cell's y-coordinate
 *               w    - maze width
 *               h    - maze height
 *
 * Return: 0 if no available cell was found
 *         1 otherwise
 */
int rand_neighbour(int **maze, int *const x, int *const y, const int w, const int h)
{
	int up, down, left, right;
	int rand_direction;

	up = down = left = right = 1;

	while (up || down || left || right) {
		rand_direction = rand() % 4;

		switch (rand_direction) {
			case UP:
				if (*y - 2 <= 0
				 || maze[*y - 2][*x] != UVISIT) {
					up = 0;
				} else {
					maze[*y - 1][*x] = VISIT;
					*y -= 2;

					return 1;
				}
				break;
			case DOWN:
				if (*y + 2 >= h
				 || maze[*y + 2][*x] != UVISIT) {
					down = 0;
				} else {
					maze[*y + 1][*x] = VISIT;
					*y += 2;

					return 1;
				}
				break;
			case LEFT:
				if (*x - 2 <= 0
				 || maze[*y][*x - 2] != UVISIT) {
					left = 0;
				} else {
					maze[*y][*x - 1] = VISIT;
					*x -= 2;

					return 1;
				}
				break;
			case RIGHT:
				if (*x + 2 >= w
				 || maze[*y][*x + 2] != UVISIT) {
					right = 0;
				} else {
					maze[*y][*x + 1] = VISIT;
					*x += 2;

					return 1;
				}
				break;
		}
	}
	return 0;
}


/* Solve the maze
 *
 * Parameter(s): maze - maze cells
 *               w    - maze width
 *               h    - maze height
 *
 * Return: 0 on allocation error
 *         1 otherwise
 */
int solve(int **maze, const int w, const int h)
{
	struct stack *st = NULL;
	int x, y;
	int tmpx, tmpy;

	x = 1;
	y = 1;
	maze[1][0] = WALL;
	maze[h - 2][w - 1] = WALL;

	while (1) {
		/* Mark cell as part of path through the maze and save its
		 * coordinates for later adding it to the stack (for
		 * backtracking) */
		maze[y][x] = PATH;
		tmpx = x;
		tmpy = y;

		if (solve_rand_neighbour(maze, &x, &y, w, h)) {
			/* If lower right cell; we solved the maze */
			if (x == w - 2 && y == h - 2) {
				maze[y][x] = PATH;

				break;
			}
			if (!push(&st, tmpx, tmpy)) {
				free_stack(&st);

				return 0;
			}
		} else {
			/* If nowhere to go, we backtrack to the mother cell */
			maze[y][x] = UPATH;
			x = st->x;
			y = st->y;

			pop(&st);
		}
	}
	free_stack(&st);

	/* Mark exit and entrance as part of the path */
	maze[1][0] = PATH;
	maze[h - 2][w - 1] = PATH;

	return 1;
}


/* Generate the maze
 *
 * Parameter(s): maze - maze cells
 *               w    - maze width
 *               h    - maze height
 *
 * Return: 0 on allocation error
 *         1 otherwise
 */
int generate(int **maze, const int w, const int h)
{
	struct stack *st = NULL;
	int x, y, tmpx, tmpy;

	/* Seed rand() for rand_neighbour() */
	srand((unsigned int) time(NULL));

	/* Maze generation starts at bottom right corner. It's easiest to solve
	 * the maze in the same 'way' it was generated, and since we believe
	 * people intuitively will start at the top left corner, we start it
	 * opposite of that (bottom right) */
	x = w - 2;
	y = h - 2;

	while (1) {
		maze[y][x] = VISIT;
		tmpx = x;
		tmpy = y;

		if (rand_neighbour(maze, &x, &y, w, h)) {
			if (!push(&st, tmpx, tmpy)) {
				free_stack(&st);

				return 0;
			}
		} else {
			/* If no neighbour, we backtrack to the mother cell */
			x = st->x;
			y = st->y;

			pop(&st);

			/* End generation if we can't backtrack further */
			if (st->next == NULL) {
				break;
			}
		}
	}
	free_stack(&st);

	/* Mark exit and entrance */
	maze[1][0] = VISIT;
	maze[h - 2][w - 1] = VISIT;

	return 1;
}


/* Initialize maze. The initial walls form a grid. Non walls are marked as
 * unvisited
 *
 * Parameter(s): w - maze width
 *               h - maze height
 *
 * Return: Pointer to the maze
 *         NULL on allocation error
 */
int **init(const int w, const int h)
{
	int **maze;
	int x, y;

	if ((maze = malloc(sizeof(*maze) * h)) == NULL) {
		return NULL;
	}
	for (y = 0; y < h; ++y) {
		if ((maze[y] = malloc(sizeof(**maze) * w)) == NULL) {
			return NULL;
		}
		for (x = 0; x < w; ++x) {
			if (y % 2 == 0
			 || x % 2 == 0) {
				maze[y][x] = WALL;
			} else {
				maze[y][x] = UVISIT;
			}
		}
	}
	return maze;
}


/* Display program name and version number - exit succesfully
 */
void version(void)
{
	printf("\
%s %s\n\
\n\
For license and copyright information see the LICENSE file, which should\n\
have been distributed with the software.\n",
	       PRG_NAME, VERSION);

	exit(EXIT_SUCCESS);
}


/* Display help information and exit succesfully
 */
void help(void)
{
	printf("\
%s - a maze generator and solver\n\
\n\
Usage: %s [options]\n\
\n\
Generates a maze in black and white and saves it to maze.png. It also solves\n\
the maze (marked in red) and saves it to solve.png.\n\
\n\
Options:\n\
  --height <cells>       Specify height of maze\n\
  --width  <cells>       Specify width of maze\n\
  --nosolve              Do not solve the generated maze\n\
  --pixpercell <pixels>  Cell width and height\n\
  -h --help              Display this information\n\
  -v --version           Display program name and version number\n",
	       PRG_NAME, PRG_NAME);

	exit(EXIT_SUCCESS);
}


/* Print error message and usage information before exiting indicating an error
 *
 * Parameter(s): fwarns - formatted warning string
 *               ...    - arguments to ferrs
 */
void warning(const char *fwarns, ...)
{
	va_list ap;

	va_start(ap, fwarns);

	fprintf(stderr, "%s: warning: ", PRG_NAME);
	vfprintf(stderr, fwarns, ap);
	fputc('\n', stderr);

	va_end(ap);
}


/* Print error message and usage information before exiting indicating an error
 *
 * Parameter(s): ferrs - formatted error string
 *               ...   - arguments to ferrs
 */
void exit_err(const char *ferrs, ...)
{
	va_list ap;

	va_start(ap, ferrs);

	fprintf(stderr, "%s: error: ", PRG_NAME);
	vfprintf(stderr, ferrs, ap);
	fputc('\n', stderr);

	va_end(ap);

	exit(EXIT_FAILURE);
}


/* Parse commandline arguments
 *
 * Parameter(s): argc - number of arguments
 *               argv - array of string arguments
 *               opts - options
 */
void parse_args(const int argc, char *argv[], struct conf *opts)
{
	int i, tmp;

	for (i = 1; i < argc; ++i) {
		if (strncmp(argv[i], "-h", 3) == 0
		 || strncmp(argv[i], "--help", 7) == 0) {
			help();		
		} else if (strncmp(argv[i], "-v", 7) == 0
		        || strncmp(argv[i], "--version", 10) == 0) {
			version();
		} else if (strncmp(argv[i], "--height", 9) == 0) {
			if (i + 1 < argc) {
				tmp = atoi(argv[++i]);

				if (tmp != 0) {
					tmp = (tmp % 2) == 0 ? tmp - 1 : tmp;
					opts->height = tmp;
				} else {
					warning("%s is not a number, using default %d",
					        argv[i], opts->height);
				}
			} else {
				exit_err("missing <cells> after %s", argv[i]);
			}
		} else if (strncmp(argv[i], "--width", 8) == 0) {
			if (i + 1 < argc) {
				tmp = atoi(argv[++i]);

				if (tmp != 0) {
					tmp = (tmp % 2) == 0 ? tmp - 1 : tmp;
					opts->width = tmp;
				} else {
					warning("%s is not a number, using default %d",
					        argv[i], opts->width);
				}
			} else {
				exit_err("missing <cells> after %s", argv[i]);
			}
		} else if (strncmp(argv[i], "--nosolve", 10) == 0) {
			opts->solve = 0;
		} else if (strncmp(argv[i], "--pixpercell", 13) == 0) {
			if (i + 1 < argc) {
				tmp = atoi(argv[++i]);

				if (tmp != 0) {
					opts->pixpercell = tmp;
				} else {
					warning("%s is not a number, using default %d",
					        argv[i], opts->pixpercell);
				}
			} else {
				exit_err("missing <pixels> after %s", argv[i]);
			}
		} else {
			warning("unkown argument %s is ignored", argv[i]);
		}
	}
}
	

/* Generate a maze and save it to maze.png
 * Solve the same maze and save it to solve.png
 *
 * Parameter(s): argc - number of arguments
 *               argv - array of string arguments
 *
 * Return: 0 on succes
 *         1 otherwise
 */
int main(int argc, char *argv[])
{
	struct conf opts;
	int **maze;

	opts.width = opts.height = 59;
	opts.pixpercell = 10;
	opts.solve = 1;

	strcpy(opts.image, "maze.png");

	parse_args(argc, argv, &opts);

	if ((maze = init(opts.width, opts.height)) == NULL) {
		exit_err("memory allocation failure while initializing maze");
	}
	if (!generate(maze, opts.width, opts.height)){
		free_maze(maze, opts.height);
		exit_err("memory allocation failure while generating maze");
	}
	maze_to_png(maze, &opts);

	if (opts.solve) {
		if (!solve(maze, opts.width, opts.height)) {
			free_maze(maze, opts.height);
			exit_err("memory allocation failure while solving maze");
		}
		strcpy(opts.image, "solve.png");
		maze_to_png(maze, &opts);
	}
	free_maze(maze, opts.height);

	return 0;
}

