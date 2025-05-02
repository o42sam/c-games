#include <stdio.h>    
#include <stdlib.h>   
#include <time.h>     
#include <conio.h>    
#include <windows.h>  

#define GAME_BOARD_WIDTH 60  
#define GAME_BOARD_HEIGHT 20 

typedef struct {
    int x_coordinate;
    int y_coordinate;
} Position;

typedef struct SnakeSegment {
    Position position;             
    struct SnakeSegment *previous_segment; 
} SnakeSegment;

typedef enum {
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} Direction;

SnakeSegment *snake_head; 
SnakeSegment *snake_tail; 
Position food_position;      
Direction current_direction;      
int player_score;          
int is_game_over;      

int is_position_on_snake(Position position); 
void spawn_new_food();           
void render_game_board();                   
void update_snake_position();             
void deallocate_snake();             

int main() {
    
    srand(time(NULL));

    
    snake_head = malloc(sizeof(SnakeSegment)); 
    if (snake_head == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return EXIT_FAILURE;
    }
    snake_head->position = (Position){GAME_BOARD_WIDTH / 2, GAME_BOARD_HEIGHT / 2}; 
    snake_head->previous_segment = NULL; 
    snake_tail = snake_head;       
    current_direction = DIRECTION_RIGHT;       
    player_score = 0;         
    is_game_over = 0;     
    spawn_new_food();   
    
    while (!is_game_over) {
        DWORD frame_start_time = GetTickCount(); 

        render_game_board(); 

        
        if (kbhit()) {
            char input_key = getch();
            switch (input_key) {
                case 'w': case 'W': if (current_direction != DIRECTION_DOWN) current_direction = DIRECTION_UP; break;
                case 's': case 'S': if (current_direction != DIRECTION_UP) current_direction = DIRECTION_DOWN; break;
                case 'a': case 'A': if (current_direction != DIRECTION_RIGHT) current_direction = DIRECTION_LEFT; break;
                case 'd': case 'D': if (current_direction != DIRECTION_LEFT) current_direction = DIRECTION_RIGHT; break;
            }
        }

        update_snake_position(); 

        
        DWORD frame_duration = GetTickCount() - frame_start_time;
        DWORD target_frame_duration = 100; 
        if (frame_duration < target_frame_duration) {
            Sleep(target_frame_duration - frame_duration);
        }
    }

    
    printf("\033[2J"); 
    printf("\033[H");  
    printf("Game Over! Final Score: %d\n", player_score);

    
    deallocate_snake(); 
    return EXIT_SUCCESS; 
}

int is_position_on_snake(Position position) {
    SnakeSegment *current_segment = snake_head; 
    while (current_segment != NULL) {     
        if (current_segment->position.x_coordinate == position.x_coordinate && current_segment->position.y_coordinate == position.y_coordinate) {
            return 1;             
        }
        current_segment = current_segment->previous_segment;  
    }
    return 0;                     
}

void spawn_new_food() {
    do {
        food_position.x_coordinate = rand() % (GAME_BOARD_WIDTH - 2) + 1;  
        food_position.y_coordinate = rand() % (GAME_BOARD_HEIGHT - 2) + 1; 
    } while (is_position_on_snake(food_position));            
}

void render_game_board() {
    
    CHAR_INFO display_buffer[GAME_BOARD_HEIGHT][GAME_BOARD_WIDTH];
    COORD buffer_dimensions = {GAME_BOARD_WIDTH, GAME_BOARD_HEIGHT};
    COORD buffer_coordinates = {0, 0};
    SMALL_RECT display_region = {0, 0, GAME_BOARD_WIDTH - 1, GAME_BOARD_HEIGHT - 1};

    
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    
    for (int y = 0; y < GAME_BOARD_HEIGHT; y++) {
        for (int x = 0; x < GAME_BOARD_WIDTH; x++) {
            Position current_position = {x, y};
            char display_character;
            if (x == 0 || x == GAME_BOARD_WIDTH - 1 || y == 0 || y == GAME_BOARD_HEIGHT - 1) {
                display_character = '#'; 
            } else if (current_position.x_coordinate == snake_head->position.x_coordinate && current_position.y_coordinate == snake_head->position.y_coordinate) {
                display_character = '@'; 
            } else if (is_position_on_snake(current_position)) {
                display_character = 'o'; 
            } else if (current_position.x_coordinate == food_position.x_coordinate && current_position.y_coordinate == food_position.y_coordinate) {
                display_character = '*'; 
            } else {
                display_character = ' '; 
            }
            display_buffer[y][x].Char.AsciiChar = display_character;
            display_buffer[y][x].Attributes = FOREGROUND_GREEN; 
        }
    }

    
    WriteConsoleOutput(console_handle, (CHAR_INFO *)display_buffer, buffer_dimensions, buffer_coordinates, &display_region);

    
    COORD score_display_position = {0, GAME_BOARD_HEIGHT};
    SetConsoleCursorPosition(console_handle, score_display_position);
    printf("Score: %d", player_score);
}

void update_snake_position() {
    Position new_head_position = snake_head->position;
    switch (current_direction) {
        case DIRECTION_UP: new_head_position.y_coordinate--; break;
        case DIRECTION_DOWN: new_head_position.y_coordinate++; break;
        case DIRECTION_LEFT: new_head_position.x_coordinate--; break;
        case DIRECTION_RIGHT: new_head_position.x_coordinate++; break;
    }

    
    if (new_head_position.x_coordinate <= 0 || new_head_position.x_coordinate >= GAME_BOARD_WIDTH - 1 || 
        new_head_position.y_coordinate <= 0 || new_head_position.y_coordinate >= GAME_BOARD_HEIGHT - 1 || 
        is_position_on_snake(new_head_position)) {
        is_game_over = 1;
        return;
    }

    
    SnakeSegment *new_head_segment = malloc(sizeof(SnakeSegment));
    if (new_head_segment == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        is_game_over = 1;
        return;
    }
    new_head_segment->position = new_head_position;
    new_head_segment->previous_segment = snake_head;
    snake_head = new_head_segment;

    
    if (new_head_position.x_coordinate == food_position.x_coordinate && new_head_position.y_coordinate == food_position.y_coordinate) {
        player_score++;
        spawn_new_food();
    } else {
        
        if (snake_head->previous_segment != NULL) { 
            SnakeSegment *current_segment = snake_head;
            while (current_segment->previous_segment != snake_tail) {
                current_segment = current_segment->previous_segment;
            }
            
            SnakeSegment *old_tail_segment = snake_tail;
            snake_tail = current_segment;
            snake_tail->previous_segment = NULL;
            free(old_tail_segment);
        }
    }
}

void deallocate_snake() {
    while (snake_head != NULL) {     
        SnakeSegment *temp_segment = snake_head;
        snake_head = snake_head->previous_segment;     
        free(temp_segment);            
    }
}