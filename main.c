#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

/*
QUESTIONS/THOUGHTS:
1. What is the best way to make this speed up? We could modify the FPS, though
then it'll be slower to move left and right. Maybe this is okay? Alternatively
we could have the y_speed change. This is probably better. It'll make the
animation smoother for downward movement, which will conflict a bit with the
more jumpy lateral movement, but that's okay.

2. Draw process is getting pretty big/awkward. Might be good to put it into
a distinct draw function that gets called.

3. Starting to get messy. Refactoring should be a goal.

4. Need to break is_game_over stuff into a discrete function.
*/

/* Outline
1. Main. Initialize everything.

2. Draw function. Draws everything. If at the end, a tet has settled, then we
call the realign function to make sure it's exactly right. When returning
to main, if current has settled then we create a new current. */


// Variables that we'll want to remain static or mostly static.
static int FPS = 60;
int y_speed; // how fast is it moving? This value should be updating.
static int DX = 54; // pixels necessary to move from column to column
static int DY = 54;
int height = 640; //We can use argv to modify these, and otherwise set a default
static int starting_x = 218; // Perfectly aligns with column five.
static int starting_y = 43;
int width = 480;

typedef struct Block { // generic typedef for the blocks that make up a tetromino
    int x;
    int y;
    ALLEGRO_BITMAP *square;
} Block;

typedef struct Tetromino {
    struct Block *blocks[4];
    int arrangement; //should range from 0 to 3, to cover all rotations.
    int type; // Likely placeholder. Range 0 to 6, so we know shape when rotating.
    int x1; // low end of x range
    int x2; // high end of x range
    int y1;
    int y2;
    bool settled; // Has the tet settled, or is it still falling?
} Tetromino;


// Declaration of various functions.
void move_left(struct Block *board[24][10], struct Tetromino *current);
void move_right(struct Block *board[24][10], struct Tetromino *current);
void increase_y_speed(int *cur_y); // not sure if/how to implement this.
void drop(int *cur_y);
void rotate_left(struct Tetromino *current);
void rotate_right(struct Tetromino *current);

// Default movement also checks if we need to create a new tet.
bool default_movement(struct Block *board[24][10], struct Tetromino *current);
int clear_lines(struct Block *board[24][10]);

void draw_screen(struct Block *board[24][10], struct Tetromino *current, ALLEGRO_BITMAP *background);
struct Tetromino* create_tetromino(ALLEGRO_BITMAP *shapes[7]);

int main(int argc, char *argv[]) {
    //create miscellaneous variables!
    srand(time(NULL));
    bool running = true;
    bool draw = true;
    struct Tetromino *current;
    /* We'll be using these three eventually. For now, they're not necessary. */
    //int score = 0;
    int total_cleared = 0;
    int level = 0;

    // Various initializations, confirmations that everything is working. //
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *queue = NULL;
    ALLEGRO_TIMER *timer = NULL;

    if (!al_init()) {
        printf("Error: failed to initialize Allegro.\n");
        return 1;
    }
    timer = al_create_timer(1.0 / FPS);
    if (!timer) {
        printf("Error: failed to initialize timer.\n");
        return 1;
    }
    al_install_keyboard();
    al_init_image_addon();

    display = al_create_display(width, height);
    if (!display) {
        printf("Error: failed to initialize display.\n");
        return 1;
    }

    // Loading images of the various shapes. //
    ALLEGRO_BITMAP *red = al_load_bitmap("red.png");
    ALLEGRO_BITMAP *background = al_load_bitmap("background.png");
    ALLEGRO_BITMAP *shapes[7];
    for (int x = 0; x < 7; ++x) { // Temporary. This will be replaced in the
        shapes[x] = red;          // end by seven distinct colors.
    }
    current = create_tetromino(shapes);

    struct Block *board[24][10]; // 22 rows height, 10 columns wide. Top four are
                                 // just a buffer, but cause game over if used.
    for (int y = 0; y < 24; ++y) {
        for (int x = 0; x < 10; ++x) {
            board[y][x] = NULL;
        }
    }

    // Create event queue, register event sources.
    queue = al_create_event_queue();
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_keyboard_event_source());

    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();
    al_start_timer(timer);

    while (running) {
        ALLEGRO_EVENT event;
        ALLEGRO_TIMEOUT timeout;

        al_init_timeout(&timeout, 0.06);

        bool get_event = al_wait_for_event_until(queue, &event, &timeout);

        if (get_event) {
            switch (event.type) {
                case ALLEGRO_EVENT_TIMER:
					draw = true;
					break;
				case ALLEGRO_EVENT_DISPLAY_CLOSE:
					running = false;
					break;
                case ALLEGRO_EVENT_KEY_DOWN:
                    switch (event.keyboard.keycode) {
                        case ALLEGRO_KEY_LEFT:
                            move_left(board, current);
                            break;
                        case ALLEGRO_KEY_RIGHT:
                            move_right(board, current);
                            break;
                        case ALLEGRO_KEY_UP:
                            // drop(cur_shape->y);
                            break;
                        case ALLEGRO_KEY_Q:
                            running = false;
                            break;
                        default:
                            break;
                    }
                default:
                    break;
            }
        }
        if (draw && al_is_event_queue_empty(queue)) {
            draw = false;
            draw_screen(board, current, background);
            if (default_movement(board, current)) {
                current = create_tetromino(shapes);
            }
        }
        int lines_cleared = clear_lines(board);
        total_cleared += lines_cleared;
        level = total_cleared / 2;
        y_speed = 10 + level;
        if (current->y1 == starting_y) { // if we're at the start and  in a block's
            for (int i = 0; i < 4; i++) { // space, we have lost and the game ends.
                if (board[0][current->blocks[i]->x / DX]) {
                    running = false;
                }
            }
        }
    }

    al_destroy_bitmap(red);
    al_destroy_bitmap(background);
    al_destroy_display(display);
    al_destroy_event_queue(queue);
    return 0;
}

bool default_movement(struct Block *board[24][10], struct Tetromino *current) {
    struct Block *cur_block;
    bool create_new_tet = false;
    int delta;
    for (int i = 0; i < 4; ++i) {
        cur_block = current->blocks[i];
        cur_block->y += y_speed;
        if (board[cur_block->y / DY][cur_block->x / DX]) {
            delta = cur_block->y - board[cur_block->y / DY][cur_block->x / DX]->y;
            create_new_tet = true;
        } else if (cur_block->y > 20 * DY - 2) {
            delta = cur_block-> y - (starting_y + 20 * DY);
            create_new_tet = true;
        }
    }
    if (create_new_tet) {
        for (int i = 0; i < 4; ++i) {
            cur_block = current->blocks[i];
            cur_block->y -= delta + DY;
            int x = cur_block->x / DX;
            int y = cur_block->y / DY;
            struct Block *new;
            new = (struct Block*) malloc(sizeof(struct Block));
            new->x = cur_block->x;
            new->y = cur_block->y;
            new->square = cur_block->square;
            if (y >= 0 && y < 22) {
                board[y][x] = new;
            }
            free(current->blocks[i]);
        }
        free(current);
    }
    return create_new_tet;
}

void draw_screen(struct Block *board[24][10], struct Tetromino *current,
                 ALLEGRO_BITMAP *background) {

    struct Block *cur_block;

    for (int i = 0; i < 4; ++i) {
        cur_block = current->blocks[i];
        al_draw_bitmap(cur_block->square, cur_block->x, cur_block->y, 0);
    }

    al_draw_bitmap(background, 10, 50, 0);
    for (int y = 0; y < 20; ++y) {
        for (int x = 0; x < 10; ++x) {
            if (board[y][x]) {
                cur_block = board[y][x];
                al_draw_bitmap(cur_block->square, cur_block->x,
                               cur_block->y, 0);
            }
        }
    }
    al_flip_display();
    al_clear_to_color(al_map_rgb(0, 0, 0));
}

void move_left(struct Block *board[24][10], struct Tetromino *current) {
    if ((current->x1 / DX) <= 0) {
        return;
    }
    for (int i = 0; i < 4; ++i) {
        struct Block *cur_block = current->blocks[i];
        if (board[cur_block->y / DY][cur_block->x / DX - 1]) {
            return;
        }
    }
    for (int i = 0; i < 4; ++i) {
        current->blocks[i]->x -= DX;
    }
    current->x1 -= DX;
    current->x2 -= DX;
}

void move_right(struct Block *board[24][10], struct Tetromino *current) {
    if (current->x2 / DX >= 9) {
        return;
    }
    for (int i = 0; i < 4; ++i) {
        struct Block *cur_block = current->blocks[i];
        if (board[cur_block->y / DY][cur_block->x / DX + 1]) {
            return;
        }
    }
    for (int i = 0; i < 4; ++i) {
        current->blocks[i]->x += DX;
    }
    current->x1 += DX;
    current->x2 += DX;
}

void increase_y_speed(int *cur_y) {
    printf("In increase_y_speed\n");
}
void drop(int *cur_y) {
    printf("In drop\n");
}

int clear_lines(struct Block *board[24][10]) {
    int cleared = 0;
    bool clear_line;
    for (int y = 0; y < 20; y++) {
        clear_line = true;
        for (int x = 0; x < 10; x++) {
            if (!board[y][x]) {
                clear_line = false;
                // break;
            }
        }
        if (clear_line) {
            cleared++;
            for (int i = y; i > 0; i--) {
                printf("I = %d\n", i);
                for (int j = 0; j < 10; j++) {
                    if (board[i][j] != NULL) {
                        free(board[i][j]);
                    }
                    board[i][j] = board[i-1][j];
                }
            }
            for (int i = 0; i < 10; i++) {
                board[0][i] = NULL;
            }
        }
    }
    return cleared;
}


struct Tetromino* create_tetromino(ALLEGRO_BITMAP *shapes[7]) {
    struct Tetromino *new_tet;
    new_tet = (struct Tetromino*) malloc(sizeof(struct Tetromino));
    int type = rand() % 7; // Determine which of the seven types of tet we get.
    type = 1;
    new_tet->type = type;
    // new_tet->square = shapes[type];
    new_tet->settled = false;
    new_tet->x1 = 10000;
    new_tet->x2 = 0;
    new_tet->y1 = 10000;
    new_tet->y2 = 0;
    new_tet->arrangement = 0;

    /* ... create blocks, put them into the new_tet.blocks arr.
           set their x and y coordinates. Track total tet x and y
           ranges, so we know to stop if any of them fall out of
           that range. */

    switch (type) {
        case 0: // stick block
            for (int i = 0; i < 4; ++i) {
                struct Block *new_block;
                new_block = (struct Block*) malloc(sizeof(struct Block));
                new_block->x = starting_x;
                new_block->y = starting_y + (i * DY);
                new_block->square = shapes[type];
                new_tet->blocks[i] = new_block;
            }
            break;
        case 1: // square.
            for (int i = 0; i < 4; ++i) {
                struct Block *new_block;
                new_block = (struct Block*) malloc(sizeof(struct Block));
                new_block->square = shapes[type];
                new_tet->blocks[i] = new_block;
            }
            new_tet->blocks[0]->x = starting_x;
            new_tet->blocks[0]->y = starting_y;
            new_tet->blocks[1]->x = starting_x + DX;
            new_tet->blocks[1]->y = starting_y;
            new_tet->blocks[2]->x = starting_x;
            new_tet->blocks[2]->y = starting_y + DY;
            new_tet->blocks[3]->x = starting_x + DX;
            new_tet->blocks[3]->y = starting_y + DY;

            break;
        case 2: // T block;
            // ...
            break;
        case 3: // El block;
            // ...
            break;
        case 4: // reverse el block;
            // ...
            break;
        case 5: // dog block
            // ...
            break;
        case 6: // reverse dog block
            // ...
            break;
        default:
            printf("Error: illegal tetromino type selection.\n");
            exit (1);
            break;
    }
    for (int i = 0; i < 4; ++i) {
        new_tet->x1 = (new_tet->x1 > new_tet->blocks[i]->x) ?
                      new_tet->blocks[i]->x : new_tet->x1;
        new_tet->x2 = (new_tet->x2 < new_tet->blocks[i]->x) ?
                      new_tet->blocks[i]->x : new_tet->x2;
        new_tet->y1 = (new_tet->y1 > new_tet->blocks[i]->y) ?
                      new_tet->blocks[i]->y : new_tet->y1;
        new_tet->y2 = (new_tet->y2 < new_tet->blocks[i]->y) ?
                      new_tet->blocks[i]->y : new_tet->y2;
    }
    return new_tet;
}
