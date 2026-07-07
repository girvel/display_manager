#include <raylib.h>

int main(void)
{
    InitWindow(0, 0, "Boot screen");

    while (!WindowShouldClose()) {
        BeginDrawing();
            DrawText("Hello, world!", 100, 100, 10, RAYWHITE);
            ClearBackground(BLACK);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
