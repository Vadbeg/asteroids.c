#include <raylib.h>

int main(void){
    InitWindow(800, 450, "raylib game flow testing");
    SetTargetFPS(60);
    int i = 0;

    while (!WindowShouldClose()){
        if (i < 20){
            i++;
        }
        
        BeginDrawing();

            ClearBackground(RAYWHITE);
            DrawText("Hey mate!", 190 + i * 10, 200, 20, LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
