#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#define N 5

typedef struct Vec2 {
    float x; 
    float y;
} Vec2;

typedef struct Ship {
    Vec2 pos;
    Vec2 vel;
    float angle;
    bool alive;
} Ship;

Vec2 vec2_add(Vec2 a, Vec2 b){
    Vec2 res;
    res.x = a.x + b.x;
    res.y = a.y + b.y;

    return res;
}

void vec2_add_in_place(Vec2 *a, Vec2 b){
    (*a).x = (*a).x + b.x;
    (*a).y = (*a).y + b.y;
}

float random_float(float low, float high){
    float unit = (float) arc4random() / (float) UINT32_MAX;
    return low + (high - low) * unit;
}

Vec2 initialize_vector(int upper_limit){
    Vec2 vector = {
        .x = random_float(0, upper_limit),
        .y = random_float(0, upper_limit),
    };

    return vector;
}


void update(Vec2 positions[], Vec2 velocities[], int size, float dt) {
    for (int i = 0; i < size; i++){
        positions[i].x = positions[i].x + velocities[i].x * dt;
        positions[i].y = positions[i].y + velocities[i].y * dt;
    }
}


int main(void){
    int position_upper_limit = 10;
    int velocities_upper_limit = 10; 

    int steps = 10;
    float tick = 0.1f;
    float current_time = 0.0f;

    Vec2 positions[N];
    Vec2 velocities[N];

    for (int i = 0; i < N; i++){
        positions[i] = initialize_vector(position_upper_limit);
        velocities[i] = initialize_vector(velocities_upper_limit);
    }

    printf("Intial positions and velocities:\n");
    for (int i = 0; i < N; i++){
        printf("position[%d] = (%f, %f)\n", i, positions[i].x, positions[i].y);
        printf("velocity[%d] = (%f, %f)\n", i, velocities[i].x, velocities[i].y);
    }

    for (int curr_step = 0; curr_step < steps; curr_step++){
        printf("\nPositions at step %d:\n", curr_step);

        update(positions, velocities, N, tick);

        for (int i = 0; i < N; i++){
            printf("position[%d] = (%f, %f)\n", i, positions[i].x, positions[i].y);
        }

        current_time += tick;
    }

    return 0;
}
