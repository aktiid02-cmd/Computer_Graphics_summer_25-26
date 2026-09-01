/*
 * Chattogram City Scenario - 2D Computer Graphics Simulation
 * Built using C++, OpenGL & GLUT
 *
 * Scenario Components:
 *  - Architecture & Skyline (Main building + 5 backdrop skyscrapers)
 *  - River & Moving Boat (Foreground river with animated water ripples & boat)
 *  - Thin Two-Lane Road with 2 moving vehicles (Bus & Car in opposite directions)
 *  - 4 Roadside Trees & 2 Animated Flying Birds
 *  - Dynamic Lighting & Modes (Day, Evening Red Sunset, Night, Winter, Rain)
 *
 * Controls:
 *  - 'd' / 'D' : Day Mode
 *  - 'e' / 'E' : Evening Mode (Red Sky Sunset Vibe)
 *  - 'n' / 'N' : Night Mode (Moon, stars, glowing streetlamps & windows)
 *  - 'f' / 'F' : Fast Moving Mode (Speed boost toggle)
 *  - 'w' / 'W' : Winter Season (Falling snow, snow capped roofs & trees)
 *  - 'r'       : Rain Mode (Realistic rain animation & audio)
 *  - 'h' / 'H' : Honk Car Horn (Dual-tone car horn sound)
 *  - 'R'       : Reset to Summer Day default state
 *  - SPACE BAR : Pause / Resume all movement
 *  - Up / Down Arrow Keys   : Adjust vehicle speed
 *  - Left Mouse Button  : Cycle Day -> Evening -> Night
 *  - Right Mouse Button : Honk car horn
 */

#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

// Global State Constants & Enums
enum Mode { DAY_MODE, EVENING_MODE, NIGHT_MODE };
enum Season { SUMMER_SEASON, WINTER_SEASON };

Mode currentMode = DAY_MODE;
Season currentSeason = SUMMER_SEASON;
bool isRaining = false;
bool isPaused = false;
bool isFastSpeed = false;

// Object Movement & Animation Variables
float busX = -1.2f;       // Bus position (moving Left to Right on road)
float carX = 1.2f;        // Car position (moving Right to Left on road)
float boatX = -0.9f;      // Boat position (moving Left to Right on river)
float cloud1X = -0.6f;    // Cloud 1 position
float cloud2X = 0.5f;     // Cloud 2 position
float bird1X = -0.7f;     // Bird 1 position
float bird2X = 0.2f;      // Bird 2 position
float wingFlapAngle = 0.0f;

float wheelAngle = 0.0f;  // Rotating wheel angle
float vehicleSpeed = 0.008f;
float boatSpeed = 0.0035f;
float cloudSpeed = 0.002f;
float birdSpeed = 0.003f;
float sunAngle = 0.0f;    // Sun ray rotation

// Particle Systems (Rain & Snow)
const int MAX_RAIN = 220;
float rainX[MAX_RAIN];
float rainY[MAX_RAIN];
float rainSpeed[MAX_RAIN];

const int MAX_SNOW = 160;
float snowX[MAX_SNOW];
float snowY[MAX_SNOW];
float snowSpeed[MAX_SNOW];

const int MAX_STARS = 70;
float starX[MAX_STARS];
float starY[MAX_STARS];

// Win32 & Cross-Platform OpenGL Function Declarations
#ifdef _WIN32
#include <windows.h>
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) { return DefWindowProc(hWnd, message, wParam, lParam); }
void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC) { glEnable(GL_LIGHTING); }
void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC) { glDisable(GL_LIGHTING); }
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) { return main(__argc, __argv); }
#else
void EnableOpenGL() { glEnable(GL_LIGHTING); }
void DisableOpenGL() { glDisable(GL_LIGHTING); }
#endif

// Function Declarations
void init();
void initGL();
void display();
void display1();
void display1_view();
void display2();
void display3();
void display4();
void display5();
void display_up();
void display_down();
void day();
void evening();
void night();
void update(int value);
void update1();
void update2();
void update_day();
void update_night();
void dis();       // Draw Bus
void disback();   // Draw Car
void drawBoat(float bx, float by);
void drawBird(float bx, float by, float flap);
void handleMouse(int button, int state, int x, int y);
void handleKeypress(unsigned char key, int x, int y);
void SpecialInput(int key, int x, int y);
void Idle();
void sound(int type = 0);
void playCarHorn();

// Helper Function: Draw Solid Circle / Polygon
void drawCircle(float cx, float cy, float r, int num_segments, float rCol, float gCol, float bCol) {
    glColor3f(rCol, gCol, bCol);
    glBegin(GL_POLYGON);
    for (int i = 0; i < num_segments; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(cx + x, cy + y);
    }
    glEnd();
}

// Helper Function: Render Text String on Screen
void renderText(float x, float y, const char* string, float r, float g, float b) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    while (*string) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *string);
        string++;
    }
}

void renderBoldText(float x, float y, const char* string, float r, float g, float b) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    while (*string) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *string);
        string++;
    }
}

// Audio Trigger Functions
void sound(int type) {
    if (type == 1 || isRaining) {
        std::cout << "[Sound Callback] Playing realistic rain & thunder audio..." << std::endl;
        system("afplay rain.wav &");
    } else {
        playCarHorn();
    }
}

void playCarHorn() {
    std::cout << "[Sound Callback] Honking Car Horn (Dual-Tone)..." << std::endl;
    system("afplay horn.wav &");
}

// Initialization of OpenGL Environment and Particle Data
void init() {
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    srand(time(NULL));
    for (int i = 0; i < MAX_RAIN; i++) {
        rainX[i] = ((float)rand() / RAND_MAX) * 2.4f - 1.2f;
        rainY[i] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        rainSpeed[i] = 0.022f + ((float)rand() / RAND_MAX) * 0.02f;
    }

    for (int i = 0; i < MAX_SNOW; i++) {
        snowX[i] = ((float)rand() / RAND_MAX) * 2.4f - 1.2f;
        snowY[i] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        snowSpeed[i] = 0.005f + ((float)rand() / RAND_MAX) * 0.005f;
    }

    for (int i = 0; i < MAX_STARS; i++) {
        starX[i] = ((float)rand() / RAND_MAX) * 2.4f - 1.2f;
        starY[i] = 0.2f + ((float)rand() / RAND_MAX) * 0.8f;
    }
}

void initGL() {
    GLfloat global_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
    glDisable(GL_LIGHTING);
}

// Day Mode Setup
void day() {
    glBegin(GL_QUADS);
    glColor3f(0.35f, 0.65f, 0.95f);
    glVertex2f(-1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    if (currentSeason == WINTER_SEASON) {
        glColor3f(0.7f, 0.8f, 0.88f);
    } else {
        glColor3f(0.75f, 0.9f, 1.0f);
    }
    glVertex2f(1.0f, 0.1f);
    glVertex2f(-1.0f, 0.1f);
    glEnd();

    // Sun with Rotating Rays
    glPushMatrix();
    glTranslatef(0.75f, 0.75f, 0.0f);
    drawCircle(0.0f, 0.0f, 0.11f, 30, 1.0f, 0.9f, 0.1f);

    glPushMatrix();
    glRotatef(sunAngle, 0.0f, 0.0f, 1.0f);
    glColor3f(1.0f, 0.85f, 0.2f);
    for (int i = 0; i < 12; i++) {
        glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(0.0f, 0.12f);
        glVertex2f(-0.02f, 0.18f);
        glVertex2f(0.02f, 0.18f);
        glEnd();
    }
    glPopMatrix();
    glPopMatrix();
}

// Evening Mode Setup (Vibrant Red Sky Sunset Vibe)
void evening() {
    glBegin(GL_QUADS);
    glColor3f(0.75f, 0.18f, 0.05f); // Deep Crimson Top
    glVertex2f(-1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glColor3f(1.0f, 0.52f, 0.08f);  // Golden Orange Horizon
    glVertex2f(1.0f, 0.1f);
    glVertex2f(-1.0f, 0.1f);
    glEnd();

    drawCircle(-0.55f, 0.25f, 0.14f, 30, 1.0f, 0.35f, 0.05f);
    drawCircle(-0.55f, 0.25f, 0.10f, 30, 1.0f, 0.85f, 0.25f);
}

// Night Mode Setup
void night() {
    glBegin(GL_QUADS);
    glColor3f(0.04f, 0.06f, 0.15f);
    glVertex2f(-1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glColor3f(0.08f, 0.12f, 0.25f);
    glVertex2f(1.0f, 0.1f);
    glVertex2f(-1.0f, 0.1f);
    glEnd();

    glPointSize(2.0f);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 1.0f, 0.9f);
    for (int i = 0; i < MAX_STARS; i++) {
        glVertex2f(starX[i], starY[i]);
    }
    glEnd();

    drawCircle(-0.75f, 0.78f, 0.09f, 30, 0.95f, 0.95f, 0.85f);
    drawCircle(-0.72f, 0.80f, 0.08f, 30, 0.04f, 0.06f, 0.15f);
}

// Helper Function: Render Animated Flying Bird
void drawBird(float bx, float by, float flap) {
    glLineWidth(2.5f);
    if (currentMode == EVENING_MODE) glColor3f(0.18f, 0.05f, 0.05f);
    else if (currentMode == NIGHT_MODE) glColor3f(0.05f, 0.05f, 0.1f);
    else glColor3f(0.1f, 0.1f, 0.15f);

    float wingY = by + sinf(flap) * 0.025f;

    glBegin(GL_LINES);
    glVertex2f(bx, by);
    glVertex2f(bx - 0.035f, wingY);

    glVertex2f(bx, by);
    glVertex2f(bx + 0.035f, wingY);
    glEnd();
}

// Upper Scenery: Skyline (5 Backside Buildings), Sky, Clouds, 2 Birds
void display_up() {
    if (currentMode == DAY_MODE) {
        day();
    } else if (currentMode == EVENING_MODE) {
        evening();
    } else {
        night();
    }

    // 5 Backside Buildings
    struct BackBuilding {
        float x1, x2, h;
    } buildings[5] = {
        {-0.98f, -0.75f, 0.58f},
        {-0.74f, -0.55f, 0.42f},
        {-0.22f,  0.22f, 0.65f},
        { 0.55f,  0.74f, 0.45f},
        { 0.75f,  0.98f, 0.62f}
    };

    for (int i = 0; i < 5; i++) {
        float x1 = buildings[i].x1;
        float x2 = buildings[i].x2;
        float h = buildings[i].h;

        glBegin(GL_QUADS);
        if (currentMode == EVENING_MODE) glColor3f(0.35f, 0.15f, 0.15f);
        else if (currentMode == NIGHT_MODE) glColor3f(0.08f, 0.10f, 0.18f);
        else if (currentSeason == WINTER_SEASON) glColor3f(0.52f, 0.56f, 0.62f);
        else glColor3f(0.6f, 0.64f, 0.7f);

        glVertex2f(x1, -0.2f);
        glVertex2f(x2, -0.2f);
        glVertex2f(x2, h);
        glVertex2f(x1, h);
        glEnd();

        for (float bx = x1 + 0.03f; bx <= x2 - 0.03f; bx += 0.07f) {
            for (float by = 0.05f; by <= h - 0.06f; by += 0.07f) {
                glBegin(GL_QUADS);
                if (currentMode == NIGHT_MODE || currentMode == EVENING_MODE) glColor3f(1.0f, 0.88f, 0.35f);
                else glColor3f(0.35f, 0.48f, 0.62f);
                glVertex2f(bx, by);
                glVertex2f(bx + 0.035f, by);
                glVertex2f(bx + 0.035f, by + 0.038f);
                glVertex2f(bx, by + 0.038f);
                glEnd();
            }
        }

        if (currentSeason == WINTER_SEASON) {
            glBegin(GL_QUADS);
            glColor3f(0.95f, 0.98f, 1.0f);
            glVertex2f(x1 - 0.01f, h);
            glVertex2f(x2 + 0.01f, h);
            glVertex2f(x2 + 0.01f, h + 0.03f);
            glVertex2f(x1 - 0.01f, h + 0.03f);
            glEnd();
        }
    }

    // Moving Clouds
    glPushMatrix();
    glTranslatef(cloud1X, 0.72f, 0.0f);
    float cCol = (currentMode == DAY_MODE) ? 0.98f : (currentMode == EVENING_MODE ? 0.92f : 0.4f);
    float cRed = (currentMode == EVENING_MODE) ? 1.0f : cCol;
    float cGrn = (currentMode == EVENING_MODE) ? 0.7f : cCol;
    float cBlu = (currentMode == EVENING_MODE) ? 0.6f : cCol;

    drawCircle(-0.1f, 0.0f, 0.06f, 20, cRed, cGrn, cBlu);
    drawCircle(0.0f, 0.03f, 0.08f, 20, cRed, cGrn, cBlu);
    drawCircle(0.1f, 0.0f, 0.06f, 20, cRed, cGrn, cBlu);
    drawCircle(0.04f, -0.02f, 0.06f, 20, cRed, cGrn, cBlu);
    drawCircle(-0.05f, -0.02f, 0.06f, 20, cRed, cGrn, cBlu);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(cloud2X, 0.82f, 0.0f);
    drawCircle(-0.12f, 0.0f, 0.07f, 20, cRed, cGrn, cBlu);
    drawCircle(0.0f, 0.04f, 0.09f, 20, cRed, cGrn, cBlu);
    drawCircle(0.12f, 0.0f, 0.07f, 20, cRed, cGrn, cBlu);
    drawCircle(0.05f, -0.02f, 0.07f, 20, cRed, cGrn, cBlu);
    drawCircle(-0.06f, -0.02f, 0.07f, 20, cRed, cGrn, cBlu);
    glPopMatrix();

    // 2 Animated Flying Birds
    drawBird(bird1X, 0.78f, wingFlapAngle);
    drawBird(bird2X, 0.68f, wingFlapAngle + 1.2f);
}

// Architectural Component 1: Main Building Base Facade
void display1() {
    glBegin(GL_QUADS);
    if (currentMode == NIGHT_MODE) glColor3f(0.15f, 0.18f, 0.25f);
    else if (currentMode == EVENING_MODE) glColor3f(0.85f, 0.70f, 0.65f);
    else glColor3f(0.92f, 0.90f, 0.85f);

    glVertex2f(-0.55f, -0.2f);
    glVertex2f(0.55f, -0.2f);
    glVertex2f(0.55f, 0.35f);
    glVertex2f(-0.55f, 0.35f);

    if (currentMode == NIGHT_MODE) glColor3f(0.12f, 0.15f, 0.22f);
    else if (currentMode == EVENING_MODE) glColor3f(0.78f, 0.62f, 0.58f);
    else glColor3f(0.85f, 0.83f, 0.78f);

    glVertex2f(-0.82f, -0.2f);
    glVertex2f(-0.55f, -0.2f);
    glVertex2f(-0.55f, 0.25f);
    glVertex2f(-0.82f, 0.25f);

    glVertex2f(0.55f, -0.2f);
    glVertex2f(0.82f, -0.2f);
    glVertex2f(0.82f, 0.25f);
    glVertex2f(0.55f, 0.25f);
    glEnd();

    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glColor3f(0.65f, 0.60f, 0.52f);
    glVertex2f(-0.82f, 0.25f); glVertex2f(0.82f, 0.25f);
    glVertex2f(-0.55f, 0.35f); glVertex2f(0.55f, 0.35f);
    glEnd();
}

// Architectural Component 2: Central Glass Tower Facade
void display2() {
    glBegin(GL_QUADS);
    if (currentMode == NIGHT_MODE) glColor3f(0.1f, 0.22f, 0.4f);
    else if (currentMode == EVENING_MODE) glColor3f(0.25f, 0.35f, 0.55f);
    else glColor3f(0.2f, 0.45f, 0.7f);

    glVertex2f(-0.25f, -0.2f);
    glVertex2f(0.25f, -0.2f);
    glVertex2f(0.25f, 0.45f);
    glVertex2f(-0.25f, 0.45f);
    glEnd();

    glLineWidth(1.5f);
    glColor3f(0.8f, 0.9f, 1.0f);
    glBegin(GL_LINES);
    for (float x = -0.20f; x <= 0.20f; x += 0.05f) {
        glVertex2f(x, -0.2f); glVertex2f(x, 0.45f);
    }
    for (float y = -0.15f; y <= 0.40f; y += 0.05f) {
        glVertex2f(-0.25f, y); glVertex2f(0.25f, y);
    }
    glEnd();

    for (float wx = -0.78f; wx <= 0.70f; wx += 0.09f) {
        if (wx >= -0.28f && wx <= 0.22f) continue;
        for (float wy = -0.12f; wy <= 0.20f; wy += 0.07f) {
            glBegin(GL_QUADS);
            if (currentMode == NIGHT_MODE || currentMode == EVENING_MODE) {
                glColor3f(1.0f, 0.85f, 0.3f);
            } else {
                glColor3f(0.3f, 0.45f, 0.6f);
            }
            glVertex2f(wx, wy);
            glVertex2f(wx + 0.05f, wy);
            glVertex2f(wx + 0.05f, wy + 0.04f);
            glVertex2f(wx, wy + 0.04f);
            glEnd();
        }
    }
}

// Architectural Component 3: Entrance Canopy & Pillars
void display3() {
    glBegin(GL_QUADS);
    if (currentMode == NIGHT_MODE) glColor3f(0.25f, 0.25f, 0.3f);
    else glColor3f(0.8f, 0.15f, 0.15f);

    glVertex2f(-0.22f, -0.08f);
    glVertex2f(0.22f, -0.08f);
    glVertex2f(0.25f, -0.03f);
    glVertex2f(-0.25f, -0.03f);

    glColor3f(0.9f, 0.9f, 0.9f);
    glVertex2f(-0.20f, -0.20f); glVertex2f(-0.18f, -0.20f); glVertex2f(-0.18f, -0.08f); glVertex2f(-0.20f, -0.08f);
    glVertex2f(-0.10f, -0.20f); glVertex2f(-0.08f, -0.20f); glVertex2f(-0.08f, -0.08f); glVertex2f(-0.10f, -0.08f);
    glVertex2f(0.08f, -0.20f);  glVertex2f(0.10f, -0.20f);  glVertex2f(0.10f, -0.08f);  glVertex2f(0.08f, -0.08f);
    glVertex2f(0.18f, -0.20f);  glVertex2f(0.20f, -0.20f);  glVertex2f(0.20f, -0.08f);  glVertex2f(0.18f, -0.08f);
    glEnd();

    glBegin(GL_QUADS);
    if (currentMode == NIGHT_MODE || currentMode == EVENING_MODE) glColor3f(1.0f, 0.9f, 0.5f);
    else glColor3f(0.2f, 0.3f, 0.4f);
    glVertex2f(-0.06f, -0.20f); glVertex2f(0.06f, -0.20f);
    glVertex2f(0.06f, -0.08f);  glVertex2f(-0.06f, -0.08f);
    glEnd();

    glLineWidth(2.0f);
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_LINES);
    glVertex2f(0.0f, -0.20f); glVertex2f(0.0f, -0.08f);
    glEnd();
}

// Architectural Component 4: Balconies
void display4() {
    for (float by = 0.0f; by <= 0.35f; by += 0.1f) {
        glBegin(GL_QUADS);
        glColor3f(0.7f, 0.7f, 0.75f);
        glVertex2f(-0.26f, by - 0.01f);
        glVertex2f(0.26f, by - 0.01f);
        glVertex2f(0.26f, by);
        glVertex2f(-0.26f, by);
        glEnd();

        glLineWidth(1.0f);
        glColor3f(0.2f, 0.2f, 0.25f);
        glBegin(GL_LINES);
        glVertex2f(-0.26f, by + 0.015f); glVertex2f(0.26f, by + 0.015f);
        for (float rx = -0.25f; rx <= 0.25f; rx += 0.05f) {
            glVertex2f(rx, by); glVertex2f(rx, by + 0.015f);
        }
        glEnd();
    }
}

// Architectural Component 5: Rooftop Crown & Flag
void display5() {
    glBegin(GL_QUADS);
    if (currentMode == NIGHT_MODE) glColor3f(0.2f, 0.3f, 0.45f);
    else glColor3f(0.15f, 0.35f, 0.65f);

    glVertex2f(-0.30f, 0.45f);
    glVertex2f(0.30f, 0.45f);
    glVertex2f(0.25f, 0.52f);
    glVertex2f(-0.25f, 0.52f);
    glEnd();

    glBegin(GL_QUADS);
    if (currentMode == NIGHT_MODE) glColor3f(0.0f, 0.3f, 0.8f);
    else glColor3f(0.05f, 0.2f, 0.5f);

    glVertex2f(-0.28f, 0.46f);
    glVertex2f(0.28f, 0.46f);
    glVertex2f(0.28f, 0.51f);
    glVertex2f(-0.28f, 0.51f);
    glEnd();

    // Flag Pole & Waving Bangladesh Flag
    glLineWidth(2.5f);
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_LINES);
    glVertex2f(0.0f, 0.52f);
    glVertex2f(0.0f, 0.65f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.42f, 0.24f);
    glVertex2f(0.0f, 0.58f);
    glVertex2f(0.09f, 0.58f);
    glVertex2f(0.09f, 0.64f);
    glVertex2f(0.0f, 0.64f);
    glEnd();

    drawCircle(0.045f, 0.61f, 0.018f, 15, 0.9f, 0.1f, 0.15f);

    if (currentSeason == WINTER_SEASON) {
        glBegin(GL_QUADS);
        glColor3f(0.95f, 0.98f, 1.0f);
        glVertex2f(-0.83f, 0.25f); glVertex2f(-0.54f, 0.25f);
        glVertex2f(-0.54f, 0.27f); glVertex2f(-0.83f, 0.27f);
        glVertex2f(0.54f, 0.25f);  glVertex2f(0.83f, 0.25f);
        glVertex2f(0.83f, 0.27f);  glVertex2f(0.54f, 0.27f);
        glVertex2f(-0.26f, 0.52f); glVertex2f(0.26f, 0.52f);
        glVertex2f(0.26f, 0.54f);  glVertex2f(-0.26f, 0.54f);
        glEnd();
    }
}

void display1_view() {
    glLoadIdentity();
}

// Moving Boat on River Rendering Function
void drawBoat(float bx, float by) {
    glPushMatrix();
    glTranslatef(bx, by, 0.0f);

    // Boat Mahogany Wood Hull
    glBegin(GL_POLYGON);
    glColor3f(0.48f, 0.25f, 0.10f);
    glVertex2f(-0.14f, 0.03f);
    glVertex2f(0.14f, 0.03f);
    glVertex2f(0.10f, -0.04f);
    glVertex2f(-0.10f, -0.04f);
    glEnd();

    // Hull Deck Trim
    glLineWidth(2.0f);
    glColor3f(0.85f, 0.85f, 0.9f);
    glBegin(GL_LINES);
    glVertex2f(-0.14f, 0.03f); glVertex2f(0.14f, 0.03f);
    glEnd();

    // Cabin Structure
    glBegin(GL_QUADS);
    glColor3f(0.92f, 0.92f, 0.95f);
    glVertex2f(-0.06f, 0.03f);
    glVertex2f(0.04f, 0.03f);
    glVertex2f(0.04f, 0.09f);
    glVertex2f(-0.06f, 0.09f);

    // Cabin Red Roof
    glColor3f(0.82f, 0.15f, 0.15f);
    glVertex2f(-0.07f, 0.09f);
    glVertex2f(0.05f, 0.09f);
    glVertex2f(0.04f, 0.11f);
    glVertex2f(-0.06f, 0.11f);
    glEnd();

    // Cabin Window
    glBegin(GL_QUADS);
    if (currentMode == NIGHT_MODE || currentMode == EVENING_MODE) glColor3f(1.0f, 0.9f, 0.4f);
    else glColor3f(0.3f, 0.5f, 0.7f);
    glVertex2f(-0.03f, 0.045f);
    glVertex2f(0.01f, 0.045f);
    glVertex2f(0.01f, 0.075f);
    glVertex2f(-0.03f, 0.075f);
    glEnd();

    // Mast & Flag
    glLineWidth(2.0f);
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_LINES);
    glVertex2f(0.09f, 0.03f);
    glVertex2f(0.09f, 0.16f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.1f, 0.6f, 0.9f);
    glVertex2f(0.09f, 0.16f);
    glVertex2f(0.14f, 0.13f);
    glVertex2f(0.09f, 0.10f);
    glEnd();

    // Water Ripple Wake behind boat
    glLineWidth(1.5f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
    glBegin(GL_LINES);
    glVertex2f(-0.14f, -0.02f); glVertex2f(-0.25f, -0.04f);
    glVertex2f(-0.12f, -0.03f); glVertex2f(-0.20f, -0.05f);
    glEnd();

    glPopMatrix();
}

// Lower Scenery: Front Lawn, 4 Trees, Thin Road, River & Moving Boat
void display_down() {
    // Hotel Front Lawn
    glBegin(GL_QUADS);
    if (currentSeason == WINTER_SEASON) glColor3f(0.8f, 0.85f, 0.85f);
    else if (currentMode == NIGHT_MODE) glColor3f(0.08f, 0.22f, 0.1f);
    else if (currentMode == EVENING_MODE) glColor3f(0.35f, 0.55f, 0.2f);
    else glColor3f(0.25f, 0.65f, 0.28f);

    glVertex2f(-1.0f, -0.40f);
    glVertex2f(1.0f, -0.40f);
    glVertex2f(1.0f, -0.20f);
    glVertex2f(-1.0f, -0.20f);
    glEnd();

    // Thin Two-Lane Road (y = -0.58f to -0.40f)
    glBegin(GL_QUADS);
    if (currentMode == NIGHT_MODE) glColor3f(0.12f, 0.12f, 0.14f);
    else if (currentMode == EVENING_MODE) glColor3f(0.22f, 0.20f, 0.22f);
    else glColor3f(0.28f, 0.28f, 0.3f);

    glVertex2f(-1.0f, -0.58f);
    glVertex2f(1.0f, -0.58f);
    glVertex2f(1.0f, -0.40f);
    glVertex2f(-1.0f, -0.40f);
    glEnd();

    // Sidewalk Curb Line
    glBegin(GL_QUADS);
    glColor3f(0.7f, 0.7f, 0.7f);
    glVertex2f(-1.0f, -0.41f); glVertex2f(1.0f, -0.41f);
    glVertex2f(1.0f, -0.40f);  glVertex2f(-1.0f, -0.40f);
    glEnd();

    // Center Dashed Yellow Road Divider Line (y = -0.49f)
    glLineWidth(2.5f);
    glColor3f(0.95f, 0.85f, 0.2f);
    glBegin(GL_LINES);
    for (float lx = -0.95f; lx <= 0.95f; lx += 0.15f) {
        glVertex2f(lx, -0.49f);
        glVertex2f(lx + 0.08f, -0.49f);
    }
    glEnd();

    // 4 Roadside Trees along Lawn Edge
    float treePosX[4] = {-0.75f, -0.25f, 0.25f, 0.75f};
    for (int i = 0; i < 4; i++) {
        float tx = treePosX[i];

        glBegin(GL_QUADS);
        glColor3f(0.42f, 0.26f, 0.12f);
        glVertex2f(tx - 0.012f, -0.40f);
        glVertex2f(tx + 0.012f, -0.40f);
        glVertex2f(tx + 0.008f, -0.28f);
        glVertex2f(tx - 0.008f, -0.28f);
        glEnd();

        glBegin(GL_TRIANGLES);
        if (currentSeason == WINTER_SEASON) glColor3f(0.35f, 0.48f, 0.40f);
        else if (currentMode == EVENING_MODE) glColor3f(0.75f, 0.35f, 0.12f);
        else if (currentMode == NIGHT_MODE) glColor3f(0.06f, 0.32f, 0.14f);
        else glColor3f(0.12f, 0.68f, 0.22f);

        glVertex2f(tx - 0.06f, -0.30f); glVertex2f(tx + 0.06f, -0.30f); glVertex2f(tx, -0.18f);
        glVertex2f(tx - 0.05f, -0.23f); glVertex2f(tx + 0.05f, -0.23f); glVertex2f(tx, -0.12f);
        glVertex2f(tx - 0.038f, -0.16f); glVertex2f(tx + 0.038f, -0.16f); glVertex2f(tx, -0.07f);
        glEnd();

        if (currentSeason == WINTER_SEASON) {
            glBegin(GL_TRIANGLES);
            glColor3f(0.95f, 0.98f, 1.0f);
            glVertex2f(tx - 0.035f, -0.16f);
            glVertex2f(tx + 0.035f, -0.16f);
            glVertex2f(tx, -0.07f);
            glEnd();
        }
    }

    // Streetlamps
    for (float lx = -0.85f; lx <= 0.85f; lx += 0.56f) {
        glLineWidth(3.0f);
        glColor3f(0.2f, 0.2f, 0.25f);
        glBegin(GL_LINES);
        glVertex2f(lx, -0.40f);
        glVertex2f(lx, -0.28f);
        glVertex2f(lx, -0.28f);
        glVertex2f(lx + 0.03f, -0.26f);
        glEnd();

        drawCircle(lx + 0.03f, -0.26f, 0.014f, 10, 0.95f, 0.95f, 0.9f);

        if (currentMode == NIGHT_MODE || currentMode == EVENING_MODE) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBegin(GL_TRIANGLES);
            glColor4f(1.0f, 0.92f, 0.5f, 0.35f);
            glVertex2f(lx + 0.03f, -0.26f);
            glVertex2f(lx - 0.08f, -0.40f);
            glVertex2f(lx + 0.14f, -0.40f);
            glEnd();
            glDisable(GL_BLEND);
        }
    }

    // Riverbank Embankment (y = -0.63f to -0.58f)
    glBegin(GL_QUADS);
    if (currentSeason == WINTER_SEASON) glColor3f(0.7f, 0.75f, 0.8f);
    else glColor3f(0.22f, 0.52f, 0.22f); // Green grassy river bank edge
    glVertex2f(-1.0f, -0.63f);
    glVertex2f(1.0f, -0.63f);
    glVertex2f(1.0f, -0.58f);
    glVertex2f(-1.0f, -0.58f);
    glEnd();

    // Embankment Stone Wall Line
    glLineWidth(2.0f);
    glColor3f(0.45f, 0.45f, 0.5f);
    glBegin(GL_LINES);
    glVertex2f(-1.0f, -0.63f); glVertex2f(1.0f, -0.63f);
    glEnd();

    // River Water Body (y = -1.00f to -0.63f)
    glBegin(GL_QUADS);
    if (currentMode == EVENING_MODE) {
        // Sunset Reddish-Orange Water Reflection
        glColor3f(0.75f, 0.22f, 0.08f); glVertex2f(-1.0f, -0.63f); glVertex2f(1.0f, -0.63f);
        glColor3f(0.45f, 0.12f, 0.05f); glVertex2f(1.0f, -1.00f); glVertex2f(-1.0f, -1.00f);
    } else if (currentMode == NIGHT_MODE) {
        // Deep Midnight Blue Water
        glColor3f(0.05f, 0.15f, 0.35f); glVertex2f(-1.0f, -0.63f); glVertex2f(1.0f, -0.63f);
        glColor3f(0.02f, 0.06f, 0.18f); glVertex2f(1.0f, -1.00f); glVertex2f(-1.0f, -1.00f);
    } else if (currentSeason == WINTER_SEASON) {
        // Frosty Blue Icy Water
        glColor3f(0.35f, 0.65f, 0.85f); glVertex2f(-1.0f, -0.63f); glVertex2f(1.0f, -0.63f);
        glColor3f(0.20f, 0.45f, 0.68f); glVertex2f(1.0f, -1.00f); glVertex2f(-1.0f, -1.00f);
    } else {
        // Sparkling River Water Blue
        glColor3f(0.08f, 0.52f, 0.85f); glVertex2f(-1.0f, -0.63f); glVertex2f(1.0f, -0.63f);
        glColor3f(0.02f, 0.35f, 0.68f); glVertex2f(1.0f, -1.00f); glVertex2f(-1.0f, -1.00f);
    }
    glEnd();

    // Animated Water Ripples
    glLineWidth(1.2f);
    if (currentMode == EVENING_MODE) glColor4f(1.0f, 0.6f, 0.3f, 0.4f);
    else if (currentMode == NIGHT_MODE) glColor4f(0.6f, 0.8f, 1.0f, 0.3f);
    else glColor4f(0.7f, 0.9f, 1.0f, 0.5f);

    glBegin(GL_LINES);
    for (float rx = -0.9f; rx <= 0.9f; rx += 0.3f) {
        float offset = sinf(rx * 10.0f + boatX * 5.0f) * 0.02f;
        glVertex2f(rx + offset, -0.72f);
        glVertex2f(rx + offset + 0.12f, -0.72f);

        glVertex2f(rx - offset, -0.88f);
        glVertex2f(rx - offset + 0.15f, -0.88f);
    }
    glEnd();

    // Moving Boat on River
    drawBoat(boatX, -0.80f);
}

// Vehicle 1: Bus moving Left to Right on Thin Road (y = -0.54f)
void dis() {
    glPushMatrix();
    glTranslatef(busX, -0.54f, 0.0f);

    glBegin(GL_QUADS);
    glColor3f(0.85f, 0.15f, 0.15f);
    glVertex2f(-0.12f, 0.01f);
    glVertex2f(0.12f, 0.01f);
    glVertex2f(0.12f, 0.11f);
    glVertex2f(-0.12f, 0.11f);

    glColor3f(0.15f, 0.15f, 0.2f);
    glVertex2f(-0.12f, 0.01f);
    glVertex2f(0.12f, 0.01f);
    glVertex2f(0.12f, 0.03f);
    glVertex2f(-0.12f, 0.03f);
    glEnd();

    for (float wx = -0.09f; wx <= 0.06f; wx += 0.045f) {
        glBegin(GL_QUADS);
        if (currentMode == NIGHT_MODE || currentMode == EVENING_MODE) glColor3f(1.0f, 0.95f, 0.6f);
        else glColor3f(0.7f, 0.85f, 0.95f);
        glVertex2f(wx, 0.05f);
        glVertex2f(wx + 0.03f, 0.05f);
        glVertex2f(wx + 0.03f, 0.095f);
        glVertex2f(wx, 0.095f);
        glEnd();
    }

    glBegin(GL_QUADS);
    if (currentMode == NIGHT_MODE || currentMode == EVENING_MODE) glColor3f(1.0f, 0.95f, 0.6f);
    else glColor3f(0.6f, 0.8f, 0.95f);
    glVertex2f(0.07f, 0.05f);
    glVertex2f(0.11f, 0.05f);
    glVertex2f(0.11f, 0.095f);
    glVertex2f(0.07f, 0.095f);
    glEnd();

    // Wheels
    glPushMatrix();
    glTranslatef(0.07f, 0.01f, 0.0f);
    drawCircle(0.0f, 0.0f, 0.02f, 15, 0.15f, 0.15f, 0.15f);
    drawCircle(0.0f, 0.0f, 0.01f, 12, 0.75f, 0.75f, 0.8f);
    glRotatef(wheelAngle, 0.0f, 0.0f, 1.0f);
    glLineWidth(1.5f);
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_LINES);
    glVertex2f(-0.01f, 0.0f); glVertex2f(0.01f, 0.0f);
    glVertex2f(0.0f, -0.01f); glVertex2f(0.0f, 0.01f);
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.07f, 0.01f, 0.0f);
    drawCircle(0.0f, 0.0f, 0.02f, 15, 0.15f, 0.15f, 0.15f);
    drawCircle(0.0f, 0.0f, 0.01f, 12, 0.75f, 0.75f, 0.8f);
    glRotatef(wheelAngle, 0.0f, 0.0f, 1.0f);
    glLineWidth(1.5f);
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_LINES);
    glVertex2f(-0.01f, 0.0f); glVertex2f(0.01f, 0.0f);
    glVertex2f(0.0f, -0.01f); glVertex2f(0.0f, 0.01f);
    glEnd();
    glPopMatrix();

    drawCircle(0.12f, 0.04f, 0.008f, 10, 1.0f, 0.95f, 0.3f);
    if (currentMode == NIGHT_MODE || currentMode == EVENING_MODE) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 1.0f, 0.6f, 0.4f);
        glVertex2f(0.12f, 0.04f);
        glVertex2f(0.30f, -0.03f);
        glVertex2f(0.30f, 0.08f);
        glEnd();
        glDisable(GL_BLEND);
    }

    glPopMatrix();
}

// Vehicle 2: Car moving Right to Left on Thin Road (y = -0.46f)
void disback() {
    glPushMatrix();
    glTranslatef(carX, -0.46f, 0.0f);

    glBegin(GL_QUADS);
    glColor3f(0.1f, 0.45f, 0.85f);
    glVertex2f(-0.09f, 0.01f);
    glVertex2f(0.09f, 0.01f);
    glVertex2f(0.09f, 0.05f);
    glVertex2f(-0.09f, 0.05f);

    glVertex2f(-0.04f, 0.05f);
    glVertex2f(0.03f, 0.05f);
    glVertex2f(0.015f, 0.09f);
    glVertex2f(-0.035f, 0.09f);
    glEnd();

    glBegin(GL_QUADS);
    if (currentMode == NIGHT_MODE || currentMode == EVENING_MODE) glColor3f(1.0f, 0.95f, 0.6f);
    else glColor3f(0.8f, 0.92f, 1.0f);
    glVertex2f(-0.03f, 0.055f);
    glVertex2f(0.01f, 0.055f);
    glVertex2f(0.0f, 0.085f);
    glVertex2f(-0.03f, 0.085f);
    glEnd();

    glPushMatrix();
    glTranslatef(-0.05f, 0.01f, 0.0f);
    drawCircle(0.0f, 0.0f, 0.016f, 15, 0.15f, 0.15f, 0.15f);
    drawCircle(0.0f, 0.0f, 0.008f, 10, 0.8f, 0.8f, 0.8f);
    glRotatef(-wheelAngle, 0.0f, 0.0f, 1.0f);
    glLineWidth(1.5f);
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_LINES);
    glVertex2f(-0.008f, 0.0f); glVertex2f(0.008f, 0.0f);
    glVertex2f(0.0f, -0.008f); glVertex2f(0.0f, 0.008f);
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.05f, 0.01f, 0.0f);
    drawCircle(0.0f, 0.0f, 0.016f, 15, 0.15f, 0.15f, 0.15f);
    drawCircle(0.0f, 0.0f, 0.008f, 10, 0.8f, 0.8f, 0.8f);
    glRotatef(-wheelAngle, 0.0f, 0.0f, 1.0f);
    glLineWidth(1.5f);
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_LINES);
    glVertex2f(-0.008f, 0.0f); glVertex2f(0.008f, 0.0f);
    glVertex2f(0.0f, -0.008f); glVertex2f(0.0f, 0.008f);
    glEnd();
    glPopMatrix();

    drawCircle(-0.09f, 0.03f, 0.007f, 10, 1.0f, 0.95f, 0.3f);
    if (currentMode == NIGHT_MODE || currentMode == EVENING_MODE) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 1.0f, 0.6f, 0.4f);
        glVertex2f(-0.09f, 0.03f);
        glVertex2f(-0.25f, -0.02f);
        glVertex2f(-0.25f, 0.07f);
        glEnd();
        glDisable(GL_BLEND);
    }

    glPopMatrix();
}

// Particle Rendering: Rain Effect
void update_day() {
    if (!isRaining) return;

    glLineWidth(1.8f);
    glColor4f(0.7f, 0.8f, 0.95f, 0.75f);
    glBegin(GL_LINES);
    for (int i = 0; i < MAX_RAIN; i++) {
        glVertex2f(rainX[i], rainY[i]);
        glVertex2f(rainX[i] - 0.02f, rainY[i] - 0.06f);
    }
    glEnd();
}

// Particle Rendering: Winter Snow Effect
void update_night() {
    if (currentSeason != WINTER_SEASON) return;

    glPointSize(3.5f);
    glColor3f(0.95f, 0.98f, 1.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_SNOW; i++) {
        glVertex2f(snowX[i], snowY[i]);
    }
    glEnd();
}

void update1() {
    if (isPaused) return;

    busX += vehicleSpeed;
    if (busX > 1.3f) busX = -1.3f;

    carX -= vehicleSpeed;
    if (carX < -1.3f) carX = 1.3f;

    wheelAngle -= (vehicleSpeed / 0.008f) * 8.0f;
    if (wheelAngle < -360.0f) wheelAngle += 360.0f;
}

void update2() {
    if (isPaused) return;

    cloud1X += cloudSpeed;
    if (cloud1X > 1.3f) cloud1X = -1.3f;

    cloud2X += cloudSpeed * 1.2f;
    if (cloud2X > 1.3f) cloud2X = -1.3f;

    bird1X += birdSpeed;
    if (bird1X > 1.3f) bird1X = -1.3f;

    bird2X += birdSpeed * 1.3f;
    if (bird2X > 1.3f) bird2X = -1.3f;

    wingFlapAngle += (birdSpeed / 0.003f) * 0.2f;

    boatX += boatSpeed;
    if (boatX > 1.3f) boatX = -1.3f;

    sunAngle += 0.5f;

    if (isRaining) {
        for (int i = 0; i < MAX_RAIN; i++) {
            rainY[i] -= rainSpeed[i];
            rainX[i] -= 0.005f;
            if (rainY[i] < -1.0f) {
                rainY[i] = 1.0f;
                rainX[i] = ((float)rand() / RAND_MAX) * 2.4f - 1.2f;
            }
        }
    }

    if (currentSeason == WINTER_SEASON) {
        for (int i = 0; i < MAX_SNOW; i++) {
            snowY[i] -= snowSpeed[i];
            snowX[i] += sinf(snowY[i] * 5.0f) * 0.002f;
            if (snowY[i] < -1.0f) {
                snowY[i] = 1.0f;
                snowX[i] = ((float)rand() / RAND_MAX) * 2.4f - 1.2f;
            }
        }
    }
}

void update(int value) {
    update1();
    update2();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// Main Display Callback Function
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    display1_view();

    display_up();
    display1();
    display2();
    display3();
    display4();
    display5();
    display_down();

    dis();
    disback();

    update_day();
    update_night();

    glFlush();
    glutSwapBuffers();
}

void Idle() {
    glutPostRedisplay();
}

// Keyboard Callback
void handleKeypress(unsigned char key, int x, int y) {
    std::cout << "[Keyboard Input] Key pressed: '" << key << "'" << std::endl;
    switch (key) {
        case 'e':
        case 'E':
            currentMode = EVENING_MODE;
            std::cout << "-> Switched to Evening Mode (Red Sunset Sky Vibe)" << std::endl;
            break;

        case 'n':
        case 'N':
            currentMode = NIGHT_MODE;
            std::cout << "-> Switched to Night Mode" << std::endl;
            break;

        case 'd':
        case 'D':
            currentMode = DAY_MODE;
            std::cout << "-> Switched to Day Mode" << std::endl;
            break;

        case 'f':
        case 'F':
            isFastSpeed = !isFastSpeed;
            vehicleSpeed = isFastSpeed ? 0.022f : 0.008f;
            boatSpeed    = isFastSpeed ? 0.010f : 0.0035f;
            birdSpeed    = isFastSpeed ? 0.009f : 0.003f;
            std::cout << "-> Fast Speed Toggled for Vehicles, Boat & Birds: " << (isFastSpeed ? "FAST MODE" : "NORMAL SPEED") << std::endl;
            break;

        case 'w':
        case 'W':
            currentSeason = (currentSeason == SUMMER_SEASON) ? WINTER_SEASON : SUMMER_SEASON;
            std::cout << "-> Season Toggled: " << (currentSeason == WINTER_SEASON ? "Winter" : "Summer") << std::endl;
            break;

        case 'r':
            isRaining = !isRaining;
            std::cout << "-> Rain Mode: " << (isRaining ? "ON" : "OFF") << std::endl;
            sound(1);
            break;

        case 'h':
        case 'H':
            playCarHorn();
            break;

        case 'R':
            currentMode = DAY_MODE;
            currentSeason = SUMMER_SEASON;
            isRaining = false;
            isPaused = false;
            isFastSpeed = false;
            vehicleSpeed = 0.008f;
            boatSpeed = 0.0035f;
            birdSpeed = 0.003f;
            cloudSpeed = 0.002f;
            busX = -1.2f;
            carX = 1.2f;
            boatX = -0.9f;
            bird1X = -0.7f;
            bird2X = 0.2f;
            std::cout << "-> Reset to Summer Day Default State!" << std::endl;
            break;

        case ' ':
            isPaused = !isPaused;
            std::cout << "-> Animation " << (isPaused ? "PAUSED" : "RESUMED") << std::endl;
            break;

        case 's':
        case 'S':
            playCarHorn();
            break;

        case 'q':
        case 27:
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void SpecialInput(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            vehicleSpeed += 0.003f;
            if (vehicleSpeed > 0.04f) vehicleSpeed = 0.04f;
            std::cout << "-> Vehicle Speed: " << vehicleSpeed << std::endl;
            break;

        case GLUT_KEY_DOWN:
            vehicleSpeed -= 0.003f;
            if (vehicleSpeed < 0.001f) vehicleSpeed = 0.001f;
            std::cout << "-> Vehicle Speed: " << vehicleSpeed << std::endl;
            break;

        case GLUT_KEY_LEFT:
            cloudSpeed -= 0.001f;
            if (cloudSpeed < -0.01f) cloudSpeed = -0.01f;
            break;

        case GLUT_KEY_RIGHT:
            cloudSpeed += 0.001f;
            if (cloudSpeed > 0.01f) cloudSpeed = 0.01f;
            break;
    }
    glutPostRedisplay();
}

// Mouse Input Callback
void handleMouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        if (button == GLUT_LEFT_BUTTON) {
            if (currentMode == DAY_MODE) currentMode = EVENING_MODE;
            else if (currentMode == EVENING_MODE) currentMode = NIGHT_MODE;
            else currentMode = DAY_MODE;
            std::cout << "[Mouse Click] Left Button -> Cycled Day/Evening/Night Mode!" << std::endl;
        } else if (button == GLUT_RIGHT_BUTTON) {
            std::cout << "[Mouse Click] Right Button -> Honking Car Horn!" << std::endl;
            playCarHorn();
        }
        glutPostRedisplay();
    }
}

int main(int argc, char** argv) {
    std::cout << "=======================================================" << std::endl;
    std::cout << " Starting Chattogram City 2D OpenGL Simulation...     " << std::endl;
    std::cout << "=======================================================" << std::endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Chattogram City Scenario - 2D OpenGL Simulation");

    init();
    initGL();

    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeypress);
    glutSpecialFunc(SpecialInput);
    glutMouseFunc(handleMouse);
    glutIdleFunc(Idle);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
