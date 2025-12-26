#include "raylib.h"
#include "menu.h"
#include "player.h"
#include "platforms.h"
#include "level1.h"

enum GameScreen { MENU, LEVEL1, DEAD, LEVEL_COMPLETE, LEVEL2};

int main() {
    const int screenWidth = 2133;
    const int screenHeight = 1600;
    InitWindow(screenWidth, screenHeight, "WaterVasya and LavAlina");
    SetTargetFPS(60);

    InitMenu(); 

    GameScreen currentScreen = MENU;
    GameScreen lastLevelScreen = LEVEL1;
    level1* map = nullptr; 
    Player water(PlayerType::Water,BLUE, "water", {100, 1400}, {20, 20}, {0, 0}, 4.0f);
    Player fire( PlayerType::Fire,RED, "fire", {200, 1400}, {20, 20}, {0, 0}, 4.0f);
    auto LoadLevel = [&](const char* levelFile, const char* bgFile) 
    {
        if (map) delete map;
        map = new level1(levelFile, bgFile);
        water.position = map->GetWaterSpawnPoint();
        fire.position = map->GetFireSpawnPoint();
        water.velocity = {0, 0};
        fire.velocity = {0, 0};
        water.isDead = false;
        fire.isDead = false;
    };
    while (!WindowShouldClose()) 
    {
        if ((currentScreen == LEVEL1 || currentScreen==LEVEL2 ) && map) 
        {
            map->Update(GetFrameTime());
            map->CheckLeverInteractions(water.position, water.size,fire.position, fire.size);
            map->CheckDiamondCollisions(water.position, water.size, fire.position, fire.size);
            water.Update(KEY_A, KEY_D, KEY_W, map->getPlatforms(), map->getLiquids(), screenWidth, screenHeight);
            fire.Update(KEY_LEFT, KEY_RIGHT, KEY_UP, map->getPlatforms(), map->getLiquids(), screenWidth, screenHeight);
        
            if (map->CheckLevelComplete(water.position, water.size, fire.position, fire.size)) 
            {
                lastLevelScreen = currentScreen; 
                currentScreen = LEVEL_COMPLETE;
            }
            else if (map->IsTimedOut()) 
            {
                currentScreen = DEAD;
            }
            else if (water.IsDead() || fire.IsDead()) 
            {
                currentScreen = DEAD;
            }
        }
        if (currentScreen == MENU) 
        {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawMenu();
            EndDrawing();

            int menuResult = Updatemenu();
            if (menuResult == 1) 
            {
                currentScreen = LEVEL1;
                lastLevelScreen = currentScreen; 
                LoadLevel("../../../platforms.json","../../../resources/level11.jpg");
            } 
            else if (menuResult == -1) 
            {
                break;
            }
        }
        if (currentScreen == LEVEL1 && map) 
        {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            map->Draw();
            water.Draw();
            fire.Draw();

            float remainingTime = map->GetLevelTimeLimit() - map->GetLevelTime();
            int displayTime = (int)std::max(0.0f, remainingTime);
            std::string timerText = "Time: " + std::to_string(displayTime) + "s";

            Color timerColor = WHITE;
            DrawText(timerText.c_str(), 990, 50, 40, timerColor);

            EndDrawing();
        }
        if (currentScreen == LEVEL2 && map) 
        {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            map->Draw();
            water.Draw();
            fire.Draw();
            float remainingTime = map->GetLevelTimeLimit() - map->GetLevelTime();
            int displayTime = (int)std::max(0.0f, remainingTime);
            std::string timerText = "Time: " + std::to_string(displayTime) + "s";
            Color timerColor = WHITE;
            DrawText(timerText.c_str(), 990, 50, 40, timerColor);
            EndDrawing();
        }
        if (currentScreen == LEVEL_COMPLETE) 
        {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            map->Draw();
            water.Draw();
            fire.Draw();

            DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 0, 0, 150});
            const char* congratsText = "LEVEL COMPLETE!";
            const char* nextText = "Press C for next level or R for menu";

            int textWidth = MeasureText(congratsText, 60);
            DrawText(congratsText, (screenWidth - textWidth) / 2,
                    screenHeight / 2 - 100, 60, YELLOW);

            int nextWidth = MeasureText(nextText, 20);
            DrawText(nextText, (screenWidth - nextWidth) / 2,
                    screenHeight / 2 + 100, 20, WHITE);

            EndDrawing();

            if (IsKeyPressed(KEY_C)) 
            {
                if (lastLevelScreen ==LEVEL1) 
                {
                    currentScreen = LEVEL2;
                    LoadLevel("../../../platforms2.json", "../../../resources/level22.jpg");
                } 
                else if (lastLevelScreen == LEVEL2) 
                {
                    currentScreen = MENU;
                    if (map) delete map;
                    map = nullptr;
                }
            } 
            else if (IsKeyPressed(KEY_R)) 
            {
                currentScreen = MENU;
                if (map) delete map;
                map = nullptr;
            }
        }
        if (currentScreen == DEAD) 
        {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            map->Draw();
            water.Draw();
            fire.Draw();

            DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 0, 0, 150});

            const char* deathText = "YOU DIED!";
            const char* retryText = "Press R to return to menu or close window";

            int deathWidth = MeasureText(deathText, 60);
            DrawText(deathText, (screenWidth - deathWidth) / 2,
                    screenHeight / 2 - 50, 60, RED);

            int retryWidth = MeasureText(retryText, 20);
            DrawText(retryText, (screenWidth - retryWidth) / 2,
                    screenHeight / 2 + 100, 20, WHITE);

            EndDrawing();

            if (IsKeyPressed(KEY_R)) {
                currentScreen = MENU;
                if (map) delete map;
                map = nullptr;
            }
        }
    }

    if (map) delete map;
    CloseMenu();
    CloseWindow();

    return 0;
}
