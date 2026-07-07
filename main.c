#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// TODO parse screen size
#define SCREEN_W 3440
#define SCREEN_H 1440

#define LOGIN_LEN 16
#define LOGIN_EXCESS 16

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

    char login_str[LOGIN_LEN + LOGIN_EXCESS];
    strcpy(login_str, "login: ");
    char *login_buffer = login_str + 7;
    size_t login_len = 0;

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);

            DrawRectangleLinesEx(login_box, 2, RAYWHITE);

            int key;
            while ((key = GetKeyPressed())) {
                if (key < 256 && login_len < LOGIN_LEN) {
                    printf("key: %c\n", key);
                    login_buffer[login_len] = key;
                    login_len++;
                    login_buffer[login_len] = '\0';
                }
            }

            if (login_len > 0 && (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE))) {
                login_len--;
                login_buffer[login_len] = '\0';
            }

            {
                Vector2 pos = {login_box.x + login_box_padding, login_box.y + login_box_padding};
                DrawTextEx(jbmono, login_str, pos, font_size, 1, RAYWHITE);
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
