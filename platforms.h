#pragma once
#include "raylib.h"
#include <vector>
#include <string>

enum class ShapeType {
    Rectangle,
    Polygon,
};

enum class LiquidType {
    Water,
    Lava,
    Poison,
};

enum DiamondType
{
    Blue,
    Red,
};

struct Platform {
    std::vector<Vector2> points;
    std::vector<Vector2> originalPoints;
    Color color=DARKGRAY;
    ShapeType type;
    bool isMoving=false;
    Vector2 startPos={0,0};
    Vector2 endPos={0,0};
    float speed=20.0f;
    float progress=0.0f;
    bool movingForward=true;
    int linkedLeverId=-1;
    bool isActive=false;
};

struct Lever {
    Vector2 position;
    Vector2 size={117, 99};
    int id = -1;
    bool triggered = false;  
    int triggerCount = 0; 
    std::string texture1;
    std::string texture2;
    Texture2D* cachedTexture1 = nullptr;
    Texture2D* cachedTexture2 = nullptr;

    Lever(Vector2 pos, int leverId, const std::string text1="", const std::string text2="") : position(pos), id(leverId), texture1(text1), texture2(text2) {}
    
    void LoadTextures();
    void UnloadTextures();
    bool CheckCollision(const Vector2& playerPos, const Vector2& playerSize) const;
    void Trigger();
    void Draw() const;
};
struct Liquid {
    std::vector<Vector2> points;
    LiquidType type;
    Color color;
};

struct Diamond
{
    Vector2 position;
    float size=15;
    DiamondType type;
    std::string texture;
    Texture2D* cachedtexture = nullptr;
    bool collected = false;
    void LoadDiamondTexture();
    void UnloadDiamondTexture();
    void DrawDiamond() const;
    Diamond()=default;
    Diamond(Vector2 pos, DiamondType t, const std::string text): position(pos), type(t), texture(text) {} 
};

class Platforms {
public:
    Platforms() = default; 
    bool LoadFromJSON(const std::string& jsonPath);  
    void DrawPlatforms() const;
    void Update(float deltaTime);
    void DrawLevers() const;
    void CheckLeverInteractions(const Vector2& player1Pos, const Vector2& player1Size,
                               const Vector2& player2Pos, const Vector2& player2Size);
    const std::vector<Platform>& GetList() const {return platforms;}
    size_t size() const;
    const std::vector<Lever>& GetLevers() const {return levers;}
private:
    std::vector<Platform> platforms;
    std::vector<Lever> levers;
};

class Liquids {
public:
    Liquids() = default;
    bool LoadFromJSON(const std::string& jsonPath);
    void DrawLiquids() const;
    const std::vector<Liquid>& GetList() const;
    LiquidType CheckCollision(const Vector2& playerPos, const Vector2& playerSize) const;
    static LiquidType CheckLiquidCollision(const std::vector<Liquid>& liquidsVec,
                                          const Vector2& playerPos,
                                          const Vector2& playerSize);

private:
    std::vector<Liquid> liquids;
    bool PointInPolygon(const Vector2& point, const std::vector<Vector2>& polygon) const;
};

class Diamonds
{
    public:
    bool LoadFromJSON(const std::string& jsonPath);
    void DrawDiamonds() const;
    bool CheckCollisionAndCollect(const Vector2& playerPos, const Vector2& playerSize, int playerType);
    const std::vector<Diamond>& GetDiamonds() const { return diamonds; }
    int GetCollectedCount() const;
    int GetCollectedCountByType(DiamondType type) const;
    
private:
    std::vector<Diamond> diamonds;
    bool CheckCircleRectCollision(const Vector2& circlePos, float radius, const Vector2& rectPos, const Vector2& rectSize) const;
};
