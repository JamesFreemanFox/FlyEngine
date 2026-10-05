#include <windows.h>
#include <GL/gl.h>

static HDC   g_dc;
static HGLRC g_gl;

const float WIN_W = 360.0f;
const float GRASS_H = 60.0f;
const float BOX = 12.0f;

float player_x = 174.0f;
float player_y = 60.0f;
float player_vy = 0.0f;
int   player_dir = 1;

int   won = 0;
int   dead = 0;
int   shoot_cd = 0;

#define MAX_BULLETS 20
float bullet_x[MAX_BULLETS];
float bullet_y[MAX_BULLETS];
float bullet_vx[MAX_BULLETS];
int   bullet_alive[MAX_BULLETS];

#define MAX_ENEMIES 4
float enemy_x[MAX_ENEMIES];
float enemy_y[MAX_ENEMIES];
float enemy_dir[MAX_ENEMIES];
int   enemy_alive[MAX_ENEMIES];

const float goal_x = 320.0f;
const float goal_y = 60.0f;

void init_game(void) {
    player_x = 174.0f;
    player_y = 60.0f;
    player_vy = 0.0f;
    player_dir = 1;
    won = 0;
    dead = 0;
    shoot_cd = 0;

    for (int i = 0; i < MAX_BULLETS; i++) {
        bullet_x[i] = -100.0f;
        bullet_y[i] = -100.0f;
        bullet_vx[i] = 0.0f;
        bullet_alive[i] = 0;
    }

    enemy_x[0] = 60.0f;  enemy_y[0] = 60.0f;  enemy_dir[0] = 1;
    enemy_x[1] = 120.0f; enemy_y[1] = 60.0f;  enemy_dir[1] = -1;
    enemy_x[2] = 200.0f; enemy_y[2] = 60.0f;  enemy_dir[2] = 1;
    enemy_x[3] = 290.0f; enemy_y[3] = 60.0f;  enemy_dir[3] = -1;
    for (int i = 0; i < MAX_ENEMIES; i++) enemy_alive[i] = 1;
}

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_CLOSE) PostQuitMessage(0);
    if (m == WM_KEYDOWN && w == VK_ESCAPE) PostQuitMessage(0);
    if (m == WM_KEYDOWN && w == 'R') init_game();
    return DefWindowProc(h, m, w, l);
}

void fly_win_open(void) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(0);
    wc.lpszClassName = "Fly";
    RegisterClass(&wc);

    HWND h = CreateWindow("Fly", "Fly Engine", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 360, 360, 0, 0, wc.hInstance, 0);
    g_dc = GetDC(h);

    PIXELFORMATDESCRIPTOR p = {0};
    p.nSize = sizeof(p);
    p.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    p.cColorBits = 16;
    SetPixelFormat(g_dc, ChoosePixelFormat(g_dc, &p), &p);

    g_gl = wglCreateContext(g_dc);
    wglMakeCurrent(g_dc, g_gl);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 360, 0, 360, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void fly_draw_rect(float x, float y, float w, float h, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

int fly_win_step(void) {
    MSG m;
    while (PeekMessage(&m, 0, 0, 0, PM_REMOVE)) {
        if (m.message == WM_QUIT) return 0;
        TranslateMessage(&m);
        DispatchMessage(&m);
    }

    if (!won && !dead) {
        if (GetAsyncKeyState(VK_LEFT)  & 0x8000) { player_x -= 5.0f; player_dir = -1; }
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) { player_x += 5.0f; player_dir = 1; }

        if (player_x < 0) player_x = 0;
        if (player_x > WIN_W - BOX) player_x = WIN_W - BOX;

        if (player_y <= GRASS_H && (GetAsyncKeyState(VK_UP) & 0x8000)) {
            player_vy = 4.2f;
        }
        player_vy -= 0.35f;
        player_y += player_vy;
        if (player_y < GRASS_H) {
            player_y = GRASS_H;
            player_vy = 0.0f;
        }

        if (shoot_cd > 0) shoot_cd--;
        if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && shoot_cd == 0) {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!bullet_alive[i]) {
                    bullet_alive[i] = 1;
                    bullet_x[i] = (player_dir > 0) ? (player_x + BOX) : (player_x - 4.0f);
                    bullet_y[i] = player_y + BOX / 2.0f - 2.0f;
                    bullet_vx[i] = player_dir * 8.0f;
                    break;
                }
            }
            shoot_cd = 6;
        }

        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullet_alive[i]) continue;
            bullet_x[i] += bullet_vx[i];
            if (bullet_x[i] < -10.0f || bullet_x[i] > WIN_W + 10.0f) {
                bullet_alive[i] = 0;
                continue;
            }
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (!enemy_alive[j]) continue;
                if (bullet_x[i] + 4.0f > enemy_x[j] && bullet_x[i] < enemy_x[j] + BOX &&
                    bullet_y[i] + 4.0f > enemy_y[j] && bullet_y[i] < enemy_y[j] + BOX) {
                    bullet_alive[i] = 0;
                    enemy_alive[j] = 0;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemy_alive[i]) continue;
            enemy_x[i] += enemy_dir[i] * 1.0f;
            if (enemy_x[i] < 0) { enemy_x[i] = 0; enemy_dir[i] = 1; }
            if (enemy_x[i] > WIN_W - BOX) { enemy_x[i] = WIN_W - BOX; enemy_dir[i] = -1; }

            if (player_x + BOX > enemy_x[i] && player_x < enemy_x[i] + BOX &&
                player_y + BOX > enemy_y[i] && player_y < enemy_y[i] + BOX) {
                dead = 1;
            }
        }

        int alive_count = 0;
        for (int i = 0; i < MAX_ENEMIES; i++) if (enemy_alive[i]) alive_count++;

        if (alive_count == 0) {
            if (player_x + BOX > goal_x && player_x < goal_x + BOX &&
                player_y + BOX > goal_y && player_y < goal_y + BOX) {
                won = 1;
            }
        }
    }

    if (dead) {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    } else if (won) {
        glClearColor(1.0f, 0.8f, 0.0f, 1.0f);
    } else {
        glClearColor(0.4f, 0.7f, 1.0f, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT);

    fly_draw_rect(0, 0, WIN_W, GRASS_H, 0.2f, 0.7f, 0.2f);

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemy_alive[i]) {
            fly_draw_rect(enemy_x[i], enemy_y[i], BOX, BOX, 0.0f, 0.0f, 0.0f);
        }
    }

    int alive_count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) if (enemy_alive[i]) alive_count++;

    if (alive_count == 0 && !won && !dead) {
        fly_draw_rect(goal_x, goal_y, BOX, BOX, 1.0f, 0.5f, 0.0f);
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullet_alive[i]) {
            fly_draw_rect(bullet_x[i], bullet_y[i], 4.0f, 4.0f, 1.0f, 1.0f, 0.0f);
        }
    }

    if (!dead) {
        fly_draw_rect(player_x, player_y, BOX, BOX, 0.2f, 0.6f, 1.0f);
    }

    SwapBuffers(g_dc);
    return 1;
}

int main(void) {
    init_game();
    fly_win_open();
    while (fly_win_step()) {
        Sleep(33);
    }
    return 0;
}