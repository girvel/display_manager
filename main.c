#include <assert.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <security/pam_appl.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#define SIZE(X) sizeof(X)/sizeof(*X)

int mod(int a, int b) {
    int result = a % b;
    if (result < 0) {
        result += (b < 0 ? -b : b);
    }
    return result;
}

// TODO handle all errors
// TODO parse screen size
// TODO blinking cursor
// TODO do not redraw unless something changed?
#define SCREEN_W 3440
#define SCREEN_H 1440

#define LOGIN_LEN 16
#define LOGIN_EXCESS 16

typedef enum {
    FOCUS_LOGIN,
    FOCUS_PASSWORD,
    FOCUS_LEN,
} Focus;

typedef struct {
    const char *login, *password;
} PamConversationPayload;

int pam_conversation_handler(
    int num_msg, const struct pam_message **msg,
    struct pam_response **resp, void *appdata_ptr)
{
    PamConversationPayload *payload = appdata_ptr;

    for (int i = 0; i < num_msg; i++) {
        switch (msg[i]->msg_style) {
        case PAM_PROMPT_ECHO_OFF:
            resp[i] = malloc(sizeof(struct pam_response));
            resp[i]->resp = strdup(payload->password);
            resp[i]->resp_retcode = 0;
            break;
        case PAM_PROMPT_ECHO_ON:
            resp[i] = malloc(sizeof(struct pam_response));
            resp[i]->resp = strdup(payload->login);
            resp[i]->resp_retcode = 0;
            break;
        case PAM_TEXT_INFO:
            printf("PAM message: %s\n", msg[i]->msg);
            break;
        case PAM_ERROR_MSG:
            printf("PAM error: %s\n", msg[i]->msg);
            break;
        }
    }

    return PAM_SUCCESS;
}

typedef enum {
    STATUS_OK,
    STATUS_WRONG_CREDENTIALS,
    STATUS_ERROR,
} AuthStatus;

typedef struct {
    AuthStatus status;
    union {
        char **envlist;
    };
} AuthResult;

AuthResult auth(const char *login, const char *password)
{
    AuthResult result;

    #define EXPECT_OK(EXPR) do { \
            int status_code = (EXPR); \
            if (status_code != PAM_SUCCESS) { \
                printf(#EXPR " returned code %d\n", status_code); \
                result.status = STATUS_ERROR; \
                return result; \
            } \
        } while (0)

    PamConversationPayload *payload = malloc(sizeof(PamConversationPayload));
    payload->login = login;
    payload->password = password;

    struct pam_conv conv;
    conv.conv = pam_conversation_handler;
    conv.appdata_ptr = payload;

    pam_handle_t *pam_handle;
    EXPECT_OK(pam_start("login", login, &conv, &pam_handle));
    pam_set_item(pam_handle, PAM_TTY, "/dev/tty2");  // TODO hardcoded
    pam_putenv(pam_handle, "XDG_VTNR=2");

    int status_code = pam_authenticate(pam_handle, 0);
    if (status_code == PAM_AUTH_ERR) {
        result.status = STATUS_WRONG_CREDENTIALS;
        return result;
    } else if (status_code != PAM_SUCCESS) {
        printf("pam_authenticate(pam_handle, 0) returned code %d\n", status_code);
        result.status = STATUS_ERROR;
        return result;
    }

    EXPECT_OK(pam_acct_mgmt(pam_handle, 0));
    EXPECT_OK(pam_setcred(pam_handle, 0));
    EXPECT_OK(pam_open_session(pam_handle, 0));

    // TODO PAM cleanup

    result.status = STATUS_OK;
    result.envlist = pam_getenvlist(pam_handle);
    for (size_t i = 0; result.envlist[i] != NULL; i++) {
        printf("env: %s\n", result.envlist[i]);
    }
    return result;

    #undef EXPECT_OK
}

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

    char **pam_envlist;

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
        bool will_auth = false;
        if (IsKeyPressed(KEY_ENTER)) {
            switch (focus) {
            case FOCUS_LOGIN:
                focus_delta = 1;
                break;
            case FOCUS_PASSWORD:
                will_auth = true;
                break;
            case FOCUS_LEN: assert(false);
            }
        }
        if (IsKeyPressed(KEY_TAB)) focus_delta = 1;  // TODO does not work, investigate
        if (IsKeyPressed(KEY_DOWN)) focus_delta = 1;
        if (IsKeyPressed(KEY_UP)) focus_delta = -1;

        int key;
        while ((key = GetKeyPressed())) {
            if (key == '\t') {
                focus_delta = 1;
                continue;
            }
            if (key >= 256) continue;
            if ('A' <= key && key <= 'Z') key = key - 'A' + 'a';

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

        if (focus_delta != 0) {
            focus = mod(focus + focus_delta, FOCUS_LEN);
        }

        if (will_auth) {
            AuthResult auth_result = auth(login, password);
            switch (auth_result.status) {
            case STATUS_OK:
                pam_envlist = auth_result.envlist;
                goto login;
            case STATUS_ERROR:
                break;
            case STATUS_WRONG_CREDENTIALS:
                break;
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
login:
    CloseWindow();

    struct passwd *passwd_data = getpwnam(login);
    assert(initgroups(login, passwd_data->pw_gid) == 0);  // TODO errno & such
    assert(setgid(passwd_data->pw_gid) == 0);
    assert(setuid(passwd_data->pw_uid) == 0);
    assert(chdir(passwd_data->pw_dir) == 0);

    char *const argv[] = {passwd_data->pw_shell, "-l", "-c", "dbus-run-session niri", NULL};

    int pam_envlist_len;
    for (pam_envlist_len = 0; pam_envlist[pam_envlist_len] != NULL; pam_envlist_len++);
    const int custom_vars_len = 8;
    char **envp = malloc((pam_envlist_len + custom_vars_len + 1) * sizeof(const char *));
    asprintf(&envp[0], "USER=%s", login);
    asprintf(&envp[1], "LOGNAME=%s", login);
    asprintf(&envp[2], "SHELL=%s", passwd_data->pw_shell);
    asprintf(&envp[3], "HOME=%s", passwd_data->pw_dir);
    envp[4] = "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin";
    envp[5] = "XDG_SESSION_TYPE=wayland";  // TODO probably excessive
    envp[6] = "XDG_SESSION_CLASS=user";
    asprintf(&envp[7], "XDG_RUNTIME_DIR=/run/user/%d", passwd_data->pw_uid);
    for (int i = 0; i < pam_envlist_len; i++) {
        envp[custom_vars_len + i] = pam_envlist[i];
    }
    envp[pam_envlist_len + 5] = NULL;
    printf("Ready to go!\n");
    execve(argv[0], argv, envp);

    return 0;
}
