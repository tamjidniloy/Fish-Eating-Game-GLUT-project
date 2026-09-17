/*
 QUICK CHANGE GUIDE
 -----------------------------------------------------------------------------------------------------
 1. Window size                    -> Section 1: Config::WINDOW_WIDTH / HEIGHT
 2. Number of fish                 -> Section 1: Config::TOTAL_FISH
 3. Bubble / particle amount        -> Section 1: TOTAL_BUBBLES / MAX_PARTICLES
 4. Score per food fish             -> Section 1: SCORE_PER_FOOD_FISH
 5. Player lives                    -> Section 1: PLAYER_START_LIVES
 6. Player movement speed           -> Section 1: PLAYER_MOVE_SPEED
 7. Level score targets             -> Section 2: LEVEL_MIN_SCORE
 8. Player size per level           -> Section 2: PLAYER_SIZE_BY_LEVEL
 9. Enemy size / speed / chance     -> Section 2: ENEMY_* arrays
10. Food fish speed                 -> Section 2: FOOD_SPEED_BY_LEVEL
11. Fish design                     -> Section 12: drawCartoonFish()
12. Collision rule                  -> Section 14: hasCollisionWithPlayer()
13. Eating / enemy-hit behavior     -> Section 14: handleFoodFishEaten(), handlePlayerHitByEnemy()
14. Screen text / UI                -> Section 19: drawMenuScreen(), drawHUD(), drawGameOverScreen()
15. Keyboard controls               -> Section 20: keyboardDown(), specialKeyDown(), specialKeyUp()
-------------------------------------------------------------------------------------------------------
*/
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <string>

using namespace std;

// ================================================================
// 1. EASY CHANGE PANEL / CONFIGURATION SECTION
// ================================================================
namespace Config
{
    // CHANGE_WINDOW_SIZE_HERE
    // These two values control the game window size.
    // Keep 1100 x 720 for the final submitted version.
    const int WINDOW_WIDTH = 1280;
    const int WINDOW_HEIGHT = 720;

    // Math
    // DO_NOT_CHANGE_UNLESS_NEEDED
    // PI is used for circles and fish wave movement.
    const float PI = 3.1415926535f;

    // CHANGE_OBJECT_COUNT_HERE
    // More fish = busier game. More bubbles/particles = better visual effect.
    // Do not set these too high on a weak computer.
    const int TOTAL_FISH = 13;
    const int TOTAL_BUBBLES = 45;
    const int MAX_PARTICLES = 150;

    // CHANGE_MAX_LEVEL_HERE
    // Current game is designed for 5 levels.
    // If you increase this, you MUST also update every level array in Section 2.
    const int MAX_LEVEL = 5;

    // CHANGE_SCORE_HERE
    // Every food fish gives this amount of score.
    const int SCORE_PER_FOOD_FISH = 5;

    // CHANGE_PLAYER_SETTINGS_HERE
    // PLAYER_START_LIVES       = how many hearts/lives the player starts with.
    // PLAYER_MOVE_SPEED        = movement speed of the player fish.
    // PLAYER_SMOOTHNESS        = how smoothly velocity changes.
    // PLAYER_INVINCIBLE_TIME   = protection time after hitting an enemy.
    const int PLAYER_START_LIVES = 3;
    const float PLAYER_MOVE_SPEED = 255.0f;
    const float PLAYER_SMOOTHNESS = 10.0f;
    const float PLAYER_INVINCIBLE_TIME = 1.35f;

    // CHANGE_PLAYER_BOUNDARY_HERE
    // These values keep the player fish away from the top HUD and sea floor.
    // Increase padding = player has less movement area.
    // Decrease padding = player can move closer to screen edge.
    const float PLAYER_TOP_PADDING = 95.0f;
    const float PLAYER_BOTTOM_PADDING = 100.0f;

    // CHANGE_GAME_SPEED_TIMING_HERE
    // FRAME_TIME_MS = 16 means about 60 FPS.
    // MAX_DELTA_TIME prevents a big movement jump if the computer lags.
    // Beginners should usually keep these unchanged.
    const int FRAME_TIME_MS = 16;
    const float MAX_DELTA_TIME = 0.05f;
}

// 2. LEVEL AND DIFFICULTY DATA
const int LEVEL_MIN_SCORE[Config::MAX_LEVEL + 1] =
    {
        0,
        0,   // Level 1
        100, // Level 2
        201, // Level 3
        301, // Level 4
        400  // Level 5
};

// CHANGE_PLAYER_SIZE_BY_LEVEL_HERE
// Player size by level.
// The player fish grows only when the level changes.
const float PLAYER_SIZE_BY_LEVEL[Config::MAX_LEVEL + 1] =
    {
        0.0f,
        30.0f, // Level 1: Very Small
        35.0f, // Level 2: Small
        40.0f, // Level 3: Small+
        45.0f, // Level 4: Small++
        50.0f  // Level 5: Medium
};

// CHANGE_LEVEL_NAME_TEXT_HERE
// Text label shown in the HUD.
const char *PLAYER_SIZE_LABEL_BY_LEVEL[Config::MAX_LEVEL + 1] =
    {
        "",
        "Very Small",
        "Small",
        "Small+",
        "Medium",
        "Big"};

// CHANGE_ENEMY_SIZE_HERE
// Enemy fish size range by level.
// Enemy fish intentionally stay larger than the player fish.
const float ENEMY_MIN_SIZE_BY_LEVEL[Config::MAX_LEVEL + 1] =
    {
        0.0f,
        45.0f,
        54.0f,
        64.0f,
        76.0f,
        90.0f};

const float ENEMY_MAX_SIZE_BY_LEVEL[Config::MAX_LEVEL + 1] =
    {
        0.0f,
        62.0f,
        74.0f,
        88.0f,
        104.0f,
        122.0f};

// CHANGE_ENEMY_SPEED_HERE
// Enemy fish speed by level.
const float ENEMY_SPEED_BY_LEVEL[Config::MAX_LEVEL + 1] =
    {
        0.0f,
        100.0f,
        200.0f,
        300.0f,
        400.0f,
        500.0f};

// CHANGE_FOOD_SPEED_HERE
// Food fish speed by level.
const float FOOD_SPEED_BY_LEVEL[Config::MAX_LEVEL + 1] =
    {
        0.0f,
        70.0f,
        140.0f,
        210.0f,
        280.0f,
        350.0f};

// CHANGE_ENEMY_SPAWN_CHANCE_HERE
// Enemy spawn chance by level.
// Higher level means more enemies.
const int ENEMY_CHANCE_BY_LEVEL[Config::MAX_LEVEL + 1] =
    {
        0,
        30, // 30%
        40, // 38%
        50, // 47%
        60, // 56%
        70  // 65%
};

// ================================================================
// 3. Basic Data Structures
// ================================================================

// A simple 2D point or movement vector.
// Example: position.x = left/right, position.y = up/down.
// CHANGE_DATA_STRUCTURE_HERE
// Vec2 stores a 2D position or velocity.
// Example: player.position.x means player's x-coordinate.
struct Vec2
{
    float x;
    float y;
};

// RGBA color values.
// r = red, g = green, b = blue, a = transparency.
// Values should stay between 0.0f and 1.0f.
// CHANGE_COLOR_SYSTEM_HERE
// Color uses RGBA values from 0.0 to 1.0.
// r = red, g = green, b = blue, a = transparency/alpha.
struct Color
{
    float r;
    float g;
    float b;
    float a;
};

// Stores all important player fish data in one place.
// CHANGE_PLAYER_DATA_HERE
// Add new player-related data here only if you create a new player feature.
// Example: shieldTimer, powerUpTimer, playerName, etc.
struct Player
{
    Vec2 position; // Current location of the player fish
    Vec2 velocity; // Current movement speed in x and y direction

    float size;            // Current visible size
    float targetSize;      // Size that player is smoothly growing toward
    float invincibleTimer; // Temporary protection after enemy hit

    int direction; // 1 = facing right, -1 = facing left
};

// Stores both food fish and enemy fish.
// isEnemy decides whether the fish is dangerous or edible.
// CHANGE_FISH_DATA_HERE
// Add new fish properties here if you create a new fish type or behavior.
// Example: poison fish, bonus fish, special speed fish.
struct Fish
{
    Vec2 position; // Current fish position

    float baseY;      // Main y-position before wave movement
    float size;       // Fish size
    float speed;      // Horizontal movement speed
    float waveOffset; // Makes each fish wave differently
    float waveAmount; // Up-down wave height

    int direction; // 1 = right, -1 = left

    bool isEnemy; // true = enemy fish, false = food fish

    Color color; // Fish body color
};

// CHANGE_BUBBLE_DATA_HERE
// Bubble is only for background decoration.
// Changing bubble values does not affect score or difficulty.
struct Bubble
{
    Vec2 position;

    float radius;
    float speed;
    float alpha;
};

// CHANGE_PARTICLE_DATA_HERE
// Particle is used for small effects after eating fish or taking damage.
struct Particle
{
    Vec2 position;
    Vec2 velocity;

    float size;
    float life;
    float maxLife;

    bool active;

    Color color;
};

// ================================================================
// 4. Game State
// ================================================================

// CHANGE_SCREEN_STATE_HERE
// Add a new screen here only if you add a completely new page.
// Example: TUTORIAL_SCREEN, SETTINGS_SCREEN, WIN_SCREEN.
enum GameState
{
    MENU_SCREEN,
    PLAYING_SCREEN,
    PAUSED_SCREEN,
    GAME_OVER_SCREEN
};

// ================================================================
// 5. Global Game Data
// ================================================================

// CHANGE_START_SCREEN_HERE
// The game starts from MENU_SCREEN. Change only if you want direct gameplay start.
GameState currentGameState = MENU_SCREEN;

Player player;
Fish fishes[Config::TOTAL_FISH];
Bubble bubbles[Config::TOTAL_BUBBLES];
Particle particles[Config::MAX_PARTICLES];

// CHANGE_INITIAL_GAME_VALUES_HERE
// These are reset during new game. Section 16 controls the reset process.
int score = 0;
int bestScore = 0;
int lives = Config::PLAYER_START_LIVES;
int currentLevel = 1;

float gameTime = 0.0f;
float damageShakeTimer = 0.0f;

int lastFrameTime = 0;

// CHANGE_KEY_STATE_HERE
// Keyboard state.
// We store key states so player movement becomes smooth.
// If you add WASD movement, add new bool variables here.
bool keyLeft = false;
bool keyRight = false;
bool keyUp = false;
bool keyDown = false;

// ================================================================
// 6. Utility Functions
// ================================================================

// HELPER_FUNCTION - usually no need to edit. Creates a Vec2 quickly.
Vec2 makeVec2(float x, float y)
{
    Vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

// CHANGE_COLOR_CREATION_HERE - edit only if you change the color system.
Color makeColor(float r, float g, float b, float a = 1.0f)
{
    Color c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}

// HELPER_FUNCTION - sends our Color structure to OpenGL.
void applyColor(Color color)
{
    glColor4f(color.r, color.g, color.b, color.a);
}

// HELPER_FUNCTION - used for random fish/bubble/particle positions.
float randomFloat(float minValue, float maxValue)
{
    return minValue + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (maxValue - minValue);
}

// HELPER_FUNCTION - used for enemy chance and random choices.
int randomInt(int minValue, int maxValue)
{
    return minValue + rand() % (maxValue - minValue + 1);
}

// HELPER_FUNCTION - keeps values inside a safe minimum and maximum range.
float clampFloat(float value, float minValue, float maxValue)
{
    if (value < minValue)
    {
        return minValue;
    }

    if (value > maxValue)
    {
        return maxValue;
    }

    return value;
}

// HELPER_FUNCTION - used for collision distance checking.
float getDistance(Vec2 a, Vec2 b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;

    return sqrt(dx * dx + dy * dy);
}

// HELPER_FUNCTION - used to show numbers like score and level as text.
string intToString(int value)
{
    stringstream ss;
    ss << value;
    return ss.str();
}

// ================================================================
// 8. Text Drawing Functions
// ================================================================

// TEXT_INTERNAL - low-level text drawing function.
void drawTextRaw(float x, float y, string text, void *font)
{
    glRasterPos2f(x, y);

    for (int i = 0; i < (int)text.length(); i++)
    {
        glutBitmapCharacter(font, text[i]);
    }
}

// CHANGE_TEXT_STYLE_HERE - change default font/shadow behavior here.
void drawText(float x, float y, string text, Color color,
              void *font = GLUT_BITMAP_HELVETICA_18,
              bool shadow = true)
{
    if (shadow)
    {
        glColor4f(0.0f, 0.0f, 0.0f, 0.45f);
        drawTextRaw(x + 2, y - 2, text, font);
    }

    applyColor(color);
    drawTextRaw(x, y, text, font);
}

// ================================================================
// 8. Basic Shape Drawing Functions
//    These functions are reused by UI, fish, hearts, bubbles, etc.
// ================================================================

// SHAPE_HELPER - used by fish, bubbles, hearts, particles.
void drawCircle(float cx, float cy, float radius)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);

    for (int i = 0; i <= 80; i++)
    {
        float angle = 2.0f * Config::PI * i / 80.0f;
        glVertex2f(cx + cos(angle) * radius,
                   cy + sin(angle) * radius);
    }

    glEnd();
}

// SHAPE_HELPER - used mainly for bubble outlines.
void drawCircleOutline(float cx, float cy, float radius)
{
    glBegin(GL_LINE_LOOP);

    for (int i = 0; i < 80; i++)
    {
        float angle = 2.0f * Config::PI * i / 80.0f;
        glVertex2f(cx + cos(angle) * radius,
                   cy + sin(angle) * radius);
    }

    glEnd();
}

// SHAPE_HELPER - used for fish body, rocks, coral.
void drawEllipse(float cx, float cy, float rx, float ry)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);

    for (int i = 0; i <= 100; i++)
    {
        float angle = 2.0f * Config::PI * i / 100.0f;
        glVertex2f(cx + cos(angle) * rx,
                   cy + sin(angle) * ry);
    }

    glEnd();
}

// SHAPE_HELPER - used for fish body outline.
void drawEllipseOutline(float cx, float cy, float rx, float ry)
{
    glBegin(GL_LINE_LOOP);

    for (int i = 0; i < 100; i++)
    {
        float angle = 2.0f * Config::PI * i / 100.0f;
        glVertex2f(cx + cos(angle) * rx,
                   cy + sin(angle) * ry);
    }

    glEnd();
}

// CHANGE_PANEL_AND_UI_BOX_STYLE_HERE - rounded panels/buttons use this.
void drawRoundedRect(float x, float y, float width, float height,
                     float radius, Color color)
{
    applyColor(color);

    // Center rectangles
    glBegin(GL_QUADS);

    glVertex2f(x + radius, y);
    glVertex2f(x + width - radius, y);
    glVertex2f(x + width - radius, y + height);
    glVertex2f(x + radius, y + height);

    glVertex2f(x, y + radius);
    glVertex2f(x + width, y + radius);
    glVertex2f(x + width, y + height - radius);
    glVertex2f(x, y + height - radius);

    glEnd();

    // Corners
    drawCircle(x + radius, y + radius, radius);
    drawCircle(x + width - radius, y + radius, radius);
    drawCircle(x + radius, y + height - radius, radius);
    drawCircle(x + width - radius, y + height - radius, radius);
}

// CHANGE_LIFE_ICON_HERE - edit heart/life drawing here.
void drawHeart(float x, float y, float size, Color color)
{
    applyColor(color);

    drawCircle(x - size * 0.30f, y + size * 0.18f, size * 0.35f);
    drawCircle(x + size * 0.30f, y + size * 0.18f, size * 0.35f);

    glBegin(GL_TRIANGLES);
    glVertex2f(x - size * 0.67f, y + size * 0.10f);
    glVertex2f(x + size * 0.67f, y + size * 0.10f);
    glVertex2f(x, y - size * 0.75f);
    glEnd();
}

// ================================================================
// 9. UI Component Functions
// ================================================================

// CHANGE_BUTTON_STYLE_HERE - edit menu/restart button look here.
void drawButton(float x, float y, float width, float height,
                string label, Color topColor, Color bottomColor)
{
    // Shadow
    drawRoundedRect(x + 5, y - 5, width, height, 16,
                    makeColor(0, 0, 0, 0.28f));

    // Main button gradient
    glBegin(GL_QUADS);

    glColor4f(topColor.r, topColor.g, topColor.b, topColor.a);
    glVertex2f(x + 16, y + height);
    glVertex2f(x + width - 16, y + height);

    glColor4f(bottomColor.r, bottomColor.g, bottomColor.b, bottomColor.a);
    glVertex2f(x + width - 16, y);
    glVertex2f(x + 16, y);

    glEnd();

    // Rounded corners
    applyColor(topColor);
    drawCircle(x + 16, y + height - 16, 16);
    drawCircle(x + width - 16, y + height - 16, 16);

    applyColor(bottomColor);
    drawCircle(x + 16, y + 16, 16);
    drawCircle(x + width - 16, y + 16, 16);

    // Border
    glColor4f(0.03f, 0.16f, 0.30f, 0.95f);
    glLineWidth(2.0f);

    glBegin(GL_LINE_LOOP);
    glVertex2f(x + 16, y);
    glVertex2f(x + width - 16, y);
    glVertex2f(x + width, y + 16);
    glVertex2f(x + width, y + height - 16);
    glVertex2f(x + width - 16, y + height);
    glVertex2f(x + 16, y + height);
    glVertex2f(x, y + height - 16);
    glVertex2f(x, y + 16);
    glEnd();

    glLineWidth(1.0f);

    drawText(x + width * 0.32f, y + height * 0.36f,
             label, makeColor(1, 1, 1, 1),
             GLUT_BITMAP_HELVETICA_18, true);
}

// ================================================================
// 10. Background Drawing Functions
// ================================================================

// CHANGE_OCEAN_BACKGROUND_COLOR_HERE - edit main ocean gradient here.
void drawOceanGradient()
{
    glBegin(GL_QUADS);

    glColor4f(0.10f, 0.74f, 0.98f, 1.0f);
    glVertex2f(0, Config::WINDOW_HEIGHT);

    glColor4f(0.04f, 0.56f, 0.90f, 1.0f);
    glVertex2f(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT);

    glColor4f(0.00f, 0.21f, 0.54f, 1.0f);
    glVertex2f(Config::WINDOW_WIDTH, 0);

    glColor4f(0.01f, 0.40f, 0.74f, 1.0f);
    glVertex2f(0, 0);

    glEnd();
}

// CHANGE_LIGHT_RAYS_HERE - edit/remove underwater light rays here.
void drawUnderwaterLightRays()
{
    for (int i = 0; i < 9; i++)
    {
        float startX = i * 145.0f + sin(gameTime * 0.25f + i) * 20.0f;

        glBegin(GL_TRIANGLES);

        glColor4f(1.0f, 1.0f, 0.95f, 0.12f);
        glVertex2f(startX, Config::WINDOW_HEIGHT);

        glColor4f(1.0f, 1.0f, 0.95f, 0.0f);
        glVertex2f(startX - 90.0f, 90.0f);
        glVertex2f(startX + 100.0f, 90.0f);

        glEnd();
    }
}

// CHANGE_DISTANT_PLANTS_HERE - edit dark background plant silhouettes here.
void drawBackgroundPlantSilhouettes()
{
    for (int i = 0; i < 12; i++)
    {
        float x = 40.0f + i * 100.0f;
        float height = 130.0f + (i % 4) * 30.0f;
        float sway = sin(gameTime * 0.7f + i) * 10.0f;

        glColor4f(0.0f, 0.25f, 0.40f, 0.24f);

        glBegin(GL_TRIANGLES);
        glVertex2f(x - 12.0f, 62.0f);
        glVertex2f(x + 12.0f, 62.0f);
        glVertex2f(x + sway, 62.0f + height);
        glEnd();
    }
}

// CHANGE_SEA_FLOOR_HERE - edit sand height, sand color, and dots here.
void drawSeaFloor()
{
    // Sand
    glBegin(GL_QUADS);

    glColor4f(0.96f, 0.79f, 0.42f, 1.0f);
    glVertex2f(0, 0);
    glVertex2f(Config::WINDOW_WIDTH, 0);

    glColor4f(0.82f, 0.63f, 0.31f, 1.0f);
    glVertex2f(Config::WINDOW_WIDTH, 82);
    glVertex2f(0, 82);

    glEnd();

    // Small sand dots
    for (int i = 0; i < 55; i++)
    {
        float x = (i * 73) % Config::WINDOW_WIDTH;
        float y = 10.0f + (i * 29) % 60;

        glColor4f(0.48f, 0.34f, 0.18f, 0.32f);
        drawCircle(x, y, 1.8f + (i % 3));
    }
}

// CHANGE_PLANTS_ROCKS_CORAL_HERE - edit decorative plants, rocks, coral here.
void drawSeaPlantsAndRocks()
{
    // Green sea plants
    for (int i = 0; i < 12; i++)
    {
        float x = 50.0f + i * 92.0f;
        float height = 75.0f + (i % 4) * 25.0f;
        float sway = sin(gameTime * 1.2f + i) * 8.0f;

        Color plantColor;

        if (i % 3 == 0)
        {
            plantColor = makeColor(0.00f, 0.52f, 0.20f, 0.95f);
        }
        else if (i % 3 == 1)
        {
            plantColor = makeColor(0.00f, 0.68f, 0.33f, 0.95f);
        }
        else
        {
            plantColor = makeColor(0.05f, 0.42f, 0.19f, 0.95f);
        }

        applyColor(plantColor);

        glBegin(GL_TRIANGLES);
        glVertex2f(x - 8.0f, 65.0f);
        glVertex2f(x + 8.0f, 65.0f);
        glVertex2f(x + sway, 65.0f + height);
        glEnd();

        glBegin(GL_TRIANGLES);
        glVertex2f(x + 20.0f, 65.0f);
        glVertex2f(x + 34.0f, 65.0f);
        glVertex2f(x + 27.0f + sway * 0.6f, 65.0f + height * 0.75f);
        glEnd();
    }

    // Coral
    applyColor(makeColor(0.92f, 0.24f, 0.35f, 1.0f));
    drawEllipse(255, 78, 16, 38);
    drawEllipse(276, 76, 13, 28);

    applyColor(makeColor(0.58f, 0.25f, 0.82f, 1.0f));
    drawEllipse(750, 76, 18, 38);
    drawEllipse(775, 74, 13, 28);

    // Rocks
    applyColor(makeColor(0.35f, 0.36f, 0.40f, 1.0f));
    drawEllipse(155, 68, 45, 24);
    drawEllipse(520, 66, 55, 25);
    drawEllipse(955, 68, 48, 24);

    applyColor(makeColor(0.50f, 0.51f, 0.54f, 1.0f));
    drawEllipse(140, 78, 22, 12);
    drawEllipse(505, 78, 24, 12);
    drawEllipse(940, 78, 22, 12);
}

// CHANGE_BACKGROUND_DRAWING_ORDER_HERE - controls which background parts draw first/last.
void drawBackground()
{
    drawOceanGradient();
    drawUnderwaterLightRays();
    drawBackgroundPlantSilhouettes();
    drawSeaFloor();
    drawSeaPlantsAndRocks();
}

// ================================================================
// 11. Bubble System
// ================================================================

// CHANGE_BUBBLE_APPEARANCE_HERE - edit bubble size, speed, opacity here.
void initializeBubble(Bubble &bubble)
{
    bubble.position = makeVec2(randomFloat(0, Config::WINDOW_WIDTH),
                               randomFloat(0, Config::WINDOW_HEIGHT));

    bubble.radius = randomFloat(3, 10);
    bubble.speed = randomFloat(25, 75);
    bubble.alpha = randomFloat(0.18f, 0.55f);
}

// BUBBLE_INTERNAL - creates all bubbles at game start.
void initializeBubbles()
{
    for (int i = 0; i < Config::TOTAL_BUBBLES; i++)
    {
        initializeBubble(bubbles[i]);
    }
}

// CHANGE_BUBBLE_MOVEMENT_HERE - edit bubble floating behavior here.
void updateBubbles(float deltaTime)
{
    for (int i = 0; i < Config::TOTAL_BUBBLES; i++)
    {
        bubbles[i].position.y += bubbles[i].speed * deltaTime;
        bubbles[i].position.x += sin(gameTime + i) * 5.0f * deltaTime;

        if (bubbles[i].position.y > Config::WINDOW_HEIGHT + 20)
        {
            bubbles[i].position.y = -20;
            bubbles[i].position.x = randomFloat(0, Config::WINDOW_WIDTH);
        }
    }
}

// CHANGE_BUBBLE_DRAWING_HERE - edit bubble outline/highlight style here.
void drawBubbles()
{
    for (int i = 0; i < Config::TOTAL_BUBBLES; i++)
    {
        Bubble &bubble = bubbles[i];

        glColor4f(0.80f, 0.96f, 1.0f, bubble.alpha);
        glLineWidth(1.6f);
        drawCircleOutline(bubble.position.x, bubble.position.y, bubble.radius);

        glColor4f(1.0f, 1.0f, 1.0f, bubble.alpha * 0.40f);
        drawCircle(bubble.position.x - bubble.radius * 0.25f,
                   bubble.position.y + bubble.radius * 0.25f,
                   bubble.radius * 0.25f);
    }

    glLineWidth(1.0f);
}

// ================================================================
// 12. Particle System
//     Used for eating effect and damage effect.
// ================================================================

// PARTICLE_INTERNAL - clears old visual effects.
void clearParticles()
{
    for (int i = 0; i < Config::MAX_PARTICLES; i++)
    {
        particles[i].active = false;
    }
}

// CHANGE_SINGLE_PARTICLE_HERE - edit particle speed, size, lifetime here.
void spawnParticle(float x, float y, Color color)
{
    for (int i = 0; i < Config::MAX_PARTICLES; i++)
    {
        if (!particles[i].active)
        {
            particles[i].active = true;
            particles[i].position = makeVec2(x, y);
            particles[i].velocity = makeVec2(randomFloat(-90, 90),
                                             randomFloat(-20, 120));
            particles[i].size = randomFloat(3, 7);
            particles[i].life = randomFloat(0.45f, 0.90f);
            particles[i].maxLife = particles[i].life;
            particles[i].color = color;

            return;
        }
    }
}

// CHANGE_PARTICLE_AMOUNT_HERE - called to create many particles at once.
void createParticleBurst(float x, float y, Color color, int amount)
{
    for (int i = 0; i < amount; i++)
    {
        spawnParticle(x, y, color);
    }
}

// CHANGE_PARTICLE_MOVEMENT_HERE - edit particle gravity/movement here.
void updateParticles(float deltaTime)
{
    for (int i = 0; i < Config::MAX_PARTICLES; i++)
    {
        if (particles[i].active)
        {
            particles[i].life -= deltaTime;

            if (particles[i].life <= 0.0f)
            {
                particles[i].active = false;
            }
            else
            {
                particles[i].position.x += particles[i].velocity.x * deltaTime;
                particles[i].position.y += particles[i].velocity.y * deltaTime;

                // Small gravity-like effect for natural movement.
                particles[i].velocity.y -= 28.0f * deltaTime;
            }
        }
    }
}

// CHANGE_PARTICLE_STYLE_HERE - edit how particles are shown.
void drawParticles()
{
    for (int i = 0; i < Config::MAX_PARTICLES; i++)
    {
        if (particles[i].active)
        {
            float alpha = particles[i].life / particles[i].maxLife;

            Color color = particles[i].color;
            color.a = alpha;

            applyColor(color);
            drawCircle(particles[i].position.x,
                       particles[i].position.y,
                       particles[i].size * alpha);
        }
    }
}

// ================================================================
// 13. Fish Drawing
// ================================================================

// CHANGE_FISH_DESIGN_HERE - edit fish body, tail, eye, fin, mouth, teeth here.
void drawCartoonFish(float x, float y, float size, Color bodyColor,
                     int direction, bool isEnemy, bool isPlayer)
{
    float visibleAlpha = 1.0f;

    // Blinking effect after damage.
    if (isPlayer && player.invincibleTimer > 0.0f)
    {
        visibleAlpha = 0.45f + 0.55f * fabs(sin(gameTime * 12.0f));
    }

    // Shadow
    glColor4f(0.0f, 0.0f, 0.0f, 0.18f * visibleAlpha);
    drawEllipse(x, y - size * 0.78f, size * 1.15f, size * 0.18f);

    Color body = bodyColor;
    body.a = visibleAlpha;

    // Tail
    applyColor(makeColor(body.r * 0.90f,
                         body.g * 0.90f,
                         body.b * 0.90f,
                         visibleAlpha));

    glBegin(GL_TRIANGLES);

    if (direction == 1)
    {
        glVertex2f(x - size * 1.25f, y);
        glVertex2f(x - size * 2.10f, y + size * 0.75f);
        glVertex2f(x - size * 2.10f, y - size * 0.75f);
    }
    else
    {
        glVertex2f(x + size * 1.25f, y);
        glVertex2f(x + size * 2.10f, y + size * 0.75f);
        glVertex2f(x + size * 2.10f, y - size * 0.75f);
    }

    glEnd();

    // Body outline
    glColor4f(0.03f, 0.09f, 0.16f, 0.28f * visibleAlpha);
    glLineWidth(2.5f);
    drawEllipseOutline(x, y, size * 1.36f, size * 0.80f);

    // Main body
    applyColor(body);
    drawEllipse(x, y, size * 1.34f, size * 0.78f);

    // Body highlight
    glColor4f(1.0f, 1.0f, 1.0f, 0.20f * visibleAlpha);
    drawEllipse(x - direction * size * 0.25f,
                y + size * 0.25f,
                size * 0.72f,
                size * 0.20f);

    // Top fin
    applyColor(makeColor(body.r * 0.78f,
                         body.g * 0.78f,
                         body.b * 0.78f,
                         visibleAlpha));

    glBegin(GL_TRIANGLES);
    glVertex2f(x - direction * size * 0.35f, y + size * 0.62f);
    glVertex2f(x + direction * size * 0.28f, y + size * 1.25f);
    glVertex2f(x + direction * size * 0.70f, y + size * 0.48f);
    glEnd();

    // Bottom fin
    glBegin(GL_TRIANGLES);
    glVertex2f(x - direction * size * 0.08f, y - size * 0.46f);
    glVertex2f(x + direction * size * 0.42f, y - size * 1.03f);
    glVertex2f(x + direction * size * 0.72f, y - size * 0.40f);
    glEnd();

    // Eye
    float eyeX = x + direction * size * 0.60f;
    float eyeY = y + size * 0.24f;

    glColor4f(1.0f, 1.0f, 1.0f, visibleAlpha);
    drawCircle(eyeX, eyeY, size * 0.22f);

    glColor4f(0.02f, 0.02f, 0.03f, visibleAlpha);
    drawCircle(eyeX + direction * size * 0.06f,
               eyeY - size * 0.02f,
               size * 0.09f);

    // Expression
    if (isEnemy)
    {
        // Angry eyebrow
        glColor4f(0.02f, 0.02f, 0.03f, visibleAlpha);
        glLineWidth(3.0f);

        glBegin(GL_LINES);
        glVertex2f(eyeX - direction * size * 0.28f,
                   eyeY + size * 0.18f);
        glVertex2f(eyeX + direction * size * 0.25f,
                   eyeY + size * 0.05f);
        glEnd();

        glLineWidth(1.0f);

        // Angry mouth
        glColor4f(0.10f, 0.02f, 0.02f, visibleAlpha);
        drawEllipse(x + direction * size * 1.05f,
                    y - size * 0.12f,
                    size * 0.27f,
                    size * 0.20f);

        // Teeth
        glColor4f(1.0f, 1.0f, 1.0f, visibleAlpha);

        glBegin(GL_TRIANGLES);
        glVertex2f(x + direction * size * 0.95f, y);
        glVertex2f(x + direction * size * 1.08f, y - size * 0.20f);
        glVertex2f(x + direction * size * 1.20f, y);
        glEnd();
    }
    else
    {
        // Friendly smile
        glColor4f(0.04f, 0.04f, 0.05f, 0.72f * visibleAlpha);
        glLineWidth(2.0f);

        glBegin(GL_LINES);
        glVertex2f(x + direction * size * 0.70f, y - size * 0.18f);
        glVertex2f(x + direction * size * 1.03f, y - size * 0.28f);
        glEnd();

        glLineWidth(1.0f);
    }
}

// ================================================================
// 14. Level System Functions
// ================================================================

// CHANGE_LEVEL_RULE_HERE - decides current level from score.
int getLevelFromScore(int currentScore)
{
    if (currentScore >= LEVEL_MIN_SCORE[5])
    {
        return 5;
    }

    if (currentScore >= LEVEL_MIN_SCORE[4])
    {
        return 4;
    }

    if (currentScore >= LEVEL_MIN_SCORE[3])
    {
        return 3;
    }

    if (currentScore >= LEVEL_MIN_SCORE[2])
    {
        return 2;
    }

    return 1;
}

// CHANGE_LEVEL_PROGRESS_BAR_RULE_HERE - controls progress bar percentage.
float getLevelProgress()
{
    int currentStartScore = LEVEL_MIN_SCORE[currentLevel];
    int nextLevelStartScore;

    if (currentLevel < Config::MAX_LEVEL)
    {
        nextLevelStartScore = LEVEL_MIN_SCORE[currentLevel + 1];
    }
    else
    {
        // Level 5 has no next level, so this gives a clean progress effect.
        nextLevelStartScore = LEVEL_MIN_SCORE[currentLevel] + 400;
    }

    float progress =
        (float)(score - currentStartScore) /
        (float)(nextLevelStartScore - currentStartScore);

    return clampFloat(progress, 0.0f, 1.0f);
}

// ================================================================
// 15. Fish Logic Functions
// ================================================================

// CHANGE_FOOD_FISH_COLOR_HERE
// This function randomly chooses one colorful food fish color.
// CHANGE_FOOD_FISH_COLORS_HERE - add/remove food fish color options here.
Color getRandomFoodFishColor()
{
    int option = randomInt(0, 5);

    // CHANGE_COLOR_OPTION_HERE
    // Each option returns one possible color for food fish.
    if (option == 0)
    {
        return makeColor(1.00f, 0.25f, 0.55f, 1.0f);
    }

    if (option == 1)
    {
        return makeColor(1.00f, 0.82f, 0.12f, 1.0f);
    }

    if (option == 2)
    {
        return makeColor(0.22f, 0.86f, 0.32f, 1.0f);
    }

    if (option == 3)
    {
        return makeColor(0.20f, 0.62f, 1.00f, 1.0f);
    }

    if (option == 4)
    {
        return makeColor(0.88f, 0.30f, 0.85f, 1.0f);
    }

    return makeColor(0.40f, 0.95f, 0.95f, 1.0f);
}

// CHANGE_FOOD_FISH_RULE_HERE
// This decides size, speed, color, and safe behavior for food fish.
// CHANGE_FOOD_FISH_LOGIC_HERE - controls food fish size, speed, color.
void setupFoodFish(Fish &fish)
{
    fish.isEnemy = false;

    // Food fish are always safely smaller than player fish.
    // CHANGE_FOOD_SIZE_RANGE_HERE
    // maxFoodSize should stay smaller than player size so food remains edible.
    float maxFoodSize = PLAYER_SIZE_BY_LEVEL[currentLevel] * 0.72f;
    float minFoodSize = 14.0f + currentLevel * 2.0f;

    if (maxFoodSize < minFoodSize + 4.0f)
    {
        maxFoodSize = minFoodSize + 4.0f;
    }

    fish.size = randomFloat(minFoodSize, maxFoodSize);
    // CHANGE_FOOD_RANDOM_SPEED_VARIATION_HERE
    fish.speed = FOOD_SPEED_BY_LEVEL[currentLevel] + randomFloat(-10.0f, 20.0f);
    fish.color = getRandomFoodFishColor();
}

// CHANGE_ENEMY_FISH_RULE_HERE
// This decides size, speed, and color for enemy fish.
// CHANGE_ENEMY_FISH_LOGIC_HERE - controls enemy fish size, speed, color.
void setupEnemyFish(Fish &fish)
{
    fish.isEnemy = true;

    fish.size = randomFloat(ENEMY_MIN_SIZE_BY_LEVEL[currentLevel],
                            ENEMY_MAX_SIZE_BY_LEVEL[currentLevel]);

    // CHANGE_ENEMY_RANDOM_SPEED_VARIATION_HERE
    fish.speed = ENEMY_SPEED_BY_LEVEL[currentLevel] + randomFloat(-15.0f, 28.0f);

    // Purple enemy color.
    fish.color = makeColor(0.48f, 0.12f, 0.78f, 1.0f);
}

// CHANGE_FISH_SPAWN_LOGIC_HERE - controls where fish appear/reappear.
void resetFish(int index, bool randomScreenPosition)
{
    Fish &fish = fishes[index];

    // CHANGE_FOOD_OR_ENEMY_SELECTION_HERE
    // Random number decides whether this fish becomes enemy or food.
    int enemyRoll = randomInt(1, 100);

    if (enemyRoll <= ENEMY_CHANCE_BY_LEVEL[currentLevel])
    {
        setupEnemyFish(fish);
    }
    else
    {
        setupFoodFish(fish);
    }

    fish.direction = (rand() % 2 == 0) ? 1 : -1;
    fish.baseY = randomFloat(120.0f, Config::WINDOW_HEIGHT - 110.0f);
    fish.position.y = fish.baseY;
    // CHANGE_FISH_WAVE_MOVEMENT_HERE
    // waveAmount controls how much fish moves up and down while swimming.
    fish.waveOffset = randomFloat(0, Config::PI * 2.0f);
    fish.waveAmount = randomFloat(5.0f, 20.0f);

    if (randomScreenPosition)
    {
        fish.position.x = randomFloat(0.0f, (float)Config::WINDOW_WIDTH);
    }
    else
    {
        if (fish.direction == 1)
        {
            fish.position.x = -randomFloat(100.0f, 280.0f);
        }
        else
        {
            fish.position.x = Config::WINDOW_WIDTH + randomFloat(100.0f, 280.0f);
        }
    }
}

// CHANGE_LEVEL_UP_REFRESH_HERE - controls which fish refresh after level up.
void refreshSomeFishAfterLevelUp()
{
    for (int i = 0; i < Config::TOTAL_FISH; i++)
    {
        if (i % 2 == 0)
        {
            resetFish(i, false);
        }
    }
}

// CHANGE_EATING_RULE_HERE - decides whether player can eat a fish.
bool isFishEdible(Fish &fish)
{
    if (fish.isEnemy)
    {
        return false;
    }

    // Food fish are edible when they are smaller than player.
    // CHANGE_EDIBLE_SIZE_PERCENT_HERE
    // 0.82f means fish must be clearly smaller than player.
    return fish.size <= player.size * 0.82f;
}

// CHANGE_COLLISION_RULE_HERE
// Increase 0.68f for easier collision, decrease it for stricter collision.
// CHANGE_COLLISION_RULE_HERE - edit collision sensitivity/range here.
bool hasCollisionWithPlayer(Fish &fish)
{
    // CHANGE_COLLISION_SENSITIVITY_HERE
    // Increase 0.68f = easier to collide/eat.
    // Decrease 0.68f = stricter collision.
    float collisionRange = (player.size + fish.size) * 0.68f;
    float distance = getDistance(player.position, fish.position);

    return distance < collisionRange;
}

// CHANGE_EATING_BEHAVIOR_HERE
// This runs when player eats a food fish.
// CHANGE_EAT_REWARD_HERE - score, particle, sound after eating fish.
void handleFoodFishEaten(int fishIndex)
{
    Fish &fish = fishes[fishIndex];

    // Requested rule: every eaten fish gives exactly +5.
    // CHANGE_SCORE_REWARD_HERE
    score += Config::SCORE_PER_FOOD_FISH;

    if (score > bestScore)
    {
        bestScore = score;
    }

    // Eating visual effect and sound.
    createParticleBurst(fish.position.x,
                        fish.position.y,
                        makeColor(1.0f, 0.88f, 0.18f, 1.0f),
                        18); // CHANGE_EAT_PARTICLE_COUNT_HERE

    resetFish(fishIndex, false);
}

// CHANGE_ENEMY_HIT_BEHAVIOR_HERE
// This runs when player touches an enemy fish.
// CHANGE_DAMAGE_RULE_HERE - lives, invincibility, game over after enemy hit.
void handlePlayerHitByEnemy(int fishIndex)
{
    if (player.invincibleTimer > 0.0f)
    {
        return;
    }

    // CHANGE_DAMAGE_AMOUNT_HERE
    // Currently one enemy hit removes one life.
    lives--;

    player.invincibleTimer = Config::PLAYER_INVINCIBLE_TIME;
    damageShakeTimer = 0.25f;

    createParticleBurst(player.position.x,
                        player.position.y,
                        makeColor(1.0f, 0.12f, 0.10f, 1.0f),
                        28); // CHANGE_DAMAGE_PARTICLE_COUNT_HERE

    resetFish(fishIndex, false);

    if (lives <= 0)
    {
        if (score > bestScore)
        {
            bestScore = score;
        }

        currentGameState = GAME_OVER_SCREEN;
    }
}

// CHANGE_LEVEL_UP_BEHAVIOR_HERE - level-up effect and player growth speed.
void updateLevelAndPlayerSizeFinal(float deltaTime)
{
    int oldLevel = currentLevel;
    int newLevel = getLevelFromScore(score);

    if (newLevel != oldLevel)
    {
        currentLevel = newLevel;

        createParticleBurst(player.position.x,
                            player.position.y,
                            makeColor(1.0f, 0.95f, 0.20f, 1.0f),
                            40);

        refreshSomeFishAfterLevelUp();
    }

    player.targetSize = PLAYER_SIZE_BY_LEVEL[currentLevel];

    // CHANGE_PLAYER_GROWTH_SMOOTHNESS_HERE
    // Higher value = player size changes faster after level-up.
    float sizeSmoothness = 3.2f;
    player.size += (player.targetSize - player.size) * sizeSmoothness * deltaTime;
}

// ================================================================
// 16. Game Initialization
// ================================================================

// CHANGE_PLAYER_START_POSITION_HERE - starting position, direction, size.
void resetPlayer()
{
    // CHANGE_PLAYER_INITIAL_POSITION_HERE
    player.position = makeVec2(160.0f, 360.0f);
    player.velocity = makeVec2(0.0f, 0.0f);
    player.direction = 1;

    player.size = PLAYER_SIZE_BY_LEVEL[1];
    player.targetSize = PLAYER_SIZE_BY_LEVEL[1];

    player.invincibleTimer = 0.0f;
}

// CHANGE_NEW_GAME_RESET_HERE - all values reset when a new game starts.
void resetGameData()
{
    score = 0;
    lives = Config::PLAYER_START_LIVES;
    currentLevel = 1;

    damageShakeTimer = 0.0f;

    keyLeft = false;
    keyRight = false;
    keyUp = false;
    keyDown = false;

    resetPlayer();
    clearParticles();

    for (int i = 0; i < Config::TOTAL_FISH; i++)
    {
        resetFish(i, true);
    }
}

// CHANGE_START_GAME_BEHAVIOR_HERE - actions when SPACE/R starts a new game.
void startNewGame()
{
    resetGameData();

    currentGameState = PLAYING_SCREEN;
}

// ================================================================
// 17. Player Update
// ================================================================

// CHANGE_PLAYER_MOVEMENT_HERE
// Arrow key states are converted into smooth player movement here.
// CHANGE_PLAYER_MOVEMENT_HERE - arrow-key movement and screen boundary.
void updatePlayerMovement(float deltaTime)
{
    Vec2 desiredVelocity = makeVec2(0.0f, 0.0f);

    // CHANGE_MOVEMENT_KEY_RESPONSE_HERE
    // These blocks convert pressed keys into target velocity.
    if (keyLeft)
    {
        desiredVelocity.x -= Config::PLAYER_MOVE_SPEED;
        player.direction = -1;
    }

    if (keyRight)
    {
        desiredVelocity.x += Config::PLAYER_MOVE_SPEED;
        player.direction = 1;
    }

    if (keyUp)
    {
        desiredVelocity.y += Config::PLAYER_MOVE_SPEED;
    }

    if (keyDown)
    {
        desiredVelocity.y -= Config::PLAYER_MOVE_SPEED;
    }

    // Smooth velocity interpolation.
    // CHANGE_MOVEMENT_SMOOTHNESS_FORMULA_HERE
    float smoothing = Config::PLAYER_SMOOTHNESS * deltaTime;
    smoothing = clampFloat(smoothing, 0.0f, 1.0f);

    player.velocity.x += (desiredVelocity.x - player.velocity.x) * smoothing;
    player.velocity.y += (desiredVelocity.y - player.velocity.y) * smoothing;

    player.position.x += player.velocity.x * deltaTime;
    player.position.y += player.velocity.y * deltaTime;

    // Keep player inside window.
    // CHANGE_PLAYER_SCREEN_LIMIT_HERE
    player.position.x = clampFloat(player.position.x,
                                   player.size * 2.2f,
                                   Config::WINDOW_WIDTH - player.size * 2.2f);

    player.position.y = clampFloat(player.position.y,
                                   Config::PLAYER_BOTTOM_PADDING,
                                   Config::WINDOW_HEIGHT - Config::PLAYER_TOP_PADDING);

    if (player.invincibleTimer > 0.0f)
    {
        player.invincibleTimer -= deltaTime;
    }
}

// ================================================================
// 18. Fish Update
// ================================================================

// CHANGE_FISH_UPDATE_AND_COLLISION_HERE - fish movement, respawn, collision.
void updateAllFish(float deltaTime)
{
    for (int i = 0; i < Config::TOTAL_FISH; i++)
    {
        Fish &fish = fishes[i];

        // CHANGE_FISH_HORIZONTAL_MOVEMENT_HERE
        // Horizontal movement.
        fish.position.x += fish.direction * fish.speed * deltaTime;

        // CHANGE_FISH_SWIMMING_WAVE_HERE
        // Small wave movement.
        fish.position.y = fish.baseY +
                          sin(gameTime * 1.7f + fish.waveOffset) *
                              fish.waveAmount;

        // Respawn when outside screen.
        if (fish.position.x < -180.0f ||
            fish.position.x > Config::WINDOW_WIDTH + 180.0f)
        {
            resetFish(i, false);
        }

        // CHANGE_COLLISION_RESPONSE_HERE
        // Collision handling.
        if (hasCollisionWithPlayer(fish))
        {
            if (isFishEdible(fish))
            {
                handleFoodFishEaten(i);
            }
            else
            {
                handlePlayerHitByEnemy(i);
            }
        }
    }
}

// ================================================================
// 19. Main Game Update
// ================================================================

// CHANGE_MAIN_GAME_UPDATE_HERE - main per-frame update order.
void updateGame(float deltaTime)
{
    gameTime += deltaTime;

    // CHANGE_UPDATE_ORDER_HERE
    // Decorative updates first, gameplay updates after that.
    updateBubbles(deltaTime);
    updateParticles(deltaTime);

    if (damageShakeTimer > 0.0f)
    {
        damageShakeTimer -= deltaTime;
    }

    if (currentGameState == PLAYING_SCREEN)
    {
        updateLevelAndPlayerSizeFinal(deltaTime);
        updatePlayerMovement(deltaTime);
        updateAllFish(deltaTime);
    }
}

// ================================================================
// 20. HUD and Screen Drawing
// ================================================================

// CHANGE_LEVEL_PROGRESS_UI_HERE - progress bar size, position, color.
void drawLevelProgressBar()
{
    float progress = getLevelProgress();

    drawRoundedRect(405, 665, 280, 18, 9,
                    makeColor(0.0f, 0.0f, 0.0f, 0.28f));

    drawRoundedRect(405, 665, 280.0f * progress, 18, 9,
                    makeColor(1.0f, 0.85f, 0.12f, 0.92f));

    drawText(472, 691, "Level Progress",
             makeColor(1, 1, 1, 0.9f),
             GLUT_BITMAP_HELVETICA_12,
             true);
}

// CHANGE_HUD_UI_HERE - score, best score, level, lives display.
void drawHUD()
{
    // Left HUD panel
    drawRoundedRect(18, 646, 330, 58, 18,
                    makeColor(0.0f, 0.14f, 0.30f, 0.38f));

    // Right HUD panel
    drawRoundedRect(735, 646, 345, 58, 18,
                    makeColor(0.0f, 0.14f, 0.30f, 0.38f));

    drawText(38, 682, "Score: " + intToString(score),
             makeColor(1, 1, 1, 1));

    drawText(38, 657, "Best: " + intToString(bestScore),
             makeColor(1.0f, 0.95f, 0.35f, 1));

    drawText(755, 682, "Level: " + intToString(currentLevel) + " / 5",
             makeColor(1, 1, 1, 1));

    drawText(755, 657,
             string("Size: ") + PLAYER_SIZE_LABEL_BY_LEVEL[currentLevel],
             makeColor(0.82f, 1.0f, 1.0f, 1));

    // Life hearts
    for (int i = 0; i < Config::PLAYER_START_LIVES; i++)
    {
        if (i < lives)
        {
            drawHeart(245 + i * 28, 674, 15,
                      makeColor(1.0f, 0.05f, 0.12f, 1));
        }
        else
        {
            drawHeart(245 + i * 28, 674, 15,
                      makeColor(0.22f, 0.22f, 0.28f, 0.75f));
        }
    }

    drawLevelProgressBar();
}

// CHANGE_MENU_SCREEN_HERE - title, subtitle, buttons, menu text.
void drawMenuScreen()
{
    drawBackground();
    drawBubbles();

    drawRoundedRect(315, 505, 470, 75, 22,
                    makeColor(0.00f, 0.20f, 0.45f, 0.38f));

    // CHANGE_GAME_TITLE_HERE
    drawText(380, 545, "FISH EATING GAME",
             makeColor(1.0f, 0.93f, 0.10f, 1),
             GLUT_BITMAP_TIMES_ROMAN_24,
             true);

    // CHANGE_MENU_SUBTITLE_HERE
    drawText(423, 515, "Ocean Survival",
             makeColor(0.80f, 1.0f, 1.0f, 1));

    drawCartoonFish(545, 420, 52,
                    makeColor(1.0f, 0.55f, 0.05f, 1),
                    1,
                    false,
                    false);

    drawButton(440, 315, 220, 48, "START",
               makeColor(0.40f, 0.88f, 0.18f, 1),
               makeColor(0.16f, 0.60f, 0.12f, 1));

    drawButton(440, 250, 220, 48, "CONTROLS",
               makeColor(0.24f, 0.72f, 1.0f, 1),
               makeColor(0.05f, 0.38f, 0.82f, 1));

    drawButton(440, 185, 220, 48, "EXIT",
               makeColor(1.0f, 0.32f, 0.20f, 1),
               makeColor(0.80f, 0.10f, 0.08f, 1));

    // CHANGE_MENU_INSTRUCTION_TEXT_HERE
    drawText(330, 130,
             "Press SPACE to Start | Arrow Keys to Move | P to Pause | R to Restart | ESC to Exit",
             makeColor(1, 1, 1, 0.95f));
}

// CHANGE_PLAYING_SCREEN_HERE - main gameplay drawing order and help text.
void drawPlayingScreen()
{
    drawBackground();
    drawBubbles();

    // Draw all fish
    for (int i = 0; i < Config::TOTAL_FISH; i++)
    {
        Fish &fish = fishes[i];

        drawCartoonFish(fish.position.x,
                        fish.position.y,
                        fish.size,
                        fish.color,
                        fish.direction,
                        fish.isEnemy,
                        false);
    }

    // Draw player
    drawCartoonFish(player.position.x,
                    player.position.y,
                    player.size,
                    makeColor(1.0f, 0.55f, 0.05f, 1),
                    player.direction,
                    false,
                    true);

    drawParticles();
    drawHUD();

    // CHANGE_PLAYING_HELP_TEXT_HERE
    drawText(345, 23,
             "Eat food fish: +5 score  |  Avoid purple enemy fish  |  5-level survival system",
             makeColor(0.90f, 1.0f, 1.0f, 0.9f),
             GLUT_BITMAP_HELVETICA_12,
             false);
}

// CHANGE_PAUSE_SCREEN_HERE - pause overlay text/design.
void drawPauseOverlay()
{
    drawRoundedRect(0, 0,
                    Config::WINDOW_WIDTH,
                    Config::WINDOW_HEIGHT,
                    0,
                    makeColor(0, 0, 0, 0.35f));

    drawRoundedRect(380, 260, 340, 185, 25,
                    makeColor(0.00f, 0.16f, 0.34f, 0.84f));

    drawText(495, 390, "PAUSED",
             makeColor(1.0f, 0.93f, 0.12f, 1),
             GLUT_BITMAP_TIMES_ROMAN_24,
             true);

    drawText(440, 340, "Press P to Resume",
             makeColor(1, 1, 1, 1));

    drawText(447, 310, "Press R to Restart",
             makeColor(0.85f, 1, 1, 1));
}

// CHANGE_GAME_OVER_SCREEN_HERE - final score/restart/menu screen.
void drawGameOverScreen()
{
    drawBackground();
    drawBubbles();

    drawRoundedRect(350, 200, 400, 330, 28,
                    makeColor(0.00f, 0.10f, 0.24f, 0.82f));

    drawText(446, 465, "GAME OVER",
             makeColor(1.0f, 0.15f, 0.08f, 1),
             GLUT_BITMAP_TIMES_ROMAN_24,
             true);

    drawText(438, 405, "Final Score: " + intToString(score),
             makeColor(1.0f, 0.92f, 0.20f, 1));

    drawText(452, 370, "Best Score: " + intToString(bestScore),
             makeColor(0.82f, 1.0f, 1.0f, 1));

    drawButton(440, 295, 220, 48, "RESTART",
               makeColor(0.40f, 0.88f, 0.18f, 1),
               makeColor(0.16f, 0.60f, 0.12f, 1));

    drawButton(440, 230, 220, 48, "MENU",
               makeColor(0.24f, 0.72f, 1.0f, 1),
               makeColor(0.05f, 0.38f, 0.82f, 1));

    drawText(405, 175,
             "Press R to Restart | Press ESC to Exit",
             makeColor(1, 1, 1, 0.95f));
}

// ================================================================
// 21. GLUT Callback Functions
// ================================================================

// GLUT_DISPLAY_FUNCTION - draws the correct screen every frame.
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Damage screen shake
    if (damageShakeTimer > 0.0f &&
        currentGameState == PLAYING_SCREEN)
    {
        float power = damageShakeTimer * 12.0f;
        glTranslatef(randomFloat(-power, power),
                     randomFloat(-power, power),
                     0);
    }

    if (currentGameState == MENU_SCREEN)
    {
        drawMenuScreen();
    }
    else if (currentGameState == PLAYING_SCREEN)
    {
        drawPlayingScreen();
    }
    else if (currentGameState == PAUSED_SCREEN)
    {
        drawPlayingScreen();
        drawPauseOverlay();
    }
    else if (currentGameState == GAME_OVER_SCREEN)
    {
        drawGameOverScreen();
    }

    glutSwapBuffers();
}

// GLUT_TIMER_FUNCTION - controls update speed and redraw loop.
void timer(int value)
{
    int currentTime = glutGet(GLUT_ELAPSED_TIME);

    float deltaTime = (currentTime - lastFrameTime) / 1000.0f;
    lastFrameTime = currentTime;

    // Prevent large time jump if system lags.
    if (deltaTime > Config::MAX_DELTA_TIME)
    {
        deltaTime = Config::MAX_DELTA_TIME;
    }

    updateGame(deltaTime);

    glutPostRedisplay();
    glutTimerFunc(Config::FRAME_TIME_MS, timer, 0);
}

// CHANGE_KEYBOARD_CONTROLS_HERE
// Normal keyboard controls: SPACE, P, R, ESC.
// CHANGE_KEYBOARD_CONTROLS_HERE - SPACE, P, R, ESC behavior.
void keyboardDown(unsigned char key, int x, int y)
{
    if (key == 27) // ESC
    {
        exit(0);
    }

    if (key == ' ')
    {
        if (currentGameState == MENU_SCREEN)
        {
            startNewGame();
        }
    }

    if (key == 'p' || key == 'P')
    {
        if (currentGameState == PLAYING_SCREEN)
        {
            currentGameState = PAUSED_SCREEN;
        }
        else if (currentGameState == PAUSED_SCREEN)
        {
            currentGameState = PLAYING_SCREEN;
        }
    }

    if (key == 'r' || key == 'R')
    {
        startNewGame();
    }
}

// CHANGE_ARROW_KEY_PRESS_HERE - arrow key press behavior.
void specialKeyDown(int key, int x, int y)
{
    if (key == GLUT_KEY_LEFT)
    {
        keyLeft = true;
    }

    if (key == GLUT_KEY_RIGHT)
    {
        keyRight = true;
    }

    if (key == GLUT_KEY_UP)
    {
        keyUp = true;
    }

    if (key == GLUT_KEY_DOWN)
    {
        keyDown = true;
    }
}

// CHANGE_ARROW_KEY_RELEASE_HERE - arrow key release behavior.
void specialKeyUp(int key, int x, int y)
{
    if (key == GLUT_KEY_LEFT)
    {
        keyLeft = false;
    }

    if (key == GLUT_KEY_RIGHT)
    {
        keyRight = false;
    }

    if (key == GLUT_KEY_UP)
    {
        keyUp = false;
    }

    if (key == GLUT_KEY_DOWN)
    {
        keyDown = false;
    }
}

// ================================================================
// 22. OpenGL Initialization
// ================================================================

// CHANGE_OPENGL_SETUP_HERE - projection, transparency, line smoothing.
void initializeOpenGL()
{
    glClearColor(0.02f, 0.45f, 0.78f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, Config::WINDOW_WIDTH, 0, Config::WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Enable alpha transparency.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Smoother lines.
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

// ================================================================
// 23. MAIN FUNCTION
// ---------------------------------------------------------------
// Program starts here.
// Order of work:
// 1. Start GLUT
// 2. Create window
// 3. Initialize OpenGL
// 4. Initialize game objects
// 5. Register callback functions
// 6. Start GLUT main loop
// ================================================================
// CHANGE_PROGRAM_STARTUP_HERE - window title, startup setup, callback registration.
int main(int argc, char **argv)
{
    srand((unsigned int)time(0));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT);
    glutInitWindowPosition(70, 25);
    // CHANGE_WINDOW_TITLE_HERE
    glutCreateWindow("Fish Eating Game - Clean Structured GLUT Version");
    initializeOpenGL();
    initializeBubbles();
    clearParticles();
    resetGameData();
    lastFrameTime = glutGet(GLUT_ELAPSED_TIME);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardDown);
    glutSpecialFunc(specialKeyDown);
    glutSpecialUpFunc(specialKeyUp);
    glutTimerFunc(Config::FRAME_TIME_MS, timer, 0);
    glutMainLoop();
    return 0;
}
