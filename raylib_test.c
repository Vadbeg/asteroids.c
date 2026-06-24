#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static const float MIN_SPEED_BOUND = -120.0f;
static const float MAX_SPEED_BOUND = 120.0f;

typedef struct Vec2 {
    float x; 
    float y;
} Vec2;

typedef struct Asteroid {
    Vec2 position;
    Vec2 velocity;
} Asteroid;

typedef struct Ship{
    Vec2 position;
    Vec2 velocity;
    float angle;
} Ship;


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


void wrap_around(Vec2 *position, int radius, int high){
    if (position->y - (float)radius >= high){
        position->y = 0.0f - (float)radius;
    }
    else if (position->y + (float)radius <= 0.0f){
        position->y = high + (float)radius;
    }

    if (position->x - (float)radius >= high){
        position->x = 0.0f - (float)radius;
    }
    else if (position->x + (float)radius <= 0.0f){
        position->x = high + (float)radius;
    }
}


void update(Vec2 *position, Vec2 *velocity, float dt, int high, int radius) {
    position->x = position->x + velocity->x * dt;
    position->y = position->y + velocity->y * dt;

    wrap_around(position, radius, high);
}


void change_speed(Vec2 *velocity, float alpha, float ceiling){
    float speed = sqrtf(velocity->x * velocity->x + velocity->y * velocity->y);
    printf("Speed: %f\n", speed);
    
    if (speed < ceiling || alpha < 0){
        velocity->x = velocity->x + alpha;
        velocity->y = velocity->y + alpha;
    }

    if (velocity->x < 0){
        velocity->x = 0;
    }

    if (velocity->y < 0){
        velocity->y = 0;
    }
}


void calculate_next_asteroids_coordinates(Asteroid asteroids[], int number, int radius, float dt, int high){
    for (int i = 0; i < number; i++){
        update(&(asteroids[i].position), &(asteroids[i].velocity), dt, high, radius);
    }
}

void calculate_next_ship_coordinates(Ship *ship, int radius, float dt, int high){
    update(&ship->position, &ship->velocity, dt, high, radius);
}

void draw_asteroids(Asteroid asteroids[], int number, int radius){
    for (int i = 0; i < number; i++){
        DrawCircle(asteroids[i].position.x, asteroids[i].position.y, radius, GRAY);
    }
}

void draw_ship(Ship ship, int radius){
    DrawCircle(ship.position.x, ship.position.y, radius, GREEN);
}


int main(void){
    int low = 0;
    int hight = 900;
    int number_of_asteroids = 5;

    int hitbox_radius = 30;
    int ship_radius = 10;

    float max_speed = 50.0f;
    float acceleration = 0.1f;
    float deceleration = -0.2f;

    Vec2 start_ship_position = {
        .x=450,
        .y=450
    };
    Vec2 start_ship_velocity = {
        .x=0,
        .y=0
    };
    Ship ship = {
        .position=start_ship_position,
        .velocity=start_ship_velocity,
    };

    InitWindow(hight, hight, "raylib game flow testing");
    SetTargetFPS(60);

    Asteroid asteroids[number_of_asteroids];
    initialize_asteroids(asteroids, number_of_asteroids, low, hight);

    while (!WindowShouldClose()){
        float dt = (float)GetFrameTime();
        calculate_next_asteroids_coordinates(asteroids, number_of_asteroids, hitbox_radius, dt, hight);
        calculate_next_ship_coordinates(&ship, ship_radius, dt, hight);

        if (IsKeyDown(KEY_UP)){
            change_speed(&ship.velocity, acceleration, max_speed);
        } else {
            change_speed(&ship.velocity, deceleration, max_speed);
        }

        BeginDrawing();

            ClearBackground(RAYWHITE);
            draw_asteroids(asteroids, number_of_asteroids, hitbox_radius);
            draw_ship(ship, ship_radius);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
