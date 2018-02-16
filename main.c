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

2. We need to redo the animations. Because we will end up at times deleting only
portions of the shapes, we can't just have a big line. Each shape needs to be
four small squares arranged together.
2a. Along these lines, we probably don't need 7 structs for shapes. Just one will
do. We load which color block at random based on what shape we end up with,
and then have like four different sets of variables for the four squares, all
interconnected. 
*/


// Variables that we'll want to remain static or mostly static.
static int FPS = 60;
int y_displace; // how fast is it moving? This value should be updating.
static int x_displace = 100; // Placeholder. Gotta figure out what x_displace
                             // is. Static because we always want to move 1/10
                             // of the play area any time we move left/right.
int height; //We can use argv to modify these, and otherwise set a default
int width;

typedef struct Line {
    int x; // x-coordinate
    int y; // y-coordinate
    int total_columns[2]; // range of columns it occupies.
    int v_column; // which column is it in when vertical?
    int h_columns[4]; // which columns is it in when horizontal?
    bool is_vertical;
    ALLEGRO_BITMAP *shape;
} Line;

typedef struct Box {
    int x; // x-coordinate
    int y; // y-coordinate
    int columns[2];
    ALLEGRO_BITMAP *shape;
} Box;

typedef struct El {
    int x; // x-coordinate
    int y; // y-coordinate
    int total_columns[2];
    int arrangement; //should range from 0 to 3, to cover different arrangements
    int columns[3]; //Can be 1, 2, or 3, but we can set unused ones to 0.
    int secondary_column; //lip might get hooked even if main line isn't.
    ALLEGRO_BITMAP *shape;
} El;

typedef struct El_R { //reversed El from above;
    int x; // x-coordinate
    int y; // y-coordinate
    int total_columns[2];
    int arrangement; //should range from 0 to 3, to cover different arrangements
    int columns[3]; //Can be 1, 2, or 3, but we can set unused ones to 0.
    int secondary_column;
    ALLEGRO_BITMAP *shape;
} El_R;

typedef struct Dog { //the snake shape;
    int x; // x-coordinate
    int y; // y-coordinate
    int total_columns[2];
    bool horizontal; // two arrangements: horizontal and vertical.
    int columns_covered[2]; //if vertical, we can set index 1 to -1 or something.
    int secondary_column;
    ALLEGRO_BITMAP *shape;
} Dog;

typedef struct Dog_R { //the snake shape, reversed;
    int x; // x-coordinate
    int y; // y-coordinate
    int total_columns[2];
    bool horizontal;
    int columns_covered[2];
    int secondary_column;
    ALLEGRO_BITMAP *shape;
} Dog_R;

typedef struct T {
    int x; // x-coordinate
    int y; // y-coordinate
    int total_columns[3];
    int arrangement;
    int columns_covere[3];
    int secondary_columns[2];
    ALLEGRO_BITMAP *shape;
}

union Shape {
    struct Line line;
    struct T t;
    struct Box box;
    struct El el;
    struct El_R el_r;
    struct Dog dog;
    struct Dog_R dog_r;
};

int move_lateral(int cur_x);

int main(int argc, char *argv[]) {

    bool running = true;

    //various initializations, confirmations that everything is working.
    if (!al_init()) {
        printf("Error: failed to initialize Allegro.\n");
        return 1;
    }
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);
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
    ALLEGRO_BITMAP *line = al_load_bitmap("Line.png");
    ALLEGRO_BITMAP *square = al_load_bitmap("Square.png");
    ALLEGRO_BITMAP *T = al_load_bitmap("T.png");

    // Create event queue, register event sources.
    queue = al_create_event_queue();
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_keyboard_event_source());

    /* First things first, let's figure out what variables we're gonna need
    in order to make this work. What do we have to keep track of? */
    int board[20][10]; // 20 rows height, 10 columns wide.
                       // Note to self: might want to modify slightly the row
                       // number in order to allow for pieces to be moved when
                       // a bunch have already piled up.

    int block_height[10] = {0}; //Current height of every row.
    int score = 0;
    int lines_cleared = 0;
    int level = 0;

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
                            cur_shape.

                    }
            }
        }
    }
}
