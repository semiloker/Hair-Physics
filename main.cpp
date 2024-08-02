#include "raylib.h"
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <thread>
#include <algorithm>

const int winHeight = 1000;
const int winWidth = 1000;
int hairCount = 1;
const int segmentsPerHair = 10;
float segmentLength = 20.0f;

class Hair
{
public:
    Vector2 playerPos = { winWidth / 2, winHeight / 2 };
    float playerRadius = 50.0f;
    std::vector<std::vector<Vector2>> hairSegments;
    std::vector<std::vector<Vector2>> hairVelocities;
    Vector2 prevPlayerPos = { winWidth / 2, winHeight / 2 };
    bool isPlayerVisible = true;
    bool drawYellowLine = false;

    Hair()
    {
        InitHair();
    }

    void InitHair()
    {
        hairSegments.resize(hairCount, std::vector<Vector2>(segmentsPerHair));
        hairVelocities.resize(hairCount, std::vector<Vector2>(segmentsPerHair, { 0, 0 }));

        for (int i = 0; i < hairCount; i++)
        {
            float angle = (2 * PI / hairCount) * i;
            for (int j = 0; j < segmentsPerHair; j++)
            {
                hairSegments[i][j] = { playerPos.x + (playerRadius + j * segmentLength) * cos(angle),
                                       playerPos.y + (playerRadius + j * segmentLength) * sin(angle) };
            }
        }
    }

    void ReinitializeHair()
    {
        InitHair();
    }

    void MousePos(float speed = 3.0f)
    {
        static bool target = false;
        Vector2 mousePos = GetMousePosition();

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            if (CheckCollisionPointCircle(mousePos, playerPos, playerRadius))
            {
                printf("circle tap\n");
                target = true;
            }

            if (target)
            {
                prevPlayerPos = playerPos;
                playerPos.x += (mousePos.x - playerPos.x) / speed;
                playerPos.y += (mousePos.y - playerPos.y) / speed;

                std::cout << "X: " << playerPos.x << " Y: " << playerPos.y << std::endl;
            }
        }
        else
        {
            target = false;
        }

        // Toggle player visibility with the 'Q' key
        if (IsKeyPressed(KEY_Q))
        {
            isPlayerVisible = !isPlayerVisible;
        }
    }

    int gravityMode = 3;
    float gravityValue = 5000;
    std::string windowTitle;

    void UpdateGravityMode()
    {
        if (IsKeyPressed(KEY_ONE))
        {
            gravityMode = 1;
            gravityValue = 5000;
            std::cout << "gravity changed to default (default 5000)\n";
        }
        else if (IsKeyPressed(KEY_TWO))
        {
            gravityMode = 2;
            gravityValue = 250;
            std::cout << "gravity changed to 250 (default 5000)\n";
        }
        else if (IsKeyPressed(KEY_THREE))
        {
            gravityMode = 3;
            gravityValue = 100;
            std::cout << "gravity changed to 100 (default 5000)\n";
        }

        // Toggle drawing yellow line with the 'R' key
        if (IsKeyPressed(KEY_R))
        {
            drawYellowLine = !drawYellowLine;
        }

        windowTitle = "C++ raylib hair physics || Gravity Mode: " + std::to_string(gravityMode) + " : Gravity Value - " + std::to_string(gravityValue) + " || Hair = " + std::to_string(hairCount) + ", segments per one hair = " + std::to_string(segmentsPerHair) + " (" + std::to_string(hairCount * segmentsPerHair) + ")" + " || FPS: " + std::to_string(GetFPS());
        SetWindowTitle(windowTitle.c_str());
    }

    void HairPhysicsThread(int start, int end, float dt)
    {
        for (int i = start; i < end; i++)
        {
            if (i >= hairSegments.size()) continue; // Check for valid index
            float angle = (2 * PI / hairCount) * i;
            Vector2 startPos = { playerPos.x + playerRadius * cos(angle), playerPos.y + playerRadius * sin(angle) };
            hairSegments[i][0] = startPos;

            for (int j = 1; j < segmentsPerHair; j++)
            {
                if (j >= hairSegments[i].size()) continue; // Check for valid index

                Vector2 gravity = { 0, gravityValue * dt };

                hairVelocities[i][j].x += gravity.x;
                hairVelocities[i][j].y += gravity.y;

                Vector2 diff = { hairSegments[i][j - 1].x - hairSegments[i][j].x,
                                 hairSegments[i][j - 1].y - hairSegments[i][j].y };
                float length = sqrt(diff.x * diff.x + diff.y * diff.y);
                float force = (length - segmentLength) * 0.1f;
                Vector2 springForce = { force * (diff.x / length), force * (diff.y / length) };

                hairVelocities[i][j].x += springForce.x;
                hairVelocities[i][j].y += springForce.y;

                hairVelocities[i][j].x *= 0.95f;
                hairVelocities[i][j].y *= 0.95f;

                hairSegments[i][j].x += hairVelocities[i][j].x * dt;
                hairSegments[i][j].y += hairVelocities[i][j].y * dt;

                diff = { hairSegments[i][j - 1].x - hairSegments[i][j].x,
                         hairSegments[i][j - 1].y - hairSegments[i][j].y };
                length = sqrt(diff.x * diff.x + diff.y * diff.y);
                Vector2 correction = { (diff.x / length) * (length - segmentLength),
                                       (diff.y / length) * (length - segmentLength) };

                hairSegments[i][j].x += correction.x;
                hairSegments[i][j].y += correction.y;
            }
        }
    }

    void HairPhysics(float dt)
    {
        int threadsCount = std::max(1, static_cast<int>(std::thread::hardware_concurrency()));
        int hairPerThread = hairCount / threadsCount;
        std::vector<std::thread> threads;

        for (int i = 0; i < threadsCount; i++)
        {
            int start = i * hairPerThread;
            int end = (i == threadsCount - 1) ? hairCount : start + hairPerThread;
            threads.push_back(std::thread(&Hair::HairPhysicsThread, this, start, end, dt));
        }

        for (auto& thread : threads)
        {
            thread.join();
        }
    }

    void DrawHair()
    {
        const float yellowLineLength = 10.0f;

        for (int i = 0; i < hairCount; i++)
        {
            for (int j = 1; j < segmentsPerHair; j++)
            {
                DrawLineV(hairSegments[i][j - 1], hairSegments[i][j], WHITE);

                if (drawYellowLine)
                {
                    Vector2 segmentCenter = hairSegments[i][j];

                    Vector2 lineDirection = { hairSegments[i][j].x - hairSegments[i][j - 1].x, hairSegments[i][j].y - hairSegments[i][j - 1].y };

                    Vector2 perpendicularDirection = { -lineDirection.y, lineDirection.x };

                    float length = sqrt(perpendicularDirection.x * perpendicularDirection.x + perpendicularDirection.y * perpendicularDirection.y);
                    perpendicularDirection.x /= length;
                    perpendicularDirection.y /= length;

                    Vector2 startPos = { segmentCenter.x - perpendicularDirection.x * yellowLineLength / 2, segmentCenter.y - perpendicularDirection.y * yellowLineLength / 2 };
                    Vector2 endPos = { segmentCenter.x + perpendicularDirection.x * yellowLineLength / 2, segmentCenter.y + perpendicularDirection.y * yellowLineLength / 2 };

                    DrawLineV(startPos, endPos, YELLOW);
                }
            }
        }

        if (isPlayerVisible)
        {
            DrawCircleLines((int)playerPos.x, (int)playerPos.y, playerRadius, GREEN);
        }
    }

};

void changeHairCount(Hair& hairPhysicsInstance)
{
    static int state = 0;

    if (IsKeyPressed(KEY_W))
    {
        switch (state)
        {
        case 0:
            hairCount = 1;
            break;
        case 1:
            hairCount = 100;
            break;
        case 2:
            hairCount = 1000;
            break;
        }
        hairPhysicsInstance.ReinitializeHair();
        state = (state + 1) % 3;
    }
}

void changeHairLength(Hair& hairPhysicsInstance)
{
    static int state = 0;

    if (IsKeyPressed(KEY_E))
    {
        switch (state)
        {
        case 0:
            segmentLength = 20.0f;
            break;
        case 1:
            segmentLength = 50.0f;
            break;
        case 2:
            segmentLength = 100.0f;
            break;
        }

        hairPhysicsInstance.ReinitializeHair();
        state = (state + 1) % 3;
    }
}

int main()
{
    InitWindow(winWidth, winHeight, "C++ raylib hair physics");
    SetTargetFPS(0);

    Hair hairPhysicsInstance;
    hairPhysicsInstance.InitHair();

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        BeginDrawing();

        ClearBackground(BLACK);

        changeHairCount(hairPhysicsInstance);
        changeHairLength(hairPhysicsInstance);

        hairPhysicsInstance.MousePos();
        hairPhysicsInstance.HairPhysics(dt);
        hairPhysicsInstance.DrawHair();
        hairPhysicsInstance.UpdateGravityMode();

        DrawText("Q - For circle visible", 5, 0, 20, RED);
        DrawText("W - For the change hair count", 5, 30, 20, RED);
        DrawText("E - For the change hair length", 5, 60, 20, RED);
        DrawText("R - For the change segments per hair", 5, 90, 20, RED);
        DrawText("1 - For gravity = 5000", 5, 120, 20, RED);
        DrawText("2 - For gravity = 250", 5, 150, 20, RED);
        DrawText("3 - For gravity = 100", 5, 180, 20, RED);

        EndDrawing();
    }

    CloseWindow();
}