#pragma once
#include "platforms.h"
#include <string>

class level1 {
public:
    level1(const std::string& platformsJson, const std::string& bgImage);
    ~level1();

    void Draw();
    void Update(float deltaTime);  
    const std::vector<Platform>& getPlatforms() const { return allplatforms.GetList(); }
    const std::vector<Liquid>& getLiquids() const { return staticLiquids.GetList(); }

    void CheckLeverInteractions(const Vector2& player1Pos, const Vector2& player1Size, const Vector2& player2Pos, const Vector2& player2Size) 
    {
        allplatforms.CheckLeverInteractions(player1Pos, player1Size, player2Pos, player2Size);
    }
    
    void CheckDiamondCollisions(const Vector2& player1Pos, const Vector2& player1Size, const Vector2& player2Pos, const Vector2& player2Size);
    
    bool CheckLevelComplete(const Vector2& waterPos, const Vector2& waterSize, const Vector2& firePos, const Vector2& fireSize) const;

    Vector2 GetWaterSpawnPoint() const { return waterSpawnPoint; }
    Vector2 GetFireSpawnPoint() const { return fireSpawnPoint; }

    const Diamonds& GetDiamonds() const { return diamonds; }
private:
    Platforms allplatforms;
    Texture2D background;
    Liquids staticLiquids;
    Diamonds diamonds;
    Doors levelDoors;
    Vector2 waterSpawnPoint;
    Vector2 fireSpawnPoint;
    void LoadSpawnPositions(const std::string& jsonPath);

};
