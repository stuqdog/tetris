#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

/*
                  ### QUESTIONS/THOUGHTS/PROBLEMS/TO DO ###:
1. Starting to get messy. Refactoring should be a goal. Might want to really
minimize the main function. Have it just set up variables and initializations
and then be like, "If running: call all the functions. Else: quit."
*/


// Variables that we'll want to remain static or mostly static.
static int FPS = 30;
int y_speed; // how fast is it moving? This value should be updating.
static int DX = 54; // pixels necessary to move from column to column
static int DY = 54;
int height = 640; //We can use argv to modify these, and otherwise set a default
static int starting_x = 218; // Perfectly aligns with column five.
static int starting_y = 43;
int width = 480;
int score = 0;
int total_cleared = 0;
int level = 0;

typedef struct Block { // generic typedef for the blocks that make up a tetromino
    int x;
    int y;
    ALLEGRO_BITMAP *square;
} Block;

typedef struct Tetromino {
    struct Block *blocks[4];
    int arrangement; // Range from 0 to 3, to cover all rotations.
    int type; // Range 0 to 6, so we know shape when rotating.
    int x1; // low end of x range
    int x2; // high end of x range
    int y; // lowest point of the tet
} Tetromino;


// Declaration of various functions.
void move_left(struct Block *board[21][10], struct Tetromino *current);
void move_right(struct Block *board[21][10], struct Tetromino *current);
void increase_y_speed(int *cur_y); // not sure if/how to implement this.
void drop(struct Tetromino *current, struct Block *board[21][10]);
void rotate(struct Tetromino *current, int direction, struct Block *board[21][10]);
bool rebalance(struct Tetromino *current, struct Block *board[21][10]);
bool is_game_over(struct Tetromino *current, struct Block *board[21][10]);
bool rotation_is_legal(struct Tetromino *new, struct Block *board[21][10]);

// Default movement also checks if we need to create a new tet.
bool default_movement(struct Block *board[21][10], struct Tetromino *current);
int clear_lines(struct Block *board[21][10]);

void draw_screen(struct Block *board[21][10], struct Tetromino *current, ALLEGRO_BITMAP *background);
struct Tetromino* create_tetromino(ALLEGRO_BITMAP *shapes[7]);

int main(int argc, char *argv[]) {
    //create miscellaneous variables!
    srand(time(NULL));
    bool running = true;
    bool draw = true;
    struct Tetromino *current;

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

    display = al_create_display(width, height);
    if (!display) {
        printf("Error: failed to initialize display.\n");
        return 1;
    }

    al_install_keyboard();
    al_init_image_addon();

    // Loading images of the various shapes. //
    ALLEGRO_BITMAP *red = al_load_bitmap("red.png");
    ALLEGRO_BITMAP *background = al_load_bitmap("background.png");
    ALLEGRO_BITMAP *shapes[7];
    for (int x = 0; x < 7; ++x) { // Temporary. This will be replaced in the
        shapes[x] = red;          // end by seven distinct colors.
    }

    // Create event queue, register event sources, start timer.
    queue = al_create_event_queue();
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_start_timer(timer);

    // Create an initial tetromino and a board.
    current = create_tetromino(shapes);

    struct Block *board[21][10]; // 21 rows height, 10 columns wide. Top row is
                                 // just a buffer, but causes game over if used.
    for (int y = 0; y < 21; ++y) {
        for (int x = 0; x < 10; ++x) {
            board[y][x] = NULL;
        }
    }

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
                            drop(current, board);
                            /* normally create tet is called from default
                            movement, but since we're circumventing default
                            movement with drop and definitely need a new tet
                            after dropping, we call create_tetromino manually. */
                            current = create_tetromino(shapes);
                            break;
                        case ALLEGRO_KEY_Q:
                            running = false;
                            break;
                        case ALLEGRO_KEY_A:
                            rotate(current, -1, board);
                            break;
                        case ALLEGRO_KEY_S:
                            rotate(current, 1, board);
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

        //update game variables - score, level, speed, etc.
        int lines_cleared = clear_lines(board);
        total_cleared += lines_cleared;
        score += pow(2, lines_cleared) * 100;
        level = 1 + total_cleared / 10;
        y_speed = 5 + level;

        if (is_game_over(current, board)) {
            running = false;
        }
    }


    al_destroy_bitmap(red);
    al_destroy_bitmap(background);
    al_destroy_display(display);
    al_destroy_event_queue(queue);
    return 0;
}

bool is_game_over(struct Tetromino *current, struct Block *board[21][10]) {
    if (current->blocks[0]->y == starting_y) { // if we're at the start and in a block's
        for (int i = 0; i < 4; i++) { // space, we have lost and the game ends.
            if (board[0][current->blocks[i]->x / DX]) {
                return true;
            }
        }
    }
    /* The below solution is cleaner, but currently doesn't work. Problem is
    board[20] never gets accessed. We need to adjust such that the top line of
    the grid is actually board[1], not board[0], and board [0] functionally
    becomes a "negative" row. Then change the if loop below to if (board[0][i])
    and voila. */
    // for (int i = 0; i < 10; ++i) {
    //     if(board[20][i]) {
    //         return false;
    //     }
    // }
    return false;
}

bool default_movement(struct Block *board[21][10], struct Tetromino *current) {
    current->y += y_speed;
    bool should_create_new_tet = rebalance(current, board);
    if (should_create_new_tet) {
        free(current);
    }
    return should_create_new_tet;
}

/* Checks if a block has landed on another block, in which case we rebalance the
current tet to neatly fit the grid */
bool rebalance(struct Tetromino *current, struct Block *board[21][10]) {
    bool should_rebalance = false;
    struct Block *cur_block;
    int delta;
    for (int i = 0; i < 4; ++i) {
        cur_block = current->blocks[i];
        cur_block->y += y_speed;
        if (current->y > 0) {
            if (board[cur_block->y / DY][cur_block->x / DX]) {
                delta = cur_block->y - board[cur_block->y / DY][cur_block->x / DX]->y;
                should_rebalance = true;
            } else if (cur_block->y > 20 * DY) {
                delta = cur_block-> y - (starting_y + 20 * DY);
                should_rebalance = true;
            }
        }
    }
    if (should_rebalance) {
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
            if (y >= 0 && y < 20) {
                board[y][x] = new;
            }
            free(current->blocks[i]);
        }
    }
    return should_rebalance;
}


void draw_screen(struct Block *board[21][10], struct Tetromino *current,
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
    al_clear_to_color(al_map_rgb(180, 180, 180));
}

void move_left(struct Block *board[21][10], struct Tetromino *current) {
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

void move_right(struct Block *board[21][10], struct Tetromino *current) {
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
void drop(struct Tetromino *current, struct Block *board[20][10]) {
    bool should_drop = true;
    struct Block *cur_block;
    while (should_drop) {
        for (int i = 0; i < 4; i++) {
            cur_block = current->blocks[i];
            cur_block->y += y_speed;
            if (board[cur_block->y / DY][cur_block->x / DX]) {
                should_drop = false;
            } else if (cur_block->y > 20 * DY) {
                should_drop = false;
            }
        }
        current->y += DY;
    }
    rebalance(current, board);
}

int clear_lines(struct Block *board[21][10]) {
    int cleared = 0;
    bool clear_line;
    for (int y = 1; y < 20; y++) {
        clear_line = true;
        for (int x = 0; x < 10; x++) {
            if (!board[y][x]) {
                clear_line = false;
                break;
            }
        }
        if (clear_line) {
            cleared++;
            for (int i = y; i > 0; i--) {
                for (int j = 0; j < 10; j++) {
                    if (board[i-1][j]) {
                        board[i][j] = board[i-1][j];
                        board[i][j]->y += DY;
                    } else {
                        board[i][j] = NULL;
                    }
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
    new_tet->type = type;
    new_tet->arrangement = 0;

    /*  create blocks, put them into the new_tet.blocks arr. set their x and y
        coordinates. Track total tet x and y ranges, so we know to stop if any
        of them fall out of that range. */

    switch (type) {
        case 0: // stick block
            for (int i = 0; i < 4; ++i) {
                struct Block *new_block;
                new_block = (struct Block*) malloc(sizeof(struct Block));
                new_block->x = starting_x;
                new_block->y = starting_y - (i * DY);
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
            new_tet->blocks[2]->y = starting_y - DY;
            new_tet->blocks[3]->x = starting_x + DX;
            new_tet->blocks[3]->y = starting_y - DY;

            break;
        case 2: // T block;
            for (int i = 0; i < 4; ++i) {
                struct Block *new_block;
                new_block = (struct Block*) malloc(sizeof(struct Block));
                new_block->square = shapes[type];
                new_tet->blocks[i] = new_block;
            }
            new_tet->blocks[0]->x = starting_x;
            new_tet->blocks[0]->y = starting_y;
            new_tet->blocks[1]->x = starting_x;
            new_tet->blocks[1]->y = starting_y - DY;
            new_tet->blocks[2]->x = starting_x - DX;
            new_tet->blocks[2]->y = starting_y - DY;
            new_tet->blocks[3]->x = starting_x + DX;
            new_tet->blocks[3]->y = starting_y - DY;
            break;
        case 3: // El block;
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
            new_tet->blocks[2]->y = starting_y - DY;
            new_tet->blocks[3]->x = starting_x;
            new_tet->blocks[3]->y = starting_y - 2 * DY;
            break;
        case 4: // reverse el block;
            for (int i = 0; i < 4; ++i) {
                struct Block *new_block;
                new_block = (struct Block*) malloc(sizeof(struct Block));
                new_block->square = shapes[type];
                new_tet->blocks[i] = new_block;
            }
            new_tet->blocks[0]->x = starting_x;
            new_tet->blocks[0]->y = starting_y;
            new_tet->blocks[1]->x = starting_x - DX;
            new_tet->blocks[1]->y = starting_y;
            new_tet->blocks[2]->x = starting_x;
            new_tet->blocks[2]->y = starting_y - DY;
            new_tet->blocks[3]->x = starting_x;
            new_tet->blocks[3]->y = starting_y - 2 * DY;
            break;
        case 5: // dog block
            for (int i = 0; i < 4; ++i) {
                struct Block *new_block;
                new_block = (struct Block*) malloc(sizeof(struct Block));
                new_block->square = shapes[type];
                new_tet->blocks[i] = new_block;
            }
            new_tet->blocks[0]->x = starting_x;
            new_tet->blocks[0]->y = starting_y;
            new_tet->blocks[1]->x = starting_x - DX;
            new_tet->blocks[1]->y = starting_y;
            new_tet->blocks[2]->x = starting_x;
            new_tet->blocks[2]->y = starting_y - DY;
            new_tet->blocks[3]->x = starting_x + DX;
            new_tet->blocks[3]->y = starting_y - DY;
            break;
        case 6: // reverse dog block
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
            new_tet->blocks[2]->y = starting_y - DY;
            new_tet->blocks[3]->x = starting_x - DX;
            new_tet->blocks[3]->y = starting_y - DY;
            break;
        default:
            printf("Error: illegal tetromino type selection.\n");
            exit (1);
            break;
    }

    new_tet->x1 = new_tet->x2 = new_tet->blocks[0]->x; // This guarantees x1, etc.
    new_tet->y = new_tet->blocks[0]->y; // will actually update.
    for (int i = 0; i < 4; ++i) {
        new_tet->x1 = (new_tet->x1 > new_tet->blocks[i]->x) ?
                      new_tet->blocks[i]->x : new_tet->x1;
        new_tet->x2 = (new_tet->x2 < new_tet->blocks[i]->x) ?
                      new_tet->blocks[i]->x : new_tet->x2;
        new_tet->y = (new_tet->y > new_tet->blocks[i]->y) ?
                      new_tet->blocks[i]->y : new_tet->y;
    }
    return new_tet;
}

void rotate(struct Tetromino *current, int direction, struct Block *board[21][10]) {

    /* First we create a copy of current to apply rotations to. This way we can
       make changes, see if they are legal, and only then apply them to the
       current tetromino. */
    struct Tetromino *new;
    new = (struct Tetromino*) malloc(sizeof(struct Tetromino));
    new->type = current->type;
    new->x1 = current->x1;
    new->x2 = current->x2;
    new->y = current->y;
    new->arrangement = (current->arrangement + direction) % 4;
    if (new->arrangement < 0) { // % is remainder in C, not mod, so we can end
        new->arrangement += 4;  // with negative values.
    }

    for (int i = 0; i < 4; ++i) {
        struct Block *new_block;
        new_block = (struct Block*) malloc(sizeof(struct Block));
        new_block->x = current->blocks[i]->x;
        new_block->y = current->blocks[i]->y;
        new_block->square = current->blocks[i]->square;
        new->blocks[i] = new_block;
    }

    struct Block *cur_block; // Lets us look at individual blocks.
    int cur_y = new->blocks[0]->y; // Determine X and Y of center block,
    int cur_x = new->blocks[0]->x; // to modulate other blocks around.
    switch (new->type) {
        case 0: // line. Unfortunately lines work kind of wonky, because the central
                // block is the second one. This makes it rotate slightly nicer IMO,
                // but does lead to some slightly wonky arithmetic.
            switch (new->arrangement) {
                case 0: case 2:
                    for (int i = 0; i < 4; i++) {
                        cur_block = new->blocks[i];
                        cur_block->x = cur_x + DX;
                        cur_block->y = cur_y - (i * DY);
                    }
                    break;
                case 1: case 3:
                    for (int i = 0; i < 4; i++) {
                        cur_block = new->blocks[i];
                        cur_block->y = cur_y;
                        cur_block->x = cur_x + (i - 1) * DX;
                    }
                    break;
                default:
                    printf("Illegal arrangement of line block\n");
                    break;
                }
                break;

        case 1: // square
            break;
        case 2: // T block
            cur_block = new->blocks[0];
            int possible_x[4] = {cur_block->x - DX, cur_block->x,
                                 cur_block->x + DX, cur_block->x};
            int possible_y[4] = {cur_block->y, cur_block->y + DY,
                                 cur_block->y, cur_block->y - DY};
            for (int i = 0; i < 3; i++) {
                new->blocks[i+1]->x = possible_x[(i+new->arrangement) % 4];
                new->blocks[i+1]->y = possible_y[(i+new->arrangement) % 4];
            }
            break;
        case 3: case 4: // Cover both el blocks with ternary.
            switch (new->arrangement) {
                case 0:
                    new->blocks[1]->y = cur_y;
                    new->blocks[1]->x = (new->type == 3) ?
                                            cur_x + DX : cur_x - DX;
                    new->blocks[2]->y = cur_y - DY;
                    new->blocks[2]->x = cur_x;
                    new->blocks[3]->y = cur_y - 2 * DY;
                    new->blocks[3]->x = cur_x;
                    break;
                case 1:
                    new->blocks[1]->y = (new->type == 3) ?
                                            cur_y + DY : cur_y - DY;
                    new->blocks[1]->x = cur_x;
                    new->blocks[2]->y = cur_y;
                    new->blocks[2]->x = cur_x + DX;
                    new->blocks[3]->y = cur_y;
                    new->blocks[3]->x = cur_x + 2 * DX;
                    break;
                case 2:
                    new->blocks[1]->y = cur_y;
                    new->blocks[1]->x = (new->type == 3) ?
                                            cur_x - DX : cur_x + DX;
                    new->blocks[2]->y = cur_y + DY;
                    new->blocks[2]->x = cur_x;
                    new->blocks[3]->y = cur_y + 2 * DY;
                    new->blocks[3]->x = cur_x;
                    break;
                case 3:
                    new->blocks[1]->y = (new->type == 3) ?
                                            cur_y - DY : cur_y + DY;
                    new->blocks[1]->x = cur_x;
                    new->blocks[2]->y = cur_y;
                    new->blocks[2]->x = cur_x - DX;
                    new->blocks[3]->y = cur_y;
                    new->blocks[3]->x = cur_x - 2 * DX;
                    break;
                default:
                    printf("Error: Illegal arrangement of el block\n");
                    break;
            }
            break;
        case 5: case 6: // dog blocks, both. Swap X, then Y, of 1/3.
            switch (new->arrangement) {
                case 0: case 2:
                    new->blocks[1]->x = (new->type == 5) ?
                                            cur_x - DX : cur_x + DX;
                    new->blocks[1]->y = cur_y;
                    new->blocks[2]->x = cur_x;
                    new->blocks[2]->y = cur_y - DY;
                    new->blocks[3]->x = (new->type == 5) ?
                                            cur_x + DX : cur_x - DX;
                    new->blocks[3]->y = cur_y - DY;
                    break;
                case 1: case 3:
                    new->blocks[1]->x = cur_x;
                    new->blocks[1]->y = (new->type == 5) ?
                                            cur_y - DY : cur_y + DY;
                    new->blocks[2]->x = cur_x + DX;
                    new->blocks[2]->y = cur_y;
                    new->blocks[3]->x = cur_x + DX;
                    new->blocks[3]->y = (current->type == 5) ?
                                            cur_y + DY : cur_y - DY;
                    break;
                default:
                    printf("Error: illegal dog block arrangement\n");
                    break;
            }
            break;
        default:
            printf("Additional rotation commands TK\n");
            break;
    }

    if (rotation_is_legal(new, board)) {
    // Update x and y ranges for current tet.
        current->arrangement = new->arrangement;
        current->x1 = current->y = 10000; // arbitrarily high/low values to
        current->x2 = 0;                  // guarantee correct updates.

        for (int i = 0; i < 4; i++) {
            cur_block = current->blocks[i];
            cur_block->x = new->blocks[i]->x;
            cur_block->y = new->blocks[i]->y;
        }
        for (int i = 0; i < 4; i++) {
            cur_block = current->blocks[i];
            current->x1 = (current->x1 < cur_block->x) ?
                          current->x1 : cur_block-> x;
            current->x2 = (current->x2 > cur_block->x) ?
                          current->x2 : cur_block-> x;
            current->y = (current->y < cur_block->y) ?
                          current->y : cur_block-> y;
        }
        while (current->x1 < 0) {
            for (int i = 0; i < 4; i++) {
                current->blocks[i]->x += DX;
            }
            current->x1 += DX;
        }
        while (current->x2 > 10 * DX) {
            for (int i = 0; i < 4; i++) {
                current->blocks[i]->x -= DX;
            }
            current->x2 -= DX;
        }
        free(new);
    }
}


bool rotation_is_legal(struct Tetromino *new, struct Block *board[21][10]) {
    struct Block *cur_block;
    for (int i = 0; i < 4; ++i) {
        cur_block = new->blocks[i];
        int cur_x = cur_block->x / DX;
        int cur_y = cur_block->y / DY;
        if (board[cur_y][cur_x]) {
            return false;
        }
    }
    return true;
}
