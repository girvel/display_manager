#include <assert.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define SIZE(X) sizeof(X)/sizeof(*X)

int mod(int a, int b) {
    int result = a % b;
    if (result < 0) {
        result += (b < 0 ? -b : b);
    }
    return result;
}

// TODO parse screen size
#define SCREEN_W 3440
#define SCREEN_H 1440

#define LOGIN_LEN 16
#define LOGIN_EXCESS 16

typedef enum {
    FOCUS_LOGIN,
    FOCUS_PASSWORD,
    FOCUS_LEN,
} Focus;

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

    Focus focus = FOCUS_LOGIN;

    char login[16] = "";
    size_t login_len = 0;

    char password[16] = "";
    char password_obscured[16] = "";
    size_t password_len = 0;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
            switch (focus) {
            case FOCUS_LOGIN:
                if (login_len > 0) {
                    login_len--;
                    login[login_len] = '\0';
                }
                break;
            case FOCUS_PASSWORD:
                if (password_len > 0) {
                    password_len--;
                    password[password_len] = '\0';
                    password_obscured[password_len] = '\0';
                }
                break;
            case FOCUS_LEN: assert(false);
            }
        }

        int focus_delta = 0;
        if (IsKeyPressed(KEY_ENTER)) {
            switch (focus) {
            case FOCUS_LOGIN:
                focus_delta = 1;
            case FOCUS_PASSWORD:
                goto exit;
            case FOCUS_LEN: assert(false);
            }
        }
        if (IsKeyPressed(KEY_TAB)) focus_delta = 1;
        if (IsKeyPressed(KEY_DOWN)) focus_delta = 1;
        if (IsKeyPressed(KEY_UP)) focus_delta = -1;

        if (focus_delta != 0) {
            focus = mod(focus + focus_delta, FOCUS_LEN);
        }

        int key;
        while ((key = GetKeyPressed())) {
            if (key >= 256) continue;

            switch (focus) {
            case FOCUS_LOGIN:
                if (login_len < SIZE(login) - 1) {
                    login[login_len] = key;
                    login_len++;
                    login[login_len] = '\0';
                }
                break;
            case FOCUS_PASSWORD:
                if (password_len < SIZE(password) - 1) {
                    password[password_len] = key;
                    password_obscured[password_len] = '*';
                    password_len++;
                    password[password_len] = '\0';
                    password_obscured[password_len] = '\0';
                }
                break;
            case FOCUS_LEN: assert(false);
            }
        }

        BeginDrawing();
            ClearBackground(BLACK);

            DrawRectangleLinesEx(login_box, 2, RAYWHITE);

            {
                const char *login_prompt = "login: ";
                Vector2 pos = {login_box.x + login_box_padding, login_box.y + login_box_padding};
                DrawTextEx(jbmono, login_prompt, pos, font_size, 1, RAYWHITE);
                pos.x += MeasureTextEx(jbmono, login_prompt, font_size, 1).x;
                DrawTextEx(jbmono, login, pos, font_size, 1, RAYWHITE);
                if (focus == FOCUS_LOGIN) {
                    pos.x += MeasureTextEx(jbmono, login, font_size, 1).x;
                    DrawTextEx(jbmono, "_", pos, font_size, 1, RAYWHITE);
                }
            }

            {
                const char *password_prompt = "password: ";
                Vector2 pos = {
                    login_box.x + login_box_padding,
                    login_box.y + login_box_padding + font_size
                };
                DrawTextEx(jbmono, password_prompt, pos, font_size, 1, RAYWHITE);
                pos.x += MeasureTextEx(jbmono, password_prompt, font_size, 1).x;
                DrawTextEx(jbmono, password_obscured, pos, font_size, 1, RAYWHITE);
                if (focus == FOCUS_PASSWORD) {
                    pos.x += MeasureTextEx(jbmono, password_obscured, font_size, 1).x;
                    DrawTextEx(jbmono, "_", pos, font_size, 1, RAYWHITE);
                }
            }
        EndDrawing();
    }
exit:
    CloseWindow();
    return 0;
}
