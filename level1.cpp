#include "level1.h"
#include "raylib.h"
#include "json.hpp"
#include <fstream>


using json = nlohmann::json;

level1::level1(const std::string& platformsJson, const std::string& bgImage) 
{
    allplatforms.LoadFromJSON(platformsJson);
    staticLiquids.LoadFromJSON(platformsJson);
    levelDoors.LoadFromJSON(platformsJson);
    diamonds.LoadFromJSON(platformsJson);
    LoadSpawnPositions(platformsJson);
    
    background = LoadTexture(bgImage.c_str());



}

void level1::Draw() 
{
    DrawTexture(background, 0, 0, WHITE);
    allplatforms.DrawPlatforms();     
    allplatforms.DrawLevers();        
    staticLiquids.DrawLiquids();      
    diamonds.DrawDiamonds();
}

level1::~level1()
{
    UnloadTexture(background);
}

void level1::Update(float deltaTime)
{
    allplatforms.Update(deltaTime);
    
}

void level1::CheckDiamondCollisions(const Vector2& player1Pos, const Vector2& player1Size, const Vector2& player2Pos, const Vector2& player2Size) 
{
    diamonds.CheckCollisionAndCollect(player1Pos, player1Size, 0);  // 0 = Water
    diamonds.CheckCollisionAndCollect(player2Pos, player2Size, 1);  // 1 = Fire
}

void level1::LoadSpawnPositions(const std::string &jsonPath)
{
    std::ifstream file(jsonPath);
    json data;
    file >> data;

    if (data.contains("spawnPositions")) 
    {
        auto& spawn = data["spawnPositions"];
        if (spawn.contains("water")) 
        {
            waterSpawnPoint = {(float)spawn["water"][0], (float)spawn["water"][1]};
        }
        if (spawn.contains("fire")) 
        {
            fireSpawnPoint = {(float)spawn["fire"][0], (float)spawn["fire"][1]};
        }
    }
}

bool level1::CheckLevelComplete(const Vector2& waterPos, const Vector2& waterSize, const Vector2& firePos, const Vector2& fireSize) const 
{
    return levelDoors.CheckBothPlayersAtDoors(waterPos, waterSize, firePos, fireSize);
}
