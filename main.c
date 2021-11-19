#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2\SDL.h>

#include "noise.h"


// Window constants
const int WIDTH = 800;
const int HEIGHT = 600;
const int SIZE = 4;
const int FPS = 50;


// Custum struct
typedef struct {
    int r;
    int g;
    int b;
} Color;

typedef struct Civilization Civilization;

struct Civilization {
    int x;
    int y;
    int expance;
    Color* color;
    Civilization* next;
};


// Returns the smaller of a and b
int min(int a, int b) {
    if (a < b) return a;
    else return b;
}
// Returns the bigger of a and b
int max(int a, int b) {
    if (a > b) return a;
    else return b;
}


// Main function
int main(int argc, char* argv[]) {

    // Seeding random
    srand(time(NULL));

    // SDL innit
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("Title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
    WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* screen = SDL_CreateRenderer(window, -1, 0);

    int rows = HEIGHT / SIZE;
    int columns = WIDTH / SIZE;

    // Event variable
    SDL_Event event;

    // Input variables
    bool mouse_left_down = false;
    bool mouse_right_down = false;
    int mouse_pos_x = 0;
    int mouse_pos_y = 0;

    // FPS vriables
    int delay = 1000 / FPS;
    int sleep = 0;

    // Time variables
    unsigned int start_time = 0;
    unsigned int end_time = 0;
    unsigned int delta = 0;

    // Background variables
    float zoom = 23.39;
    int offset = rand() % 1000000;

    // Finds local min and max
    float fmin = 2, fmax = 0;
    for (float row = 0; row < rows; row++) {
        for (float col = 0; col < columns; col++) {

            float noise = 0;
            noise += perlin((row+offset)/zoom, (col+offset)/zoom) + 1;
            noise += (perlin((row+offset)/zoom*2, (col+offset)/zoom*2) + 1) / 2;
            noise += (perlin((row+offset)/zoom*4, (col+offset)/zoom*4) + 1) / 4;

            if (noise < fmin) fmin = noise;
            if (noise > fmax) fmax = noise;
        }
    }

    // Generates a nice background
    Color background[rows][columns];
    for (float row = 0; row < rows; row++) {
        for (float col = 0; col < columns; col++) {

            float noise = 0;
            noise += perlin((row+offset)/zoom, (col+offset)/zoom) + 1;
            noise += (perlin((row+offset)/zoom*2, (col+offset)/zoom*2) + 1) / 2;
            noise += (perlin((row+offset)/zoom*4, (col+offset)/zoom*4) + 1) / 4;
            noise = (noise - fmin) / (fmax - fmin);

            Color back;
            back.r = max(3, (int)(20 * noise * noise) + 3);
            back.g = max(3, (int)(25 * noise * noise) + 3);
            back.b = max(6, (int)(40 * noise * noise) + 6);
            background[(int)row][(int)col] = back;
        }
    }

    // Civilization variables
    Civilization* civ = NULL;
    int creation_chance = 3 * FPS;  // per second
    int expansion_speed = 1 * FPS / 2;  // per second
    int expansion_counter = 0;
    int civ_id = 1;

    // Civilization id map
    int civ_map_id[rows][columns];
    Color* civ_map_color[rows][columns];
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < columns; col++) {
            civ_map_id[row][col] = 0;
        }
    }

    // Display variables
    SDL_Rect square;
    square.w = SIZE;
    square.h = SIZE;

    // Main loop
    bool running = true;
    while (running) {

        start_time = SDL_GetTicks();  // Gets loop start time

        // Get mouse position
        SDL_GetMouseState(&mouse_pos_x, &mouse_pos_y);  // Gets mouse position

        // Handles events
        SDL_PollEvent(&event);  // Gets event
        if (event.type == SDL_QUIT) {
            running = false;
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {  // Mouse down
            if (event.button.button == SDL_BUTTON_LEFT && !mouse_left_down) {  // Left click
                mouse_left_down = true;
            }
            else if (event.button.button == SDL_BUTTON_RIGHT && !mouse_right_down) {  // Right click
                mouse_right_down = true;
            }
        }
        else if (event.type == SDL_MOUSEBUTTONUP) {  // Mouse up
            if (event.button.button == SDL_BUTTON_LEFT && mouse_left_down) {  // Left click
                mouse_left_down = false;
            }
            else if (event.button.button == SDL_BUTTON_RIGHT && mouse_right_down) {  // Rightclick
                mouse_right_down = false;
            }
        }

        // Create civilizations
        if (rand() % creation_chance == 0) {

            Civilization* new_civ = malloc(sizeof(Civilization));
            int row = rand() % rows;
            int col = rand() % columns;

            if (civ_map_id[row][col] == 0) {

                new_civ->x = row;
                new_civ->y = col;

                new_civ->expance = 1;

                Color* color = malloc(sizeof(Color));
                color->r = rand() % 256;
                color->g = rand() % 256;
                color->b = rand() % 256;
                new_civ->color = color;

                civ_map_color[row][col] = color;
                civ_map_id[row][col] = civ_id;
                civ_id += 1;

                new_civ->next = civ;
                civ = new_civ;
            }
        }

        // Expand civilizations
        if (expansion_counter >= expansion_speed) {
            expansion_counter = 0;

            Civilization* exp_civ = civ;
            while (exp_civ != NULL) {
                
                int row = exp_civ->x;
                int col = exp_civ->y;
                int range = exp_civ->expance;
                bool expanded = false;
                for (int i=max(0, row - range); i<min(rows, row + range+1); i++) {
                    for (int j=max(0, col - range); j<min(columns, col + range+1); j++) {

                        if (civ_map_id[i][j] != 0) continue;
                        
                        if ( (row-i)*(row-i) + (col-j)*(col-j) <= range*range ) {
                            
                            civ_map_id[i][j] = civ_map_id[row][col];
                            civ_map_color[i][j] = exp_civ->color;
                            expanded = true;
                        }
                    }
                }
                if (expanded) exp_civ->expance++;
                else exp_civ->expance = 0;

                exp_civ = exp_civ->next;
            }
        }
        else expansion_counter++;

        // Enables alpha
        SDL_SetRenderDrawBlendMode(screen, SDL_BLENDMODE_BLEND);
        
        // Clears screen
        SDL_RenderClear(screen);

        // Display background
        for (int row=0; row < rows; row++) {
            for (int col=0; col < columns; col++) {

                Color back = background[row][col];

                square.x = col * SIZE;
                square.y = row * SIZE;

                SDL_SetRenderDrawColor(screen, back.r, back.g, back.b, 255);
                SDL_RenderFillRect(screen, &square);
            }
        }

        // Display civilizations
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < columns; col++) {

                if (civ_map_id[row][col] == 0) continue;

                Color* color = civ_map_color[row][col];

                square.x = col * SIZE;
                square.y = row * SIZE;

                SDL_SetRenderDrawColor(screen, color->r, color->g, color->b, 50);
                SDL_RenderFillRect(screen, &square);
            }
        }

        // Fills the screen
        SDL_SetRenderDrawColor(screen, 0, 0, 0, 255);
        SDL_RenderPresent(screen);

        end_time = SDL_GetTicks();  // Gets loop end time
        delta = end_time - start_time;  // Calculates loop execution time

        sleep = delay - delta;  // Calculates sleep time
        if (sleep < 0) sleep = 0;  // Prevents delay smaller then 0

        SDL_Delay(sleep);  // Starts delay
    }

    SDL_Quit();
    return 0;
}
