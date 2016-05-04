#include <ncurses.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h> // calloc, free

#include "linkedlist.h"

typedef enum { UP, DOWN, LEFT, RIGHT } direction;

typedef struct {
  int prev_posX, prev_posY; // Previous position of the snake's head
  int posX, posY;
  direction facing_direction;
  int points;
  LinkedList *body_list;
} Snake;

typedef struct {
  int chartype;
  int posX, posY;
} SnakeBodyPart;

typedef struct {
  int chartype;
  int posX, posY;
  uint16_t points_value;
} Apple;

/** Declarations **/
// Window handling
WINDOW *create_win(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);

// Snake handling
void tick_snake(uint64_t delta, Snake *snake);
void snake_append_body_part(Snake *snake);

// World management
void tick_world(uint64_t delta);

int main() {
  WINDOW *root = initscr(); /* initialize the curses library */

  cbreak();             /* Line buffering disabled pass on everything to me*/
  keypad(stdscr, true); /* For keyboard arrows 	*/
  noecho();             /* Do not echo out input */
  nodelay(root, true);  /* Make getch non-blocking */
  refresh();

  Snake snake;
  snake.posX = COLS / 2;
  snake.posY = LINES / 2;
  snake.points = 0;
  snake.body_list = linked_list_new();
  snake.facing_direction = RIGHT;

  // Add body parts to the snake
  snake_append_body_part(&snake);
  snake_append_body_part(&snake);
  snake_append_body_part(&snake);

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
      tick_world(delta);
      tick_snake(delta, &snake);
      start = now;
    }

    // Process user input
    ch = getch();
    switch (ch) {
    case KEY_LEFT:
      if (snake.facing_direction != RIGHT) {
        snake.facing_direction = LEFT;
      }
      break;
    case KEY_RIGHT:
      if (snake.facing_direction != LEFT) {
        snake.facing_direction = RIGHT;
      }
      break;
    case KEY_UP:
      if (snake.facing_direction != DOWN) {
        snake.facing_direction = UP;
      }
      break;
    case KEY_DOWN:
      if (snake.facing_direction != UP) {
        snake.facing_direction = DOWN;
      }
      break;
    case 'r':
      QUIT = true;
    }

    // Collision detection
    /*
    move snake_head forward in the snake->facing_direction
    if snake_head has hit apple {
      add points
      add snake_body_part to snake_body_list in snake_head:s old position
      draw entire snake_body_list without moving
    } else if snake_head has hit wall { DIE. }
    else { // Render snake tail as normal
      render the snake tail from front to back by moving (remove from old
      position, adding at new) the last body_part in snake->body_list to the
      snake_head:s old position
    }
     */

    SnakeBodyPart *body_part =
        (SnakeBodyPart *)linked_list_pop_last(snake.body_list);
    mvdelch(body_part->posY, body_part->posX);
    linked_list_add_front(snake.body_list, body_part);
    body_part->posX = snake.prev_posX;
    body_part->posY = snake.prev_posY;
    mvaddch(body_part->posY, body_part->posX, body_part->chartype);

    // Draw stuff
    mvaddch(snake.posY, snake.posX, '>');

    mvprintw(2, 5, "Score: %i", snake.points);
    refresh();
  }

  delwin(root);
  endwin();
}

/** Snake management **/
void tick_snake(uint64_t delta, Snake *snake) {
  static const uint64_t update_interval =
      100; // Update interval in microseconds
  static uint64_t time_since_update = 0;
  time_since_update += delta;
  if (time_since_update < update_interval) { // Only update after 500 ms
    return;
  }

  mvdelch(snake->posY, snake->posX); // Clear the old head charachter

  // Save previous position coordinates, useful when rendering the body/tail.
  snake->prev_posY = snake->posY;
  snake->prev_posX = snake->posX;
  // Move snake in the facing direction
  switch (snake->facing_direction) {
  case UP:
    snake->posY--;
    break;
  case DOWN:
    snake->posY++;
    break;
  case LEFT:
    snake->posX--;
    break;
  case RIGHT:
    snake->posX++;
    break;
  }
}

void snake_append_body_part(Snake *snake) {
  SnakeBodyPart *body_part = calloc(sizeof(SnakeBodyPart), 1);
  body_part->chartype = '=';
  body_part->posX = snake->posX;
  body_part->posY = snake->posY;

  switch (snake->facing_direction) {
  case UP:
    body_part->posY++;
    break;
  case DOWN:
    body_part->posY--;
    break;
  case LEFT:
    body_part->posX++;
    break;
  case RIGHT:
    body_part->posX--;
    break;
  }

  linked_list_add_front(snake->body_list, body_part);
}

/** World management **/
void tick_world(uint64_t delta) {
  static uint64_t tick_steps = 0;
  tick_steps += delta;
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
