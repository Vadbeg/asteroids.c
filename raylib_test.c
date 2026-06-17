#include <raylib.h>

int main(void){
    InitWindow(800, 450, "raylib game flow testing");

    while (!WindowShouldClose()){
        BeginDrawing();

        ClearBackground(RAYWHITE);
        DrawText("Hey mate!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
