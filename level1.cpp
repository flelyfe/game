#include "level1.h"
#include "raylib.h"

level1::level1(const std::string& platformsJson, const std::string& bgImage) {
    allplatforms.LoadFromJSON(platformsJson);
    staticLiquids.LoadFromJSON(platformsJson);
    diamonds.LoadFromJSON(platformsJson);
    background = LoadTexture(bgImage.c_str());

}

void level1::Draw() {
    DrawTexture(background, 0, 0, WHITE);
    staticLiquids.DrawLiquids();
    allplatforms.DrawPlatforms();
    allplatforms.DrawLevers();
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

void level1::CheckDiamondCollisions(const Vector2& player1Pos, const Vector2& player1Size,
                                    const Vector2& player2Pos, const Vector2& player2Size) {
    diamonds.CheckCollisionAndCollect(player1Pos, player1Size, 0);  // 0 = Water
    diamonds.CheckCollisionAndCollect(player2Pos, player2Size, 1);  // 1 = Fire
}

