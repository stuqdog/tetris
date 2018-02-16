#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>


/*
QUESTIONS/THOUGHTS:
1. What is the best way to make this speed up? We could modify the FPS, though
then it'll be slower to move left and right. Maybe this is okay? Alternatively
we could have the y_displace change. This is probably better. It'll make the
animation smoother for downward movement, which will conflict a bit with the
more jumpy lateral movement, but that's okay.

2. Draw process is getting pretty big/awkward. Might be good to put it into
a distinct draw function that gets called.

3. Starting to get messy. Refactoring should be a goal.

4. on_the_second is useful for testing, but won't be necessary. So figure out how
to use that.
*/


// Variables that we'll want to remain static or mostly static.
static int FPS = 60;
int y_displace = 5; // how fast is it moving? This value should be updating.
static int x_displace = 27; // Placeholder. Gotta figure out what x_displace
                             // is. Static because we always want to move 1/10
                             // of the play area any time we move left/right.
int height = 640; //We can use argv to modify these, and otherwise set a default
int width = 480;

typedef struct Tetromino { //generic typedef for any tetromino
    int x[4];  // These are the x/y coordinates of all four individual squares.
    int y[4];  // Perhaps these should instead be pointers to individual blocks?
    int arrangement; //should range from 0 to 3, to cover all rotations.
    ALLEGRO_BITMAP *square; // Which square does it get?? Initially, it may be
                            // easiest to just use a single square for all shapes,
                            // but multicolor is pretty and nice so...
    int type; // Likely placeholder. Range 0 to 6, so we know shape when rotating.
} Tetromino;

typedef struct Block { // generic typedef for the blocks that make up a tetromino
    int x;
    int y;
    ALLEGRO_BITMAP *square;
} Block;


// Declaration of movement functions.
void *move_left(int *cur_x);
void *move_right(int *cur_x);
void *increase_y_speed(int *cur_y); // not sure if/how this will get implemented.
void *drop(int *cur_y);
struct Tetromino* create_tetromino(ALLEGRO_BITMAP *shapes[7]);

int main(int argc, char *argv[]) {

    srand(time(NULL));
    bool running = true;
    bool draw = true;
    bool on_the_second = true; //purely for testing, should be removed at the end.

    //various initializations, confirmations that everything is working.
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

    //Loading images of the various shapes.
    ALLEGRO_BITMAP *red = al_load_bitmap("red.png");
    // ALLEGRO_BITMAP *line = al_load_bitmap("Line.png");
    // ALLEGRO_BITMAP *square = al_load_bitmap("Square.png");
    // ALLEGRO_BITMAP *t = al_load_bitmap("T.png");
    ALLEGRO_BITMAP *background = al_load_bitmap("background.png");

    ALLEGRO_BITMAP *shapes[7];
    for (int x = 0; x < 7; ++x) {
        shapes[x] = red;
    }

    /* First things first, let's figure out what variables we're gonna need
    in order to make this work. What do we have to keep track of? */
    struct Block *board[20][10]; // 20 rows height, 10 columns wide.
                       // Note to self: might want to modify slightly the row
                       // number in order to allow for pieces to be moved when
                       // a bunch have already piled up.
    for (int y = 0; y < 20; ++y) {
        for (int x = 0; x < 10; ++x) {
            board[y][x] = NULL;
        }
    }
    struct Block *block_count[190]; // A list of all the existing blocks.
                                    // We can free them as they leave the board.
    for (int x = 0; x < 190; ++x) {
        block_count[x] = NULL;
    }

    int block_height[10] = {0}; //Current height of every row.
    int score = 0;
    int lines_cleared = 0;
    int level = 0;

    struct Tetromino *cur_shape;
    struct Block *cur_block;
    cur_shape = (struct Tetromino*) malloc(sizeof(struct Tetromino));
    for (int i = 0; i < 4; ++i) {
        cur_shape->x[i] = 0;
        cur_shape->y[i] = 0;
    }
    cur_shape->arrangement = 0;

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
        if (rand() % 10 == 0) {
            struct Tetromino *new_tet = create_tetromino(shapes);
        }

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
                            move_left(cur_shape->x);
                            break;
                        case ALLEGRO_KEY_RIGHT:
                            move_right(cur_shape->x);
                            break;
                        case ALLEGRO_KEY_UP:
                            drop(cur_shape->y);
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
            if ((int) al_get_time() % 2 == 0 && on_the_second == true) {
                for (int x = 0; x < 190; ++x) {
                    printf("Time: %d\n", x);
                    if (!block_count[x]) {
                        struct Block *new;
                        new = (struct Block*) malloc(sizeof(struct Block));
                        new->square = red;
                        new->x = 150;
                        new->y = 60;
                        block_count[x] = new;
                        on_the_second = false;
                        break;
                    }
                }
            } else if (((int) al_get_time() + 1) % 2 == 0) {
                on_the_second = true;
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
            for (int i = 0; i < 190; ++i) {
                if (!block_count[i]) {
                    continue;
                }
                block_count[i]->y += y_displace;
                if (block_count[i]->y > 1068) {
                    free(block_count[i]);
                    block_count[i] = NULL;
                } else {
                    al_draw_bitmap(red, block_count[i]->x, block_count[i]->y, 0);
                }
            }
            al_flip_display();
            al_clear_to_color(al_map_rgb(0, 0, 0));
            draw = false;
        }
    }

    al_destroy_bitmap(red);
    al_destroy_bitmap(background);
    al_destroy_display(display);
    al_destroy_event_queue(queue);
    return 0;
}

void *move_left(int *cur_x) {
    printf("In move_left\n");
    return 0;
}
void *move_right(int *cur_x) {
    printf("In move_right\n");
    return 0;
}
void *increase_y_speed(int *cur_y) {
    printf("In increase_y_speed\n");
    return 0;
}
void *drop(int *cur_y) {
    printf("In drop\n");
    return 0;
}

struct Tetromino* create_tetromino(ALLEGRO_BITMAP *shapes[7]) {
    struct Tetromino *new_tet;
    new_tet = (struct Tetromino*) malloc(sizeof(struct Tetromino));
    int random = rand() % 7;
    new_tet->type = 0;
    new_tet->square = shapes[new_tet->type];

    return new_tet;
}
