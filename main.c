#include <ncurses.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h> // calloc, free

#include "linkedlist.h"

typedef enum { UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3 } direction;

typedef struct {
  LinkedList *apples_list;
  uint8_t max_apples;
  LinkedList *obstacles_list;
  uint8_t max_obstacles;
} World;

typedef struct {
  int chartype;
  int prev_posX, prev_posY; // Previous position of the snake's head
  int posX, posY;
  direction facing_direction;
  uint16_t points;
  uint16_t highscore;
  LinkedList *body_list;
  World *world;
} Snake;

typedef struct {
  int chartype;
  int posX, posY;
} SnakeBodyPart;

typedef struct {
  int chartype;
  uint16_t points_value;
} Apple;

typedef struct {
  int posX, posY;
  int width, height;
} Obstacle;

/** Declarations **/
// Window handling
WINDOW *create_win(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);

// Snake handling
void tick_snake(Snake *snake, uint64_t delta);
void snake_append_body_part(Snake *snake);
Snake *snake_new();
void snake_reset(Snake *snake);

// World management
void tick_world(World *world, uint64_t delta);
void world_reset(World *world);
World *world_new();

// Apple handling
Apple *apple_new();

int main() {
  srand(time(NULL));
  WINDOW *root = initscr(); /* initialize the curses library */

  cbreak();             /* Line buffering disabled pass on everything to me*/
  keypad(stdscr, true); /* For keyboard arrows 	*/
  noecho();             /* Do not echo out input */
  nodelay(root, true);  /* Make getch non-blocking */

  if (has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
  }
  start_color(); /* Start color */
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  // attron(COLOR_PAIR(1));
  bkgdset(COLOR_PAIR(1));
  refresh();

  World *world = world_new();
  Snake *snake = snake_new();

  // Window border
  create_win(LINES, COLS, 0, 0);

  // TODO: Add walls/obstacles to the World.

  int ch;
  bool QUIT = false;

  struct timeval start;
  struct timeval now;
  gettimeofday(&start, NULL);
  uint64_t delta;
  uint64_t const DELTA_INTERVAL = 50; // Tick rate in microseconds.

  while (!QUIT) {
    // Tick the world
    gettimeofday(&now, NULL);
    delta = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) -
            ((start.tv_sec * 1000) + (start.tv_usec / 1000));
    if (delta >= DELTA_INTERVAL) {
      tick_world(world, delta);
      tick_snake(snake, delta);
      start = now;
    }

    // Process user input
    ch = getch();
    switch (ch) {
    case KEY_LEFT:
      if (snake->facing_direction != RIGHT) {
        snake->facing_direction = LEFT;
      }
      break;
    case KEY_RIGHT:
      if (snake->facing_direction != LEFT) {
        snake->facing_direction = RIGHT;
      }
      break;
    case KEY_UP:
      if (snake->facing_direction != DOWN) {
        snake->facing_direction = UP;
      }
      break;
    case KEY_DOWN:
      if (snake->facing_direction != UP) {
        snake->facing_direction = DOWN;
      }
      break;
    case 'a':
      snake_append_body_part(snake);
      snake->points = snake->body_list->length;
      break;
    case 'r':
      QUIT = true;
    }

    mvprintw(2, COLS / 16, "Score: %i", snake->points);
    mvprintw(2, COLS - COLS / 8, "Highscore: %i", snake->highscore);
    refresh();
  }

  delwin(root);
  endwin();
}

/** Snake management **/
void tick_snake(Snake *snake, uint64_t delta) {
  static const uint64_t update_interval =
      100; // Update interval in microseconds
  static uint64_t time_since_update = 0;
  time_since_update += delta;
  if (time_since_update < update_interval) { // Only update after 100 ms
    return;
  }

  mvaddch(snake->posY, snake->posX, ' '); // Clear the old head charachter

  // Save previous position coordinates, useful when rendering the body/tail.
  snake->prev_posY = snake->posY;
  snake->prev_posX = snake->posX;
  // FIXME: Change moving speed depending on vertical/horizontal movement
  // direction
  // Move snake in the facing direction
  switch (snake->facing_direction) {
  case UP:
    snake->posY--;
    snake->chartype = '^';
    break;
  case DOWN:
    snake->posY++;
    snake->chartype = 'v';
    break;
  case LEFT:
    snake->posX--;
    snake->chartype = '<';
    break;
  case RIGHT:
    snake->posX++;
    snake->chartype = '>';
    break;
  }

  // Check collision with the screen bounds
  if (snake->posX >= COLS - 2 || snake->posX <= 0) {
    snake_reset(snake);
    return;
  } else if (snake->posY >= LINES - 2 || snake->posY <= 0) {
    snake_reset(snake);
    return;
  }
  // TODO: Check collision with the tail

  // TODO: Check collision with all the obstacles

  // Check collsion with apples
  if (mvinch(snake->posY, snake->posX) == '@') {
    mvaddch(snake->posY, snake->posX, ' ');
    snake_append_body_part(snake);
  }

  // TODO: Change the chartype of the tail when going vertical/horizontal

  SnakeBodyPart *body_part =
      (SnakeBodyPart *)linked_list_pop_last(snake->body_list);
  mvaddch(body_part->posY, body_part->posX, ' '); // Clear the character
  linked_list_add_front(snake->body_list, body_part);
  body_part->posX = snake->prev_posX;
  body_part->posY = snake->prev_posY;
  mvaddch(body_part->posY, body_part->posX, body_part->chartype);

  // Draw head
  mvaddch(snake->posY, snake->posX, snake->chartype);
}

void snake_append_body_part(Snake *snake) {
  SnakeBodyPart *body_part = calloc(sizeof(SnakeBodyPart), 1);
  body_part->chartype = 'z';
  body_part->posX = snake->prev_posX;
  body_part->posY = snake->prev_posY;
  linked_list_add_front(snake->body_list, body_part);
}

Snake *snake_new() {
  Snake *snake = calloc(sizeof(Snake), 1);
  snake_reset(snake);
  return snake;
}

void snake_reset(Snake *snake) {
  snake->posX = COLS / 2;
  snake->posY = LINES / 2;
  snake->points = 0;
  snake->facing_direction = rand() % 4;
  if (snake->body_list != NULL) {
    // Remove all the snakes body parts from the screen if there are any
    if (snake->body_list->length > 0) {
      uint32_t body_length = snake->body_list->length;
      for (size_t i = 0; i < body_length; i++) {
        SnakeBodyPart *body_part = linked_list_pop_last(snake->body_list);
        mvaddch(body_part->posY, body_part->posX, ' ');
      }
    }
    linked_list_dealloc(snake->body_list);
  }
  snake->body_list = linked_list_new();
  snake_append_body_part(snake);
}

/** World management **/
void tick_world(World *world, uint64_t delta) {
  static uint64_t tick_steps = 0;
  tick_steps += delta;

  // FIXME: Better approach might be a switch and fallthorugh it ...
  if (tick_steps >= 3000) {
    // Spawn Apple at random location every 3rd
    // untill there are world->max_apples apple on screen.
    if (world->apples_list->length < world->max_apples) {
      Apple *apple = apple_new();
      int rand_y = rand() % LINES - 2;
      int rand_x = rand() % COLS - 2;
      mvaddch(rand_y, rand_x, apple->chartype);
      linked_list_add_front(world->apples_list, apple);
    }
    tick_steps = 0; // Reset when used
  }
}

World *world_new() {
  World *world = calloc(sizeof(World), 1);
  world->apples_list = linked_list_new();
  world->max_apples = 5;
  world->obstacles_list = linked_list_new();
  world->max_obstacles = 3;
  return world;
}

/** Apple handling **/
Apple *apple_new() {
  Apple *apple = calloc(sizeof(Apple), 1);
  apple->chartype = '@';
  apple->points_value = 10;
  return apple;
}

/** Window handling **/
// Allocs a new window and sets a box around it plus displays it
WINDOW *create_win(int height, int width, int starty, int startx) {
  WINDOW *local_win;

  local_win = newwin(height, width, starty, startx);
  box(local_win, 0, 0); /* 0, 0 gives default characters
                         * for the vertical and horizontal
                         * lines			*/
  wrefresh(local_win);  /* Show that box 		*/

  return local_win;
}

// Deallocs the window and removes leftover artefacts
void destroy_win(WINDOW *local_win) {
  /* box(local_win, ' ', ' '); : This won't produce the desired
   * result of erasing the window. It will leave it's four corners
   * and so an ugly remnant of window.
   */
  wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(local_win);
  delwin(local_win);
}
