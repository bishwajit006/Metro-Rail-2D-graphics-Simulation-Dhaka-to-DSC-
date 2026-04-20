#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#define PI 3.14159265358979323846

// ==========================================
// MASSIVE GLOBAL VARIABLES & STATE ENGINE
// ==========================================
float trainX = -5.5f;
float doorOffset = 0.0f;
float wheelAngle = 0.0f;
float trainSpeed = 0.0f;
int trainState = 0; // 0:Arrive, 1:Wait, 2:Open, 3:Deboard, 4:Board, 5:Close, 6:Wait, 7:Exit
int waitTimer = 0;

int currentStation = 0;
float cloudX[10] = {-1.0f, -0.6f, -0.2f, 0.2f, 0.6f, 1.0f, 1.4f, 1.8f, -1.5f, 0.0f};
float cloudSpeed[10];
float planeX = -2.0f;
float planeY = 0.85f;
float busX = -3.0f;
int isDayTime = 1;
float starsX[150], starsY[150];
float sunAngle = 0.0f;

// Advanced Human Kinematics
typedef struct {
    float x, y, scale;
    int type, state, hasBag, hasHat;
    float walkTimer, speed;
    float rS, gS, bS;
    float rP, gP, bP;
    float rH, gH, bH;
} Person;

Person deboarders[8];
Person boarders[8];
int showBoarding = 1, showDeboarding = 0;

// ==========================================
// CORE DRAWING PRIMITIVES
// ==========================================
void drawText(float x, float y, char *string, void* font) {
    glRasterPos2f(x, y);
    for (int i = 0; i < (int)strlen(string); i++) glutBitmapCharacter(font, string[i]);
}

void drawRect(float x, float y, float w, float h, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

void drawGradientRect(float x, float y, float w, float h, float r1, float g1, float b1, float r2, float g2, float b2) {
    glBegin(GL_QUADS);
    glColor3f(r1, g1, b1); glVertex2f(x, y); glVertex2f(x + w, y);
    glColor3f(r2, g2, b2); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

void drawCircle(float cx, float cy, float r, int segments, float red, float green, float blue) {
    glColor3f(red, green, blue);
    glBegin(GL_POLYGON);
    for (int i = 0; i < segments; i++) {
        glVertex2f(cx + r * cos(2.0f * PI * i / segments), cy + r * sin(2.0f * PI * i / segments));
    }
    glEnd();
}

void drawHollowCircle(float cx, float cy, float r, float thickness, int segments, float red, float green, float blue) {
    glColor3f(red, green, blue);
    glLineWidth(thickness);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++) glVertex2f(cx + r * cos(2.0f * PI * i / segments), cy + r * sin(2.0f * PI * i / segments));
    glEnd();
    glLineWidth(1.0f);
}

// ==========================================
// ENVIRONMENT, DAY/NIGHT & SKY ENGINE
// ==========================================
void initEnvironment() {
    for(int i=0; i<150; i++) {
        starsX[i] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        starsY[i] = ((float)rand() / RAND_MAX) * 0.7f + 0.3f;
    }
    for(int i=0; i<10; i++) {
        cloudSpeed[i] = 0.0003f + ((float)rand() / RAND_MAX) * 0.0005f;
    }
}

void drawDetailedClouds() {
    for(int i=0; i<10; i++) {
        float cx = cloudX[i];
        float cy = 0.5f + (i * 0.05f);
        float cr = isDayTime ? 1.0f : 0.35f;
        drawCircle(cx, cy, 0.06f, 30, cr, cr, cr);
        drawCircle(cx + 0.08f, cy + 0.03f, 0.08f, 30, cr, cr, cr);
        drawCircle(cx + 0.15f, cy, 0.06f, 30, cr, cr, cr);
        drawCircle(cx + 0.04f, cy - 0.02f, 0.05f, 30, cr*0.9f, cr*0.9f, cr*0.9f);
        drawCircle(cx + 0.12f, cy - 0.02f, 0.04f, 30, cr*0.9f, cr*0.9f, cr*0.9f);
    }
}

void drawSky() {
    if(isDayTime) {
        drawGradientRect(-1.0, -0.2, 2.0, 1.2, 0.4, 0.7, 1.0, 0.6, 0.85, 1.0);
        glPushMatrix();
        glTranslatef(0.7f, 0.8f, 0.0f);
        glRotatef(sunAngle, 0, 0, 1);
        glColor4f(1.0f, 0.9f, 0.2f, 0.4f); drawCircle(0.0f, 0.0f, 0.15f, 40, 1, 1, 1);
        drawCircle(0.0f, 0.0f, 0.1f, 40, 1.0f, 0.9f, 0.1f);
        glColor3f(1.0f, 0.8f, 0.0f);
        for(int i=0; i<12; i++) {
            float a = i * (PI/6.0f);
            glBegin(GL_LINES); glVertex2f(0.12f * cos(a), 0.12f * sin(a)); glVertex2f(0.18f * cos(a), 0.18f * sin(a)); glEnd();
        }
        glPopMatrix();
    } else {
        drawGradientRect(-1.0, -0.2, 2.0, 1.2, 0.02, 0.02, 0.08, 0.05, 0.1, 0.2);
        glBegin(GL_POINTS);
        for(int i=0; i<150; i++) {
            if(rand()%10 > 2) glColor3f(1.0, 1.0, 1.0); else glColor3f(0.5, 0.5, 0.5);
            glVertex2f(starsX[i], starsY[i]);
        }
        glEnd();
        drawCircle(0.7f, 0.8f, 0.08f, 40, 0.9f, 0.9f, 0.8f);
        drawCircle(0.67f, 0.82f, 0.015f, 20, 0.8f, 0.8f, 0.7f);
        drawCircle(0.73f, 0.78f, 0.01f, 20, 0.8f, 0.8f, 0.7f);
        drawCircle(0.68f, 0.76f, 0.02f, 20, 0.8f, 0.8f, 0.7f);
        drawCircle(0.65f, 0.83f, 0.08f, 40, 0.05f, 0.1f, 0.2f);
    }
    drawDetailedClouds();
}

// ==========================================
// NEW: OVERHEAD ELECTRIC WIRES (CATENARY SYSTEM)
// ==========================================
void drawOverheadWires() {
    // Upper Catnary Wire
    drawRect(-2.0f, 0.36f, 4.0f, 0.005f, 0.15f, 0.15f, 0.15f);
    // Lower Contact Wire (Touches the pantograph)
    drawRect(-2.0f, 0.28f, 4.0f, 0.005f, 0.25f, 0.25f, 0.25f);

    // Droppers (Vertical connecting wires)
    glColor3f(0.2f, 0.2f, 0.2f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for(float dx = -2.0f; dx <= 2.0f; dx += 0.15f) {
        glVertex2f(dx, 0.36f);
        glVertex2f(dx, 0.28f);
    }
    glEnd();

    // Masts (Support Poles)
    for(float px = -0.9f; px <= 1.2f; px += 0.8f) {
        // Vertical Mast
        drawGradientRect(px, 0.18f, 0.02f, 0.35f, 0.4f, 0.4f, 0.45f, 0.2f, 0.2f, 0.25f);
        // Top Support Arm
        drawRect(px - 0.15f, 0.36f, 0.15f, 0.01f, 0.2f, 0.2f, 0.2f);
        // Bottom Support Arm
        drawRect(px - 0.15f, 0.28f, 0.15f, 0.008f, 0.3f, 0.3f, 0.3f);

        // Diagonal tension rod
        glBegin(GL_LINES);
        glVertex2f(px, 0.45f);
        glVertex2f(px - 0.1f, 0.36f);
        glEnd();
    }
}

// ==========================================
// STATIONS
// ==========================================
void drawDIUBuilding() {
    // Ground + water
    drawGradientRect(-1.0, -0.3, 2.0, 0.4, 0.05, 0.3, 0.05, 0.0, 0.15, 0.0);
    drawRect(-1.0f, -0.3f, 2.0f, 0.08f, 0.0, 0.3, 0.6);

    // --- LEFT TALL BUILDING ---
    drawRect(-0.95, -0.1, 0.3, 0.9, 0.96, 0.96, 0.96);
    for(float h = -0.05; h < 0.75; h += 0.09) {
        for(float w = -0.92; w < -0.7; w += 0.07)
            drawRect(w, h, 0.045, 0.055, 0.2, 0.4, 0.7);
    }

    // --- LEFT SIDE SMALL BLOCK ---
    drawRect(-0.65, 0.2, 0.15, 0.6, 0.95, 0.95, 0.95);
    drawRect(-0.65, -0.1, 0.15, 0.25, 0.9, 0.9, 0.9);
    drawGradientRect(-0.5, -0.1, 0.06, 0.85, 0.6, 0.8, 1.0, 0.3, 0.5, 0.8);

    // --- GREEN WALL CENTER ---
    drawRect(-0.44, -0.1, 0.22, 0.75, 0.2, 0.55, 0.25);
    for(float x = -0.43; x < -0.25; x += 0.02)
        drawRect(x, -0.1, 0.004, 0.75, 0.1, 0.4, 0.1);

    // --- MAIN RIGHT BUILDING (BACK) ---
    drawRect(0.0, -0.1, 0.9, 0.85, 0.97, 0.97, 0.97);
    for(float h = 0.0; h < 0.75; h += 0.12)
        drawRect(0.05, h, 0.8, 0.06, 0.2, 0.4, 0.7);

    // --- FRONT CENTER BLOCK ---
    drawRect(0.25, -0.1, 0.45, 0.65, 0.95, 0.95, 0.95);
    for(float h = 0.0; h < 0.6; h += 0.1)
        drawRect(0.28, h, 0.39, 0.05, 0.2, 0.4, 0.7);
    for(float x = 0.28; x < 0.65; x += 0.08)
        drawRect(x, -0.1, 0.008, 0.65, 0.85, 0.85, 0.85);

    // --- ROOF NAME BAR ---
    drawRect(0.35, 0.75, 0.5, 0.06, 0.15, 0.25, 0.55);
    glColor3f(1,1,1);
    drawText(0.37, 0.77, "Daffodil International University", GLUT_BITMAP_HELVETICA_12);

    // --- TREES ---
    for(float tx = -1.0; tx < 1.1; tx += 0.12) {
        drawCircle(tx, -0.15, 0.12, 15, 0.1, 0.5, 0.1);
        drawCircle(tx+0.04, -0.1, 0.1, 15, 0.2, 0.6, 0.2);
    }
}

void drawDhakaStation() {
    drawGradientRect(-1.0, -0.2, 2.0, 1.2, 0.8f, 0.82f, 0.85f, 0.55f, 0.55f, 0.6f);
    glColor3f(0.5f, 0.5f, 0.5f);
    for(float lineY=0.0f; lineY<1.0f; lineY+=0.15f) {
        glBegin(GL_LINES); glVertex2f(-1.0f, lineY); glVertex2f(1.0f, lineY); glEnd();
    }
    for(float x = -0.9; x <= 0.9; x += 0.45) {
        drawGradientRect(x, -0.2, 0.15, 1.0, 0.7f, 0.7f, 0.7f, 0.4f, 0.4f, 0.45f);
        drawGradientRect(x, -0.2, 0.02, 1.0, 0.3f, 0.3f, 0.3f, 0.15f, 0.15f, 0.2f);
    }
    drawRect(-0.6f, 0.2f, 0.25f, 0.15f, 0.05f, 0.05f, 0.05f);
    drawGradientRect(-0.58f, 0.22f, 0.21f, 0.11f, 0.1f, 0.4f, 0.8f, 0.0f, 0.2f, 0.5f);
    glColor3f(1, 1, 1); drawText(-0.55f, 0.25f, "Dhaka", GLUT_BITMAP_HELVETICA_10);
    drawRect(0.5f, 0.2f, 0.25f, 0.15f, 0.05f, 0.05f, 0.05f);
    drawGradientRect(0.52f, 0.22f, 0.21f, 0.11f, 0.8f, 0.4f, 0.1f, 0.5f, 0.2f, 0.0f);
    glColor3f(1, 1, 1); drawText(0.55f, 0.25f, "Smart Card", GLUT_BITMAP_HELVETICA_10);
    drawRect(-0.25f, 0.65f, 0.5f, 0.15f, 0.1f, 0.1f, 0.1f);
    drawRect(-0.24f, 0.66f, 0.48f, 0.13f, 1.0f, 1.0f, 1.0f);
    drawRect(-0.24f, 0.66f, 0.48f, 0.02f, 0.0f, 0.6f, 0.3f);
    glColor3f(0.1f, 0.1f, 0.1f); drawText(-0.1f, 0.72f, "DHAKA METRO", GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINES); glVertex2f(-0.15f, 0.8f); glVertex2f(-0.15f, 1.0f); glVertex2f(0.15f, 0.8f); glVertex2f(0.15f, 1.0f); glEnd();
    for(float x = -1.0; x < 1.0; x += 0.3) {
        drawRect(x, 0.2f, 0.28f, 0.05f, 0.0f, 0.5f, 0.2f);
        glColor4f(0.6f, 0.8f, 0.9f, 0.2f);
        glRectf(x, -0.2, x+0.28, 0.2);
        glColor3f(0.7f, 0.7f, 0.7f); glLineWidth(3.0f);
        glBegin(GL_LINE_LOOP); glVertex2f(x,-0.2); glVertex2f(x+0.28,-0.2); glVertex2f(x+0.28,0.2); glVertex2f(x,0.2); glEnd();
        glLineWidth(1.0f);
        drawRect(x+0.1f, 0.05f, 0.08f, 0.02f, 1.0f, 0.8f, 0.0f);
        drawRect(x+0.1f, 0.03f, 0.08f, 0.015f, 0.8f, 0.1f, 0.1f);
        glColor3f(0,0,0); drawText(x+0.11f, 0.055f, "CAUTION", GLUT_BITMAP_HELVETICA_10);
    }
}

// ==========================================
// CROWD / HUMAN PHYSICS
// ==========================================
void initPeople() {
    float colors[8][3] = {{0.8,0.2,0.2}, {0.2,0.4,0.8}, {0.2,0.8,0.4}, {0.8,0.2,0.8}, {0.9,0.5,0.1}, {0.2,0.8,0.8}, {0.8,0.8,0.2}, {0.4,0.2,0.6}};
    for(int i=0; i<8; i++) {
        deboarders[i].x = -0.12f + (i*0.03f); deboarders[i].y = -0.15f + ((i%2)*0.02f); deboarders[i].scale = 0.85f + ((rand()%10)*0.01f);
        deboarders[i].type = rand()%2; deboarders[i].state = 0; deboarders[i].hasBag = rand()%2; deboarders[i].speed = 0.0035f + ((rand()%10)*0.0001f);
        deboarders[i].rS = colors[i%8][0]; deboarders[i].gS = colors[i%8][1]; deboarders[i].bS = colors[i%8][2];

        boarders[i].x = 0.4f + (i*0.08f); boarders[i].y = -0.38f + ((i%3)*0.015f); boarders[i].scale = 0.85f + ((rand()%10)*0.01f);
        boarders[i].type = rand()%2; boarders[i].state = 0; boarders[i].hasBag = rand()%2; boarders[i].speed = 0.0035f + ((rand()%10)*0.0001f);
        boarders[i].rS = colors[(i+3)%8][0]; boarders[i].gS = colors[(i+3)%8][1]; boarders[i].bS = colors[(i+3)%8][2];
    }
}

void drawArticulatedPerson(Person *p) {
    glPushMatrix();
    float bob = 0.0f, swingAngle = 0.0f, elbowAngle = 0.0f;
    if (p->state == 1) { swingAngle = sin(p->walkTimer) * 20.0f; bob = fabs(sin(p->walkTimer * 2.0f)) * 0.005f; elbowAngle = fabs(sin(p->walkTimer)) * 10.0f; }
    glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
    drawCircle(p->x + 0.02f, p->y - 0.05f, 0.03f - (bob*0.2f), 15, 0,0,0);
    glTranslatef(p->x, p->y + bob, 0.0f); glScalef(p->scale, p->scale, 1.0f);
    float rP = 0.15f, gP = 0.15f, bP = 0.2f;

    glPushMatrix(); glTranslatef(0.025f, 0.05f, 0.0f); glRotatef(-swingAngle, 0, 0, 1);
    drawRect(-0.012f, -0.05f, 0.024f, 0.05f, rP, gP, bP);
    drawRect(-0.018f, -0.065f, 0.03f, 0.015f, 0.05f,0.05f,0.05f);
    drawRect(-0.01f, -0.055f, 0.01f, 0.01f, 0.8f,0.8f,0.8f); glPopMatrix();

    glPushMatrix(); glTranslatef(0.025f, 0.05f, 0.0f); glRotatef(swingAngle, 0, 0, 1);
    drawRect(-0.012f, -0.05f, 0.024f, 0.05f, rP*0.8f, gP*0.8f, bP*0.8f);
    drawRect(-0.018f, -0.065f, 0.03f, 0.015f, 0.05f,0.05f,0.05f); glPopMatrix();

    drawRect(-0.005f, 0.05f, 0.06f, 0.1f, p->rS, p->gS, p->bS);
    drawRect(-0.005f, 0.05f, 0.06f, 0.01f, 0.2f, 0.1f, 0.0f);
    drawRect(0.02f, 0.05f, 0.01f, 0.01f, 0.8f, 0.8f, 0.0f);

    glPushMatrix(); glTranslatef(0.025f, 0.13f, 0.0f); glRotatef(swingAngle, 0, 0, 1);
    drawRect(-0.01f, -0.07f, 0.02f, 0.07f, p->rS*0.8f, p->gS*0.8f, p->bS*0.8f);
    glPushMatrix(); glTranslatef(0.0f, -0.07f, 0.0f); glRotatef(-elbowAngle, 0, 0, 1);
    drawRect(-0.008f, -0.02f, 0.016f, 0.02f, 0.9f, 0.7f, 0.6f); glPopMatrix(); glPopMatrix();

    glPushMatrix(); glTranslatef(0.025f, 0.13f, 0.0f); glRotatef(-swingAngle, 0, 0, 1);
    drawRect(-0.01f, -0.07f, 0.02f, 0.07f, p->rS, p->gS, p->bS);
    glPushMatrix(); glTranslatef(0.0f, -0.07f, 0.0f); glRotatef(elbowAngle, 0, 0, 1);
    drawRect(-0.008f, -0.02f, 0.016f, 0.02f, 0.9f, 0.7f, 0.6f);
    if (p->hasBag) { drawRect(-0.02f, -0.07f, 0.04f, 0.06f, 0.2f, 0.1f, 0.1f); drawRect(-0.005f, -0.01f, 0.01f, 0.02f, 0.1f, 0.1f, 0.1f); }
    glPopMatrix(); glPopMatrix();

    drawRect(0.015f, 0.15f, 0.02f, 0.01f, 0.8f, 0.6f, 0.5f);
    drawCircle(0.025f, 0.175f, 0.022f, 20, 0.9f, 0.7f, 0.6f);
    if(p->type == 1) { drawCircle(0.025f, 0.185f, 0.024f, 20, 0.1f, 0.05f, 0.0f); drawRect(0.001f, 0.15f, 0.048f, 0.04f, 0.1f, 0.05f, 0.0f); }
    else { drawCircle(0.025f, 0.185f, 0.024f, 20, 0.1f, 0.1f, 0.1f); glBegin(GL_TRIANGLES); glColor3f(0.1f,0.1f,0.1f); glVertex2f(0.01f, 0.19f); glVertex2f(0.02f, 0.21f); glVertex2f(0.03f, 0.19f); glEnd(); }
    glPopMatrix();
}

// ==========================================
// MASSIVE TRAIN RENDERING ENGINE
// ==========================================
void drawTrainBogieBase(float offsetX) {
    glPushMatrix(); glTranslatef(offsetX, 0.0f, 0.0f);

    // Interior
    drawRect(-0.15f, -0.18f, 0.3f, 0.36f, 0.85f, 0.85f, 0.9f);
    drawRect(-0.12f, -0.02f, 0.24f, 0.12f, 0.1f, 0.2f, 0.3f);
    drawRect(-0.15f, -0.1f, 0.06f, 0.08f, 0.9f, 0.4f, 0.0f);
    drawRect(-0.15f, -0.04f, 0.02f, 0.06f, 0.8f, 0.3f, 0.0f);
    drawRect(0.09f, -0.1f, 0.06f, 0.08f, 0.9f, 0.4f, 0.0f);
    drawRect(0.13f, -0.04f, 0.02f, 0.06f, 0.8f, 0.3f, 0.0f);

    drawRect(-0.08f, 0.14f, 0.16f, 0.03f, 0.05f, 0.05f, 0.05f);
    glColor3f(1.0f, 0.2f, 0.2f); glBegin(GL_POINTS); for(float px=-0.07f; px<0.07f; px+=0.005f) glVertex2f(px, 0.155f); glEnd();

    glColor3f(0.6f, 0.6f, 0.6f); glLineWidth(2.0f);
    glBegin(GL_LINES); glVertex2f(-0.15f, 0.12f); glVertex2f(0.15f, 0.12f); glEnd();
    glLineWidth(1.0f);
    for(float hx=-0.1f; hx<=0.1f; hx+=0.05f) {
        glBegin(GL_LINES); glVertex2f(hx, 0.12f); glVertex2f(hx, 0.08f); glEnd();
        drawHollowCircle(hx, 0.06f, 0.02f, 2.0f, 15, 0.9f, 0.7f, 0.0f);
    }
    drawRect(-0.06f, -0.18f, 0.015f, 0.36f, 0.7f, 0.7f, 0.75f);
    drawRect(0.045f, -0.18f, 0.015f, 0.36f, 0.7f, 0.7f, 0.75f);

    // Metallic Exterior
    drawGradientRect(-0.7f, -0.2f, 0.55f, 0.4f, 0.92f, 0.92f, 0.95f, 0.5f, 0.5f, 0.55f);
    drawGradientRect(0.15f, -0.2f, 0.55f, 0.4f, 0.92f, 0.92f, 0.95f, 0.5f, 0.5f, 0.55f);
    drawGradientRect(-0.15f, 0.18f, 0.3f, 0.02f, 0.95f, 0.95f, 0.98f, 0.7f, 0.7f, 0.75f);

    glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
    glRectf(-0.7f, 0.08f, -0.15f, 0.1f);
    glRectf(0.15f, 0.08f, 0.7f, 0.1f);

    drawRect(-0.7f, -0.05f, 1.4f, 0.02f, 0.85f, 0.1f, 0.1f);
    drawRect(-0.7f, -0.03f, 1.4f, 0.02f, 0.0f, 0.6f, 0.2f);

    // Windows (Corrected Position)
    float windowPositions[2] = {-0.65f, 0.3f};
    for(int w = 0; w < 2; w++) {
        float i = windowPositions[w];
        drawRect(i - 0.01f, 0.04f, 0.32f, 0.15f, 0.1f, 0.1f, 0.1f);
        drawGradientRect(i, 0.05f, 0.3f, 0.13f, 0.02f, 0.02f, 0.05f, 0.1f, 0.1f, 0.15f);
        glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
        glBegin(GL_POLYGON); glVertex2f(i+0.05f, 0.05f); glVertex2f(i+0.12f, 0.05f); glVertex2f(i+0.22f, 0.18f); glVertex2f(i+0.15f, 0.18f); glEnd();
        glBegin(GL_POLYGON); glVertex2f(i+0.15f, 0.05f); glVertex2f(i+0.18f, 0.05f); glVertex2f(i+0.28f, 0.18f); glVertex2f(i+0.25f, 0.18f); glEnd();
    }

    // Wheels
    drawRect(-0.7f, -0.25f, 1.4f, 0.05f, 0.1f, 0.1f, 0.1f);
    for(float wx = -0.4f; wx <= 0.4f; wx += 0.8f) {
        glPushMatrix(); glTranslatef(wx, -0.25f, 0.0f); glRotatef(wheelAngle, 0, 0, -1);
        drawCircle(0, 0, 0.05f, 25, 0.15f, 0.15f, 0.15f);
        drawCircle(0, 0, 0.03f, 20, 0.6f, 0.6f, 0.6f);
        drawCircle(0, 0, 0.01f, 15, 0.1f, 0.1f, 0.1f);
        glColor3f(0.0f,0.0f,0.0f); glBegin(GL_LINES); glVertex2f(-0.03f,0); glVertex2f(0.03f,0); glVertex2f(0,-0.03f); glVertex2f(0,0.03f); glEnd();
        glPopMatrix();
    }

    // Pantograph
    glColor3f(0.3f, 0.3f, 0.3f); glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP); glVertex2f(-0.3f, 0.2f); glVertex2f(-0.2f, 0.28f); glVertex2f(-0.1f, 0.2f); glEnd();
    glBegin(GL_LINES); glVertex2f(-0.25f, 0.28f); glVertex2f(-0.15f, 0.28f); glEnd(); glLineWidth(1.0f);

    glPopMatrix();
}

void drawTrainDoors(float offsetX) {
    glPushMatrix(); glTranslatef(offsetX, 0.0f, 0.0f);
    drawGradientRect(-0.15f - doorOffset, -0.2f, 0.15f, 0.38f, 0.85f, 0.85f, 0.88f, 0.5f, 0.5f, 0.55f);
    drawGradientRect(0.0f + doorOffset, -0.2f, 0.15f, 0.38f, 0.85f, 0.85f, 0.88f, 0.5f, 0.5f, 0.55f);

    drawRect(-0.15f - doorOffset, -0.05f, 0.15f, 0.02f, 0.85f, 0.1f, 0.1f); drawRect(-0.15f - doorOffset, -0.03f, 0.15f, 0.02f, 0.0f, 0.6f, 0.2f);
    drawRect(0.0f + doorOffset, -0.05f, 0.15f, 0.02f, 0.85f, 0.1f, 0.1f); drawRect(0.0f + doorOffset, -0.03f, 0.15f, 0.02f, 0.0f, 0.6f, 0.2f);

    drawRect(-0.14f - doorOffset, 0.04f, 0.13f, 0.15f, 0.1f, 0.1f, 0.1f);
    drawRect(0.01f + doorOffset, 0.04f, 0.13f, 0.15f, 0.1f, 0.1f, 0.1f);
    drawGradientRect(-0.13f - doorOffset, 0.05f, 0.11f, 0.13f, 0.02f, 0.02f, 0.05f, 0.1f, 0.1f, 0.15f);
    drawGradientRect(0.02f + doorOffset, 0.05f, 0.11f, 0.13f, 0.02f, 0.02f, 0.05f, 0.1f, 0.1f, 0.15f);

    drawRect(-0.01f - doorOffset, -0.2f, 0.01f, 0.38f, 0.15f, 0.15f, 0.15f);
    drawRect(0.0f + doorOffset, -0.2f, 0.01f, 0.38f, 0.15f, 0.15f, 0.15f);
    glPopMatrix();
}

void drawFullTrain() {
    glPushMatrix(); glTranslatef(trainX, 0.0f, 0.0f);

    drawTrainBogieBase(0.0f);
    drawTrainBogieBase(-1.45f);
    drawTrainBogieBase(-2.9f);

    // Rubber Connectors (Bogie link)
    drawRect(-0.75f, -0.18f, 0.05f, 0.36f, 0.15f, 0.15f, 0.15f);
    for(float by = -0.18f; by < 0.18f; by += 0.04f) drawRect(-0.75f, by, 0.05f, 0.005f, 0.05f, 0.05f, 0.05f);
    drawRect(-2.2f, -0.18f, 0.05f, 0.36f, 0.15f, 0.15f, 0.15f);
    for(float by = -0.18f; by < 0.18f; by += 0.04f) drawRect(-2.2f, by, 0.05f, 0.005f, 0.05f, 0.05f, 0.05f);

    // Curved Nose
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);
    glVertex2f(0.7f, -0.2f); glVertex2f(1.1f, -0.2f); glVertex2f(1.1f, -0.15f);
    for(int i=0; i<=90; i+=5) {
        float angle = i * PI / 180.0;
        glVertex2f(0.7f + 0.4f * cos(angle), -0.15f + 0.35f * sin(angle));
    }
    glVertex2f(0.7f, 0.2f);
    glEnd();

    // Curved Green Top
    glColor3f(0.0f, 0.5f, 0.2f);
    glBegin(GL_POLYGON);
    glVertex2f(0.7f, 0.2f);
    for(int i=90; i>=0; i-=5) {
        float angle = i * PI / 180.0;
        glVertex2f(0.7f + 0.4f * cos(angle), -0.15f + 0.35f * sin(angle));
    }
    for(int i=0; i<=90; i+=5) {
        float angle = i * PI / 180.0;
        glVertex2f(0.7f + 0.41f * cos(angle), -0.12f + 0.35f * sin(angle));
    }
    glEnd();

    // Curved Glass
    glColor3f(0.05f, 0.05f, 0.05f);
    glBegin(GL_POLYGON);
    glVertex2f(0.72f, 0.05f); glVertex2f(1.03f, 0.05f);
    for(int i=30; i<=80; i+=5) {
        float angle = i * PI / 180.0;
        glVertex2f(0.72f + 0.32f * cos(angle), -0.1f + 0.3f * sin(angle));
    }
    glVertex2f(0.72f, 0.2f);
    glEnd();

    // Embedded UTTARA Board
    drawRect(0.74f, 0.12f, 0.16f, 0.035f, 0.1f, 0.1f, 0.1f);
    glColor3f(1.0f, 0.7f, 0.0f);
    drawText(0.75f, 0.13f, "DIU METRO", GLUT_BITMAP_HELVETICA_10);

    // Red Bumper
    glColor3f(0.8f, 0.0f, 0.0f);
    glBegin(GL_POLYGON);
    glVertex2f(0.7f, -0.25f); glVertex2f(1.1f, -0.25f);
    glVertex2f(1.1f, -0.2f); glVertex2f(0.7f, -0.2f);
    glEnd();

    // Headlights
    drawCircle(0.78f, -0.1f, 0.018f, 20, 1, 1, 0.9f);
    drawCircle(1.02f, -0.1f, 0.018f, 20, 1, 1, 0.9f);
    drawCircle(0.78f, -0.15f, 0.008f, 15, 0.8f, 0.4f, 0.0f);
    drawCircle(1.02f, -0.15f, 0.008f, 15, 0.8f, 0.4f, 0.0f);

    glPopMatrix();
}

// ==========================================
// CONTROL DASHBOARD & METRICS
// ==========================================
void drawDashboard() {
    drawGradientRect(-1.0, -1.0, 2.0, 0.35, 0.1, 0.1, 0.15, 0.02, 0.02, 0.05);
    glColor3f(0.0f, 0.8f, 1.0f); glLineWidth(3.0f); glBegin(GL_LINES); glVertex2f(-1.0, -0.65); glVertex2f(1.0, -0.65); glEnd(); glLineWidth(1.0f);

    drawRect(-0.95f, -0.9f, 0.4f, 0.15f, 0.05f, 0.05f, 0.05f);
    drawHollowCircle(-0.9f, -0.825f, 0.05f, 2.0f, 30, 0.0f, 0.8f, 1.0f);
    time_t rawtime; struct tm * timeinfo; char timeBuf[15]; time(&rawtime); timeinfo = localtime(&rawtime); strftime(timeBuf, 15, "%I:%M:%S %p", timeinfo);
    glColor3f(0.0f, 1.0f, 0.5f); drawText(-0.82f, -0.85f, timeBuf, GLUT_BITMAP_HELVETICA_18);

    drawRect(-0.45f, -0.9f, 0.7f, 0.15f, 0.05f, 0.05f, 0.05f);
    char destMsg[50]; sprintf(destMsg, "DESTINATION: %s", (currentStation == 0 ? "DHAKA METRO" : "DIU CAMPUS"));
    glColor3f(1.0f, 0.8f, 0.0f); drawText(-0.35f, -0.85f, destMsg, GLUT_BITMAP_HELVETICA_18);

    drawRect(0.35f, -0.9f, 0.6f, 0.15f, 0.05f, 0.05f, 0.05f);
    float progress = (trainX + 5.5f) / 11.0f; if (progress < 0) progress = 0; if (progress > 1) progress = 1;
    drawRect(0.4f, -0.82f, 0.5f, 0.04f, 0.2f, 0.2f, 0.2f);
    drawGradientRect(0.4f, -0.82f, 0.5f * progress, 0.04f, 0.0f, 0.5f, 1.0f, 0.0f, 0.8f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f); drawText(0.4f, -0.88f, "ROUTE PROGRESS", GLUT_BITMAP_HELVETICA_10);
    char speedMsg[20]; sprintf(speedMsg, "SPD: %.0f KM/H", fabs(trainSpeed)*3000.0f);
    glColor3f(1.0f, 0.2f, 0.2f); drawText(0.75f, -0.88f, speedMsg, GLUT_BITMAP_HELVETICA_10);
}

// ==========================================
// CORE LOOP & PHYSICS ENGINE
// ==========================================
void display() {
    glClear(GL_COLOR_BUFFER_BIT); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH); glPointSize(3.0f);

    time_t rawtime; struct tm * timeinfo; time(&rawtime); timeinfo = localtime(&rawtime);
    isDayTime = (timeinfo->tm_hour >= 6 && timeinfo->tm_hour < 18);;

    drawSky();
    if (currentStation == 0) drawDIUBuilding(); else drawDhakaStation();

    // ⚡ OVERHEAD WIRES DRAWN HERE
    drawOverheadWires();

    drawFullTrain();

    if(showDeboarding) for(int i=0; i<8; i++) if(deboarders[i].y >= -0.25f) drawArticulatedPerson(&deboarders[i]);
    if(showBoarding) for(int i=0; i<8; i++) if(boarders[i].y >= -0.25f) drawArticulatedPerson(&boarders[i]);

    glPushMatrix(); glTranslatef(trainX, 0, 0); drawTrainDoors(0.0f); drawTrainDoors(-1.45f); drawTrainDoors(-2.9f); glPopMatrix();

    drawGradientRect(-1.0, -0.65, 2.0, 0.4, 0.45, 0.45, 0.45, 0.25, 0.25, 0.25);
    for(float px=-1.0f; px<=1.0f; px+=0.2f) { drawRect(px, -0.65f, 0.005f, 0.4f, 0.3f, 0.3f, 0.3f); }

    drawGradientRect(-1.0, -0.26, 2.0, 0.06, 1.0f, 0.85f, 0.0f, 0.8f, 0.65f, 0.0f);
    drawRect(-1.0, -0.26, 2.0, 0.01, 0.2f, 0.2f, 0.2f);
    drawRect(-1.0, -0.21, 2.0, 0.01, 0.2f, 0.2f, 0.2f);

    glColor3f(0.8f, 0.6f, 0.0f);
    for(float tactX=-1.0f; tactX<=1.0f; tactX+=0.02f) {
        glBegin(GL_LINES); glVertex2f(tactX, -0.25f); glVertex2f(tactX, -0.22f); glEnd();
    }

    drawGradientRect(-1.0f, -0.3f, 2.0f, 0.05f, 0.4f, 0.4f, 0.45f, 0.2f, 0.2f, 0.25f);
    drawRect(-1.0f, -0.28f, 2.0f, 0.015f, 0.7f, 0.7f, 0.75f);

    if(showDeboarding) for(int i=0; i<8; i++) if(deboarders[i].y < -0.25f) drawArticulatedPerson(&deboarders[i]);
    if(showBoarding) for(int i=0; i<8; i++) if(boarders[i].y < -0.25f) drawArticulatedPerson(&boarders[i]);

    drawDashboard();
    glutSwapBuffers();
}

void timer(int v) {
    sunAngle += 0.1f;
    for(int i=0; i<10; i++) { cloudX[i] += cloudSpeed[i]; if(cloudX[i] > 2.5f) cloudX[i] = -2.0f; }

    switch(trainState) {
        case 0: trainSpeed = (0.0f - trainX) * 0.015f; if(trainSpeed < 0.005f) trainSpeed = 0.005f; trainX += trainSpeed; wheelAngle -= trainSpeed * 300.0f; if(trainX >= 0.0f) { trainX = 0.0f; trainState = 1; waitTimer = 0; } break;
        case 1: if(++waitTimer > 60) trainState = 2; break;
        case 2: if(doorOffset < 0.15f) doorOffset += 0.003f; else { trainState = 3; showDeboarding = 1; } break;
        case 3: { int allOut = 1; for(int i=0; i<8; i++) { if(deboarders[i].y > -0.38f - (i%2)*0.05f) { deboarders[i].state = 1; deboarders[i].walkTimer += deboarders[i].speed*50; deboarders[i].y -= deboarders[i].speed; allOut = 0; } else if(deboarders[i].x < 1.5f) { deboarders[i].state = 1; deboarders[i].walkTimer += deboarders[i].speed*50; deboarders[i].x += deboarders[i].speed; allOut = 0; } else { deboarders[i].state = 0; } } if(allOut) { trainState = 4; showDeboarding = 0; } } break;
        case 4: { int allIn = 1; for(int i=0; i<8; i++) { float targetX = -0.1f + (i%4) * 0.06f; if(fabs(boarders[i].x - targetX) > 0.02f) { boarders[i].state = 1; boarders[i].walkTimer += boarders[i].speed*50; boarders[i].x -= boarders[i].speed; allIn = 0; } else if(boarders[i].y < -0.15f + (i%2)*0.02f) { boarders[i].state = 1; boarders[i].walkTimer += boarders[i].speed*50; boarders[i].y += boarders[i].speed; allIn = 0; } else { boarders[i].state = 0; } } if(allIn) { showBoarding = 0; trainState = 5; } } break;
        case 5: if(doorOffset > 0.0f) doorOffset -= 0.003f; else { trainState = 6; waitTimer = 0; } break;
        case 6: if(++waitTimer > 60) { trainState = 7; trainSpeed = 0.002f; } break;
        case 7: trainSpeed += 0.0003f; trainX += trainSpeed; wheelAngle -= trainSpeed * 300.0f; if(trainX > 6.0f) { trainX = -6.0f; trainState = 0; currentStation = !currentStation; initPeople(); showBoarding = 1; initEnvironment(); } break;
    }
    glutPostRedisplay(); glutTimerFunc(16, timer, 0);
}

int main(int argc, char** argv) {
    srand(time(NULL)); initEnvironment(); initPeople();
    glutInit(&argc, argv); glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1600, 900);
    glutCreateWindow("Ultra-Premium DIU Smart City & BD MRT Simulator");
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glutDisplayFunc(display); glutTimerFunc(0, timer, 0); glutMainLoop();
    return 0;
}
