#include <windows.h>
#include <GL/gl.h>

static HDC   g_dc;
static HGLRC g_gl;

const float WIN_W = 360.0f;
const float GRASS_H = 60.0f;
const float PLAYER_SIZE = 12.0f;
const float JUMP_MAX = 42.0f;

float player_x = (360.0f - 12.0f) / 2.0f;
float player_y = GRASS_H;
float jump_h = 0.0f;
float player_vy = 0.0f;

const float goal_x = 250.0f;
const float goal_y = GRASS_H;

int won = 0;

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_CLOSE) PostQuitMessage(0);
    if (m == WM_KEYDOWN && w == VK_ESCAPE) PostQuitMessage(0);
        if (m == WM_KEYDOWN && w == 'R') {
        player_x = (WIN_W - PLAYER_SIZE) / 2.0f;
        player_y = GRASS_H;
        player_vy = 0.0f;
        won = 0;
    }
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

    if (!won) {
        if (GetAsyncKeyState(VK_LEFT)  & 0x8000) player_x -= 5.0f;
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) player_x += 5.0f;

        if (player_x < 0) player_x = 0;
        if (player_x > WIN_W - PLAYER_SIZE) player_x = WIN_W - PLAYER_SIZE;

        if (player_y <= GRASS_H && (GetAsyncKeyState(VK_UP) & 0x8000)) {
    player_vy = 10.0f;
}
player_vy -= 0.9f;
player_y += player_vy;
if (player_y < GRASS_H) {
    player_y = GRASS_H;
    player_vy = 0.0f;
}
        if (player_x + PLAYER_SIZE > goal_x && player_x < goal_x + PLAYER_SIZE &&
            player_y + PLAYER_SIZE > goal_y && player_y < goal_y + PLAYER_SIZE) {
            won = 1;
        }
    }

    if (won) {
        glClearColor(1.0f, 0.8f, 0.0f, 1.0f);
    } else {
        glClearColor(0.4f, 0.7f, 1.0f, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT);

    fly_draw_rect(0, 0, WIN_W, GRASS_H, 0.2f, 0.7f, 0.2f);

    if (!won) {
        fly_draw_rect(goal_x, goal_y, PLAYER_SIZE, PLAYER_SIZE, 1.0f, 0.0f, 0.0f);
    }

    fly_draw_rect(player_x, player_y, PLAYER_SIZE, PLAYER_SIZE, 1.0f, 0.5f, 0.0f);

    SwapBuffers(g_dc);
    return 1;
}

int main(void) {
    fly_win_open();
    timeBeginPeriod(1);
    while (fly_win_step()) {
        Sleep(33);
    }
    return 0;
}