#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static const float MIN_SPEED_BOUND = -60.0f;
static const float MAX_SPEED_BOUND = 60.0f;

typedef struct Vec2 {
    float x; 
    float y;
} Vec2;

typedef struct Asteroid {
    Vec2 position;
    Vec2 velocity;
} Asteroid;


float random_float(float low, float high){
    float unit = (float) arc4random() / (float) UINT32_MAX;
    return low + (high - low) * unit;
}

void initialize_asteroids(Asteroid asteroids[], int number, int low, int high){
    for (int i = 0; i < number; i++){
        asteroids[i].position = (Vec2){
            random_float((float)low, (float)high),
            random_float((float)low, (float)high)
        };
        asteroids[i].velocity = (Vec2){
            random_float(MIN_SPEED_BOUND, MAX_SPEED_BOUND),
            random_float(MIN_SPEED_BOUND, MAX_SPEED_BOUND)
        };
    }
}


void update(Vec2 *position, Vec2 *velocity, float dt) {
    position->x = position->x + velocity->x * dt;
    position->y = position->y + velocity->y * dt;
}


void calculate_next_asteroids_coordinates(Asteroid asteroids[], int number, float dt){
    for (int i = 0; i < number; i++){
        update(&(asteroids[i].position), &(asteroids[i].velocity), dt);
    }
}

void draw_asteroids(Asteroid asteroids[], int number){
    for (int i = 0; i < number; i++){
        DrawCircle(asteroids[i].position.x, asteroids[i].position.y, 3, GRAY);
    }
}


int main(void){
    int low = 0;
    int hight = 900;
    int number_of_asteroids = 100;

    InitWindow(hight, hight, "raylib game flow testing");
    SetTargetFPS(60);
    int i = 0;

    Asteroid asteroids[number_of_asteroids];
    initialize_asteroids(asteroids, number_of_asteroids, low, hight);

    while (!WindowShouldClose()){
        if (i < 20){
            i++;
        }
        
        float dt = (float)GetFrameTime();
        printf("Time elapsed: %f\n", dt);
        calculate_next_asteroids_coordinates(asteroids, number_of_asteroids, dt);

        BeginDrawing();

            ClearBackground(RAYWHITE);
            // DrawText("Hey mate!", 190 + i * 10, 200, 20, LIGHTGRAY);
            draw_asteroids(asteroids, number_of_asteroids);

        EndDrawing();

        int key_pressed = GetKeyPressed();
        if (key_pressed == KEY_A) {
            i = 0;
        } 
    }

    CloseWindow();

    return 0;
}
