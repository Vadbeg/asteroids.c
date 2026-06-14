#include <stdio.h>
#include <stdbool.h>


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

void vec2_add_in_palce(Vec2 *a, Vec2 b){
    (*a).x = (*a).x + b.x;
    (*a).y = (*a).y + b.y;
}


int main(void){
    Vec2 a = {5, 5};
    Vec2 b = {7, 7};

    Vec2 c = vec2_add(a, b);

    printf("A = (X=%f, Y=%f)\n", a.x, a.y);
    printf("B = (X=%f, Y=%f)\n", b.x, b.y);
    printf("C = (X=%f, Y=%f)\n", c.x, c.y);

    vec2_add_in_palce(&a, b);

    printf("A = (X=%f, Y=%f)\n", a.x, a.y);

    printf("sizeof(Vec2) = %zu\n", sizeof(Vec2));
    printf("sizeof(Ship) = %zu\n", sizeof(Ship));

    int xs[5] = {0};

    printf("%d\n", xs[1]);

    return 0;
}
