#include <unistd.h>
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>

// ========================= MACROS =======================
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define CLAMP(x,l,h) ((x) > (h) ? (h) : ((x) < (l) ? (l) : (x)))

  // ===================== CONFIG ====================
float timer;
const char *text;
const int fps              = 60;
const int flags            = FLAG_FULLSCREEN_MODE | FLAG_WINDOW_TOPMOST;
int SCREEN_WIDTH           = 1920;
int SCREEN_HEIGHT          = 1080;
int winscore               = 10;
bool AI                    = true;
const int FONT_SIZE        = 40;
const int PAUSE_FONT_SIZE  = 50; 
float PADDLE_SPEED         = 500;
float deadZone             = 15.0f; 
float ballspeedmod         = 1.20;


int main(void)
{
    // ================= GAME STUFF ===================
    const char *TITLE = "PONG";
    const float PLAY_AREA_TOP = 0.0462f * SCREEN_HEIGHT;

    int leftScore  = 0;
    int rightScore = 0;
    bool paused    = false;
    bool gameover;
    float ballRadius = SCREEN_HEIGHT * 0.03f;

    Rectangle leftPaddle = {
        .x = 0.05f * SCREEN_WIDTH,
        .y = 0.40f * SCREEN_HEIGHT,
        .width  = 0.01f * SCREEN_WIDTH,
        .height = 0.15f * SCREEN_HEIGHT
    };

    Rectangle rightPaddle = {
        .x = 0.95f * SCREEN_WIDTH,
        .y = 0.40f * SCREEN_HEIGHT,
        .width  = 0.01f * SCREEN_WIDTH,
        .height = 0.15f * SCREEN_HEIGHT
    };

    Vector2 ballPos = {
        .x = 0.5f * SCREEN_WIDTH,
        .y = 0.5f * SCREEN_HEIGHT
    };

    Vector2 ballVel = {
        .x = SCREEN_WIDTH / -5.4f,
        .y = 0.0f
    };

    // =================== INIT ===============
    SetConfigFlags(flags);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE);
    SetExitKey(KEY_NULL);
    SetTargetFPS(fps);
    // =================== GAME LOOP ===============
    while (!WindowShouldClose())
    {
        // Toggle pause
        if (IsKeyPressed(KEY_ESCAPE))
            paused = !paused;

        if (IsKeyPressed(KEY_R))
            {
                gameover = false;
                leftScore = 0;
                rightScore = 0;
                ballPos = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
                ballVel = (Vector2){SCREEN_WIDTH / -5.4f, 0};
                leftPaddle.y = 0.40f * SCREEN_HEIGHT;
                rightPaddle.y = 0.40f * SCREEN_HEIGHT;
                text = ("game reset");
                timer = 1.0f;
            }

        if ((!paused) && (!gameover)) 
        {
            float dt = GetFrameTime();

            // ==== INPUT ====
            if (IsKeyDown(KEY_W))
                leftPaddle.y = MAX(PLAY_AREA_TOP, leftPaddle.y - PADDLE_SPEED * dt);

            if (IsKeyDown(KEY_S))
                leftPaddle.y = MIN(SCREEN_HEIGHT - leftPaddle.height, leftPaddle.y + PADDLE_SPEED * dt);
            
            // ===== AI =====
            if (!AI)
            {
                if (IsKeyDown(KEY_UP)) 
                    rightPaddle.y = MAX(PLAY_AREA_TOP, rightPaddle.y - PADDLE_SPEED * dt);

                if (IsKeyDown(KEY_DOWN))
                    rightPaddle.y = MIN(SCREEN_HEIGHT - rightPaddle.height, rightPaddle.y + PADDLE_SPEED * dt);
            }

            else if (AI)
            {   
                if (ballVel.x > 0)
                {
                    float paddleCenter = rightPaddle.y + (rightPaddle.height / 2);
                    if (ballPos.y < paddleCenter - 10) 
                    {
                        float targetY = CLAMP(ballPos.y - rightPaddle.height / 2, PLAY_AREA_TOP, SCREEN_HEIGHT - rightPaddle.height);
                        float movewant = Lerp(rightPaddle.y, targetY, 0.1f) - rightPaddle.y;
                        float move = MAX(movewant, -(PADDLE_SPEED * dt));
                        rightPaddle.y += move;
                    }
                    
                    else if (ballPos.y > paddleCenter + 10)
                    {
                        float targetY = CLAMP(ballPos.y - rightPaddle.height / 2, PLAY_AREA_TOP, SCREEN_HEIGHT - rightPaddle.height);
                        float movewant = Lerp(rightPaddle.y, targetY, 0.1f) - rightPaddle.y;
                        float move = MIN(movewant, PADDLE_SPEED * dt);
                        rightPaddle.y += move;

                    }
                }
                    
            
            }

            // ==== BALL MOVEMENT ====
            ballPos.x += ballVel.x * dt;
            ballPos.y += ballVel.y * dt;

            // ==== PADDLE COLLISION ====
            if (ballVel.x < 0)
            {
                if (CheckCollisionCircleRec(ballPos, ballRadius, leftPaddle))
                {
                    float hit = (ballPos.y - leftPaddle.y) / leftPaddle.height;
                    hit = CLAMP(hit, 0.f, 1.f);

                    float angle = (hit - 0.5f) * PI / 4;
                    float speed = MIN(Vector2Length(ballVel) * ballspeedmod, SCREEN_WIDTH / 3.0f);

                    ballVel.x = speed * cosf(angle);
                    ballVel.y = speed * sinf(angle);
                    ballVel.y += (hit - 0.5f) * 200;
                }
            }
            else
            {
                if (CheckCollisionCircleRec(ballPos, ballRadius, rightPaddle))
                {
                    float hit = (ballPos.y - rightPaddle.y) / rightPaddle.height;
                    hit = CLAMP(hit, 0.f, 1.f);

                    float angle = PI - (hit - 0.5f) * PI / 4;
                    float speed = MIN(Vector2Length(ballVel) * 1.05f, SCREEN_WIDTH / 2.0f);

                    ballVel.x = speed * cosf(angle);
                    ballVel.y = speed * sinf(angle);
                    ballVel.y += (hit - 0.5f) * 200;
                }
            }

            // ==== WALL COLLISION ====
            if (ballVel.y > 0)
            {
                if (ballPos.y >= SCREEN_HEIGHT - ballRadius)
                {
                    ballVel.y *= -1;
                    ballPos.y = SCREEN_HEIGHT - ballRadius;
                }
            }
            else
            {
                if (ballPos.y <= PLAY_AREA_TOP + ballRadius)
                {
                    ballVel.y *= -1;
                    ballPos.y = PLAY_AREA_TOP + ballRadius;
                }
            }

            // ==== SCORING ====
            if (ballPos.x < 0)
            {
                rightScore++;
                ballPos = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
                ballVel = (Vector2){SCREEN_WIDTH / -5.4f, 0};
                leftPaddle.y = 0.40f * SCREEN_HEIGHT;
                rightPaddle.y = 0.40f * SCREEN_HEIGHT;
                if (rightScore >= winscore)
                {
                    gameover = true;
                    leftScore = 0;
                    rightScore = 0;
                    if (!AI)
                    {
                        text = ("RIGHT PLAYER WINS\n\n\npress r to restart");
                    }
                    else
                    {
                        text = ("YOU LOSE\n\n\npress r to restart");
                    }
                }
            }
            else if (ballPos.x > SCREEN_WIDTH)
            {
                leftScore++;
                ballPos = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
                ballVel = (Vector2){SCREEN_WIDTH / -5.4f, 0};
                leftPaddle.y = 0.40f * SCREEN_HEIGHT;
                rightPaddle.y = 0.40f * SCREEN_HEIGHT;
                if (leftScore >= winscore)
                {
                    gameover = true;
                    rightScore = 0;
                    leftScore = 0;
                    {
                    text = ("LEFT PLAYER WINS\n\n\npress r to restart");
                    }
                }

            }
        }

        // ==================== DRAW =====================
        BeginDrawing();
        ClearBackground(BLACK);

        int titleWidth = MeasureText(TITLE, FONT_SIZE);

        // draw stuff
        DrawLine(SCREEN_WIDTH / 2, PLAY_AREA_TOP, SCREEN_WIDTH / 2, SCREEN_HEIGHT, WHITE);
        DrawCircleLines(SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) + PLAY_AREA_TOP, SCREEN_HEIGHT / 10.8, WHITE);
        DrawRectangleRec(leftPaddle, WHITE);
        DrawRectangleRec(rightPaddle, WHITE);
        DrawCircleV(ballPos, ballRadius, WHITE);

        // bar
        DrawLine(0, PLAY_AREA_TOP, SCREEN_WIDTH, PLAY_AREA_TOP, WHITE);
        DrawText(TITLE, (SCREEN_WIDTH - titleWidth) / 2, SCREEN_HEIGHT / 108, FONT_SIZE, WHITE);

        // scores
        char buffer[8];

        sprintf(buffer, "%d", leftScore);
        DrawText(buffer, 0.1f * SCREEN_WIDTH, SCREEN_HEIGHT / 108, FONT_SIZE,
                 (leftScore > rightScore) ? GREEN :
                 (leftScore < rightScore) ? RED : WHITE);

        sprintf(buffer, "%d", rightScore);
        DrawText(buffer, 0.9f * SCREEN_WIDTH, SCREEN_HEIGHT / 108, FONT_SIZE,
                 (rightScore > leftScore) ? GREEN :
                 (rightScore < leftScore) ? RED : WHITE);

        // pause 
        if (paused)
        {
            const char *text_paused = "PAUSED\na to toggle single / multiplayer\n- to decrease paddle speed\n+ to increase paddlespeed\nr to restart game\n1 to decrease winning score winning score\n2 to increase winning score";
            int w = MeasureText(text_paused, PAUSE_FONT_SIZE);

            DrawRectangle(0, PLAY_AREA_TOP, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
            if ((leftScore < winscore) && (rightScore < winscore))
            {
                DrawText(text_paused, (SCREEN_WIDTH - w) / 2, SCREEN_HEIGHT / 4, PAUSE_FONT_SIZE, WHITE);
            }
            if (IsKeyPressed(KEY_A))
            {
                AI = !AI;
                leftScore = 0;
                rightScore = 0;
                ballPos = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
                ballVel = (Vector2){SCREEN_WIDTH / -5.4f, 0};
                leftPaddle.y = 0.40f * SCREEN_HEIGHT;
                rightPaddle.y = 0.40f * SCREEN_HEIGHT;
                char *playnumber = ((AI) < (1) ? ("multi") : ("single"));
                text = (TextFormat("%splayer", playnumber));
                timer = 1.0f;
            }
            if (IsKeyPressed(KEY_EQUAL) && PADDLE_SPEED < 1000)
            {
                PADDLE_SPEED += 100;
                text = (TextFormat("paddle speed is now %.0f (in pixels per second)", PADDLE_SPEED));
                timer = 1.0f;
            }
            if (IsKeyPressed(KEY_MINUS) && PADDLE_SPEED > 100)
            {
                PADDLE_SPEED -= 100;
                text = (TextFormat("paddle speed is now %.0f(in pixels per second)", PADDLE_SPEED));
                timer = 1.0f;
            }
            if (IsKeyPressed(KEY_ONE) && winscore >= 1)
            {
                winscore -= 1;
                text = (TextFormat("winning score is now %d", winscore));
                timer = 1.0f;
            }
            if (IsKeyPressed(KEY_TWO) && winscore <= 100)
            {
                winscore += 1;
                text = (TextFormat("winning score is now %d", winscore));
                timer = 1.0f;
            }
        }
        if (gameover)
        {
            //gameover code here
        }
        if (timer > 0)
        {
            int w = MeasureText(text, PAUSE_FONT_SIZE);
            DrawText(text, (SCREEN_WIDTH - w) / 2, SCREEN_HEIGHT - PAUSE_FONT_SIZE, PAUSE_FONT_SIZE, WHITE); 
            float dt = GetFrameTime();
            timer -= dt;
        }

        EndDrawing();
    
    }
    CloseWindow();
    return 0;
}