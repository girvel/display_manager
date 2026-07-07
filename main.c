#include <raylib.h>

// TODO parse screen size
#define SCREEN_W 3440
#define SCREEN_H 1440

int main(void)
{
    InitWindow(SCREEN_W, SCREEN_H, "Display Manager");
    const int font_size = 32;
    Font jbmono = LoadFontEx("assets/jbmono.ttf", font_size * 2, 0, 0);
    float char_w = MeasureTextEx(jbmono, "w", font_size, 1).x;
    SetTextureFilter(jbmono.texture, TEXTURE_FILTER_BILINEAR);

    const int login_box_padding = 20;
    const int login_box_h = login_box_padding * 2 + font_size * 2;
    const int login_box_w = login_box_padding * 2 + char_w * 32;
    Rectangle login_box = {
        (SCREEN_W - login_box_w) / 2., (SCREEN_H - login_box_h) / 2.,
        login_box_w, login_box_h
    };

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);

            DrawRectangleLinesEx(login_box, 2, RAYWHITE);

            {
                const char *line_login = "login: girvel";
                Vector2 pos = {login_box.x + login_box_padding, login_box.y + login_box_padding};
                DrawTextEx(jbmono, line_login, pos, font_size, 1, RAYWHITE);
            }

            {
                const char *line_password = "password: ******";
                Vector2 pos = {
                    login_box.x + login_box_padding,
                    login_box.y + login_box_padding + font_size
                };
                DrawTextEx(jbmono, line_password, pos, font_size, 1, RAYWHITE);
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
