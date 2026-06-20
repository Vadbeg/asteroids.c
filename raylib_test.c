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


void update(Vec2 *position, Vec2 *velocity, float dt, int high) {
    // position->x = position->x + velocity->x * dt;
    // position->y = position->y + velocity->y * dt;

    float new_x = position->x + velocity->x * dt;
    float new_y = position->y + velocity->y * dt;

    if (new_y >= high){
        new_y = 0.0f;
    }
    else if (new_y <= 0.0f){
        new_y = high;
    }

    if (new_x >= high){
        new_x = 0.0f;
    }
    else if (new_x <= 0.0f){
        new_x = high;
    }

    position->x = new_x;
    position->y = new_y;
}


void calculate_next_asteroids_coordinates(Asteroid asteroids[], int number, float dt, int high){
    for (int i = 0; i < number; i++){
        update(&(asteroids[i].position), &(asteroids[i].velocity), dt, high);
    }
}

void draw_asteroids(Asteroid asteroids[], int number){
    for (int i = 0; i < number; i++){
        DrawCircle(asteroids[i].position.x, asteroids[i].position.y, 5, GRAY);
    }
}


int main(void){
    int low = 0;
    int hight = 900;
    int number_of_asteroids = 100;

    InitWindow(hight, hight, "raylib game flow testing");
    SetTargetFPS(60);

    Asteroid asteroids[number_of_asteroids];
    initialize_asteroids(asteroids, number_of_asteroids, low, hight);

    while (!WindowShouldClose()){
        float dt = (float)GetFrameTime();
        calculate_next_asteroids_coordinates(asteroids, number_of_asteroids, dt, hight);

        BeginDrawing();

            ClearBackground(RAYWHITE);
            draw_asteroids(asteroids, number_of_asteroids);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
