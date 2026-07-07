#include <raylib.h>

// TODO parse screen size
#define SCREEN_W 3440
#define SCREEN_H 1440

int main(void)
{
    InitWindow(SCREEN_W, SCREEN_H, "Display Manager");
    const int font_size = 48;
    Font jbmono = LoadFontEx("assets/jbmono.ttf", font_size, 0, 0);
    SetTextureFilter(jbmono.texture, TEXTURE_FILTER_BILINEAR);

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);

            {
                const char *line_login = "login: girvel";
                const int line_login_w = MeasureTextEx(jbmono, line_login, font_size, 1).x;
                Vector2 pos = {(SCREEN_W - line_login_w) / 2., SCREEN_H / 2. - font_size};
                DrawTextEx(jbmono, line_login, pos, font_size, 1, RAYWHITE);
            }

            {
                const char *line_password = "password: ******";
                const int line_password_w = MeasureTextEx(jbmono, line_password, font_size, 1).x;
                Vector2 pos = {(SCREEN_W - line_password_w) / 2., SCREEN_H / 2.};
                DrawTextEx(jbmono, line_password, pos, font_size, 1, RAYWHITE);
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
