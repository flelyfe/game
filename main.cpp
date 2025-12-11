#include "raylib.h"
#include "menu.h"
#include "player.h"
#include "platforms.h"
#include "level1.h"

enum GameScreen { MENU, LEVEL1, END, DEAD, LEVEL_COMPLETE};

int main() {
    const int screenWidth = 2133;
    const int screenHeight = 1600;
    InitWindow(screenWidth, screenHeight, "WaterVasya and LavAlina");
    SetTargetFPS(60);

    InitMenu(); 

    GameScreen currentScreen = MENU;
    level1* map = nullptr; 


    Player water(PlayerType::Water,BLUE, "water", {100, 1400}, {20, 20}, {0, 0}, 4.0f);
    Player fire( PlayerType::Fire,RED, "fire", {200, 1400}, {20, 20}, {0, 0}, 4.0f);
    auto LoadLevel = [&](const char* levelFile, const char* bgFile) {
        if (map) delete map;
        map = new level1(levelFile, bgFile);
        water.position = map->GetWaterSpawnPoint();
        fire.position = map->GetFireSpawnPoint();
        water.velocity = {0, 0};
        fire.velocity = {0, 0};
        water.isDead = false;
        fire.isDead = false;
    };

    while (!WindowShouldClose()) {
        if (currentScreen == MENU) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawMenu();
            EndDrawing();

            int menuResult = Updatemenu();
            if (menuResult == 1) 
            {
                currentScreen = LEVEL1;
                LoadLevel("../../../platforms.json","../../../resources/level1.jpg");
            } 
            else if (menuResult == -1) 
            {
                break;
            }
        }
        else if (currentScreen == LEVEL1 && map) 
        {
    
            map->Update(GetFrameTime());
            map->CheckLeverInteractions(water.position, water.size,fire.position, fire.size);
            map->CheckDiamondCollisions(water.position, water.size, fire.position, fire.size);
            water.Update(KEY_A, KEY_D, KEY_W, map->getPlatforms(), map->getLiquids(), screenWidth, screenHeight);
            fire.Update(KEY_LEFT, KEY_RIGHT, KEY_UP, map->getPlatforms(), map->getLiquids(), screenWidth, screenHeight);
             if (map->CheckLevelComplete(water.position, water.size,
                                       fire.position, fire.size)) {
                currentScreen = LEVEL_COMPLETE;
            }
            if (water.IsDead() || fire.IsDead()) 
            {
                currentScreen = DEAD;
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);
            map->Draw();
            water.Draw();
            fire.Draw();
            EndDrawing();
        }
        else if (currentScreen == LEVEL_COMPLETE) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            map->Draw();
            water.Draw();
            fire.Draw();

            DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 0, 0, 150});
            const char* congratsText = "LEVEL COMPLETE!";
            const char* nextText = "Press SPACE for next level or R for menu";

            int textWidth = MeasureText(congratsText, 60);
            DrawText(congratsText, (screenWidth - textWidth) / 2,
                    screenHeight / 2 - 100, 60, YELLOW);

            int nextWidth = MeasureText(nextText, 20);
            DrawText(nextText, (screenWidth - nextWidth) / 2,
                    screenHeight / 2 + 100, 20, WHITE);

            EndDrawing();

            if (IsKeyPressed(KEY_SPACE)) {
                // TODO: Для уровня 2:
                // LoadLevel("../../../platforms2.json", "../../../resources/level2.jpg");
                // currentScreen = LEVEL1;

                // Пока переходим в меню:
                currentScreen = MENU;
                if (map) delete map;
                map = nullptr;
            } else if (IsKeyPressed(KEY_R)) {
                currentScreen = MENU;
                if (map) delete map;
                map = nullptr;
            }
        }
        else if (currentScreen == DEAD) {
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
