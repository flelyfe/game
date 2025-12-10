#include "raylib.h"
#include "menu.h"
#include "player.h"
#include "platforms.h"
#include "level1.h"

enum GameScreen { MENU, LEVEL1, END, DEAD };

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
    int deathTimer = 0;
    const int deathScreenDuration = 180;

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
                map = new level1("../../../platforms.json", "../../../resources/level1.jpg");
                water.position = {100, 1400};
                fire.position = {200, 1400};
                water.velocity = {0, 0};
                fire.velocity = {0, 0};
                water.isDead = false;
                fire.isDead = false;
                deathTimer = 0;
            } 
            else if (menuResult == -1) 
            {
                break;
            }
        }
        else if (currentScreen == LEVEL1 && map) {
            map->Update(GetFrameTime());
            map->CheckLeverInteractions(water.position, water.size,fire.position, fire.size);
            map->CheckDiamondCollisions(water.position, water.size, fire.position, fire.size);
            water.Update(KEY_A, KEY_D, KEY_W, map->getPlatforms(), map->getLiquids(), screenWidth, screenHeight);
            fire.Update(KEY_LEFT, KEY_RIGHT, KEY_UP, map->getPlatforms(), map->getLiquids(), screenWidth, screenHeight);
            if (water.IsDead() || fire.IsDead()) {
                currentScreen = DEAD;
                deathTimer = 0;
            }
            
            BeginDrawing();
            ClearBackground(RAYWHITE);
            
            map->Draw();
            water.Draw();
            fire.Draw();
            
            EndDrawing();
        }
        else if (currentScreen == DEAD) 
        {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            
            map->Draw();
            water.Draw();
            fire.Draw();
            
            DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 0, 0, 150});
            
            
            DrawText("Press R to return to menu or close window", 
                    (screenWidth - 300) / 2, screenHeight / 2 + 100, 20, WHITE);
            
            EndDrawing();
            
            if (IsKeyPressed(KEY_R)) {
                if (map) delete map;
                map = nullptr;
                currentScreen = MENU;
                DrawMenu();
            }
        }
    }
    
    if (map) delete map;
    CloseMenu();
    CloseWindow();
    
    return 0;
}