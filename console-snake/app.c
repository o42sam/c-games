#include <stdio.h>    
#include <stdlib.h>   
#include <time.h>     
#include <conio.h>    
#include <windows.h>  


#define WIDTH 60  
#define HEIGHT 20 


typedef struct {
    int x;
    int y;
} Position;


typedef struct SnakeSegment {
    Position pos;             
    struct SnakeSegment *prev; 
} SnakeSegment;


typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;


SnakeSegment *head; 
SnakeSegment *tail; 
Position food;      
Direction dir;      
int score;          
int game_over;      


int is_on_snake(Position pos); 
void generate_food();           
void draw();                   
void move_snake();             
void free_snake();             

int main() {
    
    srand(time(NULL));

    
    head = malloc(sizeof(SnakeSegment)); 
    if (head == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return EXIT_FAILURE;
    }
    head->pos = (Position){WIDTH / 2, HEIGHT / 2}; 
    head->prev = NULL; 
    tail = head;       
    dir = RIGHT;       
    score = 0;         
    game_over = 0;     
    generate_food();   
    
    while (!game_over) {
        DWORD startTime = GetTickCount(); 

        draw(); 

        
        if (kbhit()) {
            char key = getch();
            switch (key) {
                case 'w': case 'W': if (dir != DOWN) dir = UP; break;
                case 's': case 'S': if (dir != UP) dir = DOWN; break;
                case 'a': case 'A': if (dir != RIGHT) dir = LEFT; break;
                case 'd': case 'D': if (dir != LEFT) dir = RIGHT; break;
            }
        }

        move_snake(); 

        
        DWORD frameTime = GetTickCount() - startTime;
        DWORD targetFrameTime = 100; 
        if (frameTime < targetFrameTime) {
            Sleep(targetFrameTime - frameTime);
        }
    }

    
    printf("\033[2J"); 
    printf("\033[H");  
    printf("Game Over! Final Score: %d\n", score);

    
    free_snake(); 
    return EXIT_SUCCESS; 
}


int is_on_snake(Position pos) {
    SnakeSegment *current = head; 
    while (current != NULL) {     
        if (current->pos.x == pos.x && current->pos.y == pos.y) {
            return 1;             
        }
        current = current->prev;  
    }
    return 0;                     
}


void generate_food() {
    do {
        food.x = rand() % (WIDTH - 2) + 1;  
        food.y = rand() % (HEIGHT - 2) + 1; 
    } while (is_on_snake(food));            
}


void draw() {
    
    CHAR_INFO buffer[HEIGHT][WIDTH];
    COORD bufferSize = {WIDTH, HEIGHT};
    COORD bufferCoord = {0, 0};
    SMALL_RECT writeRegion = {0, 0, WIDTH - 1, HEIGHT - 1};

    
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            Position pos = {x, y};
            char c;
            if (x == 0 || x == WIDTH - 1 || y == 0 || y == HEIGHT - 1) {
                c = '#'; 
            } else if (pos.x == head->pos.x && pos.y == head->pos.y) {
                c = '@'; 
            } else if (is_on_snake(pos)) {
                c = 'o'; 
            } else if (pos.x == food.x && pos.y == food.y) {
                c = '*'; 
            } else {
                c = ' '; 
            }
            buffer[y][x].Char.AsciiChar = c;
            buffer[y][x].Attributes = FOREGROUND_GREEN; 
        }
    }

    
    WriteConsoleOutput(hConsole, (CHAR_INFO *)buffer, bufferSize, bufferCoord, &writeRegion);

    
    COORD scorePos = {0, HEIGHT};
    SetConsoleCursorPosition(hConsole, scorePos);
    printf("Score: %d", score);
}


void move_snake() {
    Position new_pos = head->pos;
    switch (dir) {
        case UP: new_pos.y--; break;
        case DOWN: new_pos.y++; break;
        case LEFT: new_pos.x--; break;
        case RIGHT: new_pos.x++; break;
    }

    
    if (new_pos.x <= 0 || new_pos.x >= WIDTH - 1 || 
        new_pos.y <= 0 || new_pos.y >= HEIGHT - 1 || 
        is_on_snake(new_pos)) {
        game_over = 1;
        return;
    }

    
    SnakeSegment *new_head = malloc(sizeof(SnakeSegment));
    if (new_head == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        game_over = 1;
        return;
    }
    new_head->pos = new_pos;
    new_head->prev = head;
    head = new_head;

    
    if (new_pos.x == food.x && new_pos.y == food.y) {
        score++;
        generate_food();
    } else {
        
        if (head->prev != NULL) { 
            SnakeSegment *current = head;
            while (current->prev != tail) {
                current = current->prev;
            }
            
            SnakeSegment *old_tail = tail;
            tail = current;
            tail->prev = NULL;
            free(old_tail);
        }
    }
}


void free_snake() {
    while (head != NULL) {     
        SnakeSegment *temp = head;
        head = head->prev;     
        free(temp);            
    }
}