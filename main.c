#include <math.h>
#include <raylib.h>
#include <stdbool.h>
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
    bool alive;
} Asteroid;

typedef struct Ship{
    Vec2 position;
    Vec2 velocity;
    float angle;
} Ship;

typedef struct Bullet{
    Vec2 position;
    Vec2 velocity;
    bool alive;
    float distance_traveled;
} Bullet;


float random_float(float low, float high){
    float unit = (float) arc4random() / (float) UINT32_MAX;
    return low + (high - low) * unit;
}

float get_distance(Vec2 position1, Vec2 position2){
    float distance = sqrtf(powf(position1.x - position2.x, 2) + powf(position1.y - position2.y, 2));
    return distance;
}

bool is_collision(Vec2 position1, Vec2 position2, int radius1, int radius2){
    float distance_between = get_distance(position1, position2);

    if (distance_between <= (float)radius1 + (float)radius2){
        return true;
    }
    
    return false;
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
        asteroids[i].alive = true;
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


void update(Vec2 *position, Vec2 velocity, float dt, int high, int radius) {
    position->x = position->x + velocity.x * dt;
    position->y = position->y + velocity.y * dt;

    wrap_around(position, radius, high);
}


Vec2 get_direction(float angle){
    float radians = angle * DEG2RAD;

    Vec2 direction = {
        .x = cosf(radians),
        .y = sinf(radians)
    };
    return direction;
}


void change_speed(Vec2 *velocity, float angle, float acceleration, float dt){
    Vec2 direction = get_direction(angle);
    
    velocity->x = velocity->x + direction.x * acceleration * dt;
    velocity->y = velocity->y + direction.y * acceleration * dt;
}


void apply_friction(Vec2 *velocity, float friction, float dt){
    velocity->x = velocity->x * friction;
    velocity->y = velocity->y * friction;
}

void calculate_next_asteroids_coordinates(Asteroid asteroids[], int number, int radius, float dt, int high){
    for (int i = 0; i < number; i++){
        update(&(asteroids[i].position), asteroids[i].velocity, dt, high, radius);
    }
}

void calculate_next_ship_coordinates(Ship *ship, int radius, float dt, int high){
    update(&ship->position, ship->velocity, dt, high, radius);
}

void calculate_next_bullet_coordinates(Bullet bullets[], int length, int radius, float dt, int high){
    for (int i = 0; i < length; i++){
        if (bullets[i].alive){
            update(&(bullets[i].position), bullets[i].velocity, dt, high, radius);
            bullets[i].distance_traveled += sqrtf(powf(bullets[i].velocity.x * dt, 2) + powf(bullets[i].velocity.y * dt, 2));

            if (bullets[i].distance_traveled > (float)high / 2){
                bullets[i].alive = false;
                bullets[i].distance_traveled = 0.0f;
            }
        }
    }
}

void draw_asteroids(Asteroid asteroids[], int number, int radius){
    for (int i = 0; i < number; i++){
        if (asteroids[i].alive){
            DrawCircle(asteroids[i].position.x, asteroids[i].position.y, radius, WHITE);
        }
    }
}

void draw_ship(Ship ship, int radius){
    DrawCircle(ship.position.x, ship.position.y, radius, GREEN);
    
    Vec2 direction = get_direction(ship.angle);
    Vec2 nose_position = {
        .x=ship.position.x + direction.x * radius,
        .y=ship.position.y + direction.y * radius
    };

    Vec2 left_wing_position = {
        .x=ship.position.x - direction.x * radius,
        .y=ship.position.y - direction.y * radius
    };
    // Vec2 right_wing_position = {
    //     .x=ship.position.x + direction.x * radius,
    //     .y=ship.position.y + direction.y * radius
    // };

    DrawLine(ship.position.x, ship.position.y, nose_position.x, nose_position.y, BLACK);

    DrawLine(nose_position.x, nose_position.y, left_wing_position.x, left_wing_position.y, WHITE);
    // DrawLine(nose_position.x, nose_position.y, right_wing_position.x, right_wing_position.y, WHITE);
}

void draw_bullets(Bullet bullets[], int number, int bullet_radius){
    for (int i = 0; i < number; i++){
        if (bullets[i].alive){
            DrawCircle(bullets[i].position.x, bullets[i].position.y, bullet_radius, RED);
        }
    }
}

void initialize_bullets(Bullet bullets[], int length){
    for (int i = 0; i < length; i++){
        bullets[i] = (Bullet){
            .alive = false
        };
    }
}

void fire_bullet(Ship ship, Bullet bullets[], int ship_radius, int bullet_index, int speed_mulitplier){
    Vec2 direction = get_direction(ship.angle);
    Vec2 nose_position = {
        .x=ship.position.x + direction.x * ship_radius,
        .y=ship.position.y + direction.y * ship_radius
    };

    Vec2 bullet_velocity = {
        .x = direction.x * speed_mulitplier,
        .y = direction.y * speed_mulitplier
    };

    bullets[bullet_index].velocity = bullet_velocity;
    bullets[bullet_index].position = nose_position;
    bullets[bullet_index].alive = true;
}


void check_asteroids_and_bullets_collisions(
    Bullet bullets[], 
    Asteroid asteroids[], 
    int bullets_length, 
    int asteroids_length,
    int bulled_radius,
    int asteroid_radius
){
    for (int bullet_index = 0; bullet_index < bullets_length; bullet_index++){
        for (int asteroid_index = 0; asteroid_index < asteroids_length; asteroid_index++){
            if (!asteroids[asteroid_index].alive){
                continue;
            }
            if (!bullets[bullet_index].alive){
                continue;
            }

            bool collision = is_collision(
                bullets[bullet_index].position,
                asteroids[asteroid_index].position,
                bulled_radius,
                asteroid_radius
            );

            if (collision){
                bullets[bullet_index].alive = false;
                bullets[bullet_index].distance_traveled = 0.0f;
                asteroids[asteroid_index].alive = false;
            }
        }
    }
}


int main(void){
    int low = 0;
    int hight = 900;
    int number_of_asteroids = 5;
    int number_of_bullets = 8;
    int bullet_speed_multiplier = 300;

    int current_bullet_index = 0;

    int hitbox_radius = 30;
    int ship_radius = 10;
    int bullet_radius = 3;

    float acceleration = 100.0f;
    float friction = 0.99f;
    float angle_change = 200.0f;

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
        .angle=270
    };

    
    InitWindow(hight, hight, "Asteroids.c");
    SetTargetFPS(60);
    
    Asteroid asteroids[number_of_asteroids];
    initialize_asteroids(asteroids, number_of_asteroids, low, hight);
    
    Bullet bullets[number_of_bullets];
    initialize_bullets(bullets, number_of_bullets);

    while (!WindowShouldClose()){
        float dt = (float)GetFrameTime();
        calculate_next_asteroids_coordinates(asteroids, number_of_asteroids, hitbox_radius, dt, hight);
        calculate_next_bullet_coordinates(bullets, number_of_bullets, bullet_radius, dt, hight);
        calculate_next_ship_coordinates(&ship, ship_radius, dt, hight);

        check_asteroids_and_bullets_collisions(
            bullets, 
            asteroids, 
            number_of_bullets, 
            number_of_asteroids, 
            bullet_radius, 
            hitbox_radius
        );

        if (IsKeyDown(KEY_UP)){
            change_speed(&ship.velocity, ship.angle, acceleration, dt);
        }

        if (IsKeyPressed(KEY_SPACE)){
            fire_bullet(ship, bullets, ship_radius, current_bullet_index, bullet_speed_multiplier);
            current_bullet_index = (current_bullet_index + 1) % number_of_bullets; 
        }

        if (IsKeyDown(KEY_LEFT)){
            ship.angle -= angle_change * dt;
        }
        if (IsKeyDown(KEY_RIGHT)){
            ship.angle += angle_change * dt;
        }

        apply_friction(&ship.velocity, friction, dt);
        
        BeginDrawing();

            ClearBackground(BLACK);
            draw_asteroids(asteroids, number_of_asteroids, hitbox_radius);
            draw_ship(ship, ship_radius);
            draw_bullets(bullets, number_of_bullets, bullet_radius);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
