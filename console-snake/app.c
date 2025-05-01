#include <stdio.h>    // (Standard input/output: printf, fprintf)
#include <stdlib.h>   // (Memory allocation: malloc, free; rand, exit)
#include <time.h>     // (Time functions: time for random seed)
#include <conio.h>    // (Console I/O: kbhit, getch for keyboard input)
#include <windows.h>  // (Windows API: Sleep for delays)

// Constants for game grid size
#define WIDTH 20  // (Width of the game area)
#define HEIGHT 20 // (Height of the game area)

// Struct for x, y coordinates (groups related data into a single unit)
typedef struct {
    int x;
    int y;
} Position;

// Struct for snake segments, forming a linked list (each segment links to the previous)
typedef struct SnakeSegment {
    Position pos;             // (Position of this segment)
    struct SnakeSegment *prev; // (Pointer to the previous segment)
} SnakeSegment;

// Enum for snake direction (named constants for clarity)
typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

// Global variables (stored in memory, accessible everywhere)
SnakeSegment *head; // (Pointer to snake's head)
SnakeSegment *tail; // (Pointer to snake's tail)
Position food;      // (Position of food)
Direction dir;      // (Current direction)
int score;          // (Player's score)
int game_over;      // (Flag: 0 = running, 1 = game over)

// Function declarations
int is_on_snake(Position pos); // (Checks if position is on snake)
void generate_food();           // (Places new food randomly)
void draw();                   // (Draws game grid)
void move_snake();             // (Moves snake, checks collisions)
void free_snake();             // (Frees snake memory)

int main() {
    // Seed random number generator (ensures different random numbers each run)
    srand(time(NULL));

    // Initialize snake at center
    head = malloc(sizeof(SnakeSegment)); // (Allocate memory for head)
    if (head == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return EXIT_FAILURE;
    }
    head->pos = (Position){WIDTH / 2, HEIGHT / 2}; // (Start at 10,10)
    head->prev = NULL; // (No previous segment)
    tail = head;       // (Tail is head initially)
    dir = RIGHT;       // (Start moving right)
    score = 0;         // (Score starts at 0)
    game_over = 0;     // (Game is running)
    generate_food();   // (Place first food)

    // Game loop
    while (!game_over) {
        draw();          // (Show game state)
        Sleep(100);      // (Pause 100ms; Windows equivalent of usleep)

        // Check for keypress and update direction
        if (kbhit()) {   // (Check if a key is pressed, non-blocking)
            char key = getch(); // (Get key without waiting)
            switch (key) {
                case 'w': case 'W': if (dir != DOWN) dir = UP; break;    // (W moves up)
                case 's': case 'S': if (dir != UP) dir = DOWN; break;    // (S moves down)
                case 'a': case 'A': if (dir != RIGHT) dir = LEFT; break; // (A moves left)
                case 'd': case 'D': if (dir != LEFT) dir = RIGHT; break; // (D moves right)
            }
        }

        move_snake(); // (Update snake position)
    }

    // Game over message
    printf("\033[2J"); // (Clear screen with ANSI escape)
    printf("\033[H");  // (Move cursor to top-left)
    printf("Game Over! Final Score: %d\n", score);

    // Clean up
    free_snake(); // (Free snake memory)
    return EXIT_SUCCESS; // (Exit successfully)
}

// Check if a position overlaps with the snake
int is_on_snake(Position pos) {
    SnakeSegment *current = head; // (Start at head)
    while (current != NULL) {     // (Traverse linked list)
        if (current->pos.x == pos.x && current->pos.y == pos.y) {
            return 1;             // (Position is on snake)
        }
        current = current->prev;  // (Move to previous segment)
    }
    return 0;                     // (Position is not on snake)
}

// Place food at a random spot not on the snake
void generate_food() {
    do {
        food.x = rand() % (WIDTH - 2) + 1;  // (Random x from 1 to 18)
        food.y = rand() % (HEIGHT - 2) + 1; // (Random y from 1 to 18)
    } while (is_on_snake(food));            // (Repeat if on snake)
}

// Draw the game grid
void draw() {
    printf("\033[2J"); // (Clear screen)
    printf("\033[H");  // (Move cursor to top-left)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            Position pos = {x, y};
            if (x == 0 || x == WIDTH - 1 || y == 0 || y == HEIGHT - 1) {
                printf("#"); // (Boundary)
            } else if (pos.x == head->pos.x && pos.y == head->pos.y) {
                printf("@"); // (Snake head)
            } else if (is_on_snake(pos)) {
                printf("o"); // (Snake body)
            } else if (pos.x == food.x && pos.y == food.y) {
                printf("*"); // (Food)
            } else {
                printf(" "); // (Empty space)
            }
        }
        printf("\n"); // (New line after row)
    }
    printf("Score: %d\n", score); // (Show score)
}

// Move the snake and handle collisions
void move_snake() {
    Position new_pos = head->pos;
    switch (dir) {
        case UP: new_pos.y--; break;
        case DOWN: new_pos.y++; break;
        case LEFT: new_pos.x--; break;
        case RIGHT: new_pos.x++; break;
    }

    // Check collisions with boundaries or self
    if (new_pos.x <= 0 || new_pos.x >= WIDTH - 1 || 
        new_pos.y <= 0 || new_pos.y >= HEIGHT - 1 || 
        is_on_snake(new_pos)) {
        game_over = 1;
        return;
    }

    // Add new head
    SnakeSegment *new_head = malloc(sizeof(SnakeSegment));
    if (new_head == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        game_over = 1;
        return;
    }
    new_head->pos = new_pos;
    new_head->prev = head;
    head = new_head;

    // Check if food is eaten
    if (new_pos.x == food.x && new_pos.y == food.y) {
        score++;
        generate_food();
    } else {
        // Remove tail correctly
        if (head->prev != NULL) { // Ensure there’s at least one segment before tail
            SnakeSegment *current = head;
            while (current->prev != tail) {
                current = current->prev;
            }
            // current->prev is tail; make current the new tail
            SnakeSegment *old_tail = tail;
            tail = current;
            tail->prev = NULL;
            free(old_tail);
        }
    }
}

// Free all snake segments
void free_snake() {
    while (head != NULL) {     // (Loop until no segments)
        SnakeSegment *temp = head;
        head = head->prev;     // (Move to previous)
        free(temp);            // (Free current)
    }
}