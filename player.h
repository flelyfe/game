#pragma once

#include "raylib.h"
#include <vector>
#include <string>
#include <cmath>

enum class PlayerType {
    Water,
    Fire
};

class Platforms;
class Liquids;
struct Platform;
struct Liquid;
static Vector2 ClosestPointOnSegment(Vector2 point, Vector2 a, Vector2 b);
static float Distance(Vector2 a, Vector2 b);
class Player {
public:
    Color color;
    std::string name;
    Vector2 position;
    Vector2 size;
    Vector2 velocity;
    float speed;
    bool isOnGround;
    bool canJump;
    bool isDead;
    PlayerType type;
    int jumpInputBuffer;

    Player(PlayerType t, Color c, const std::string& n, Vector2 pos, Vector2 sz, Vector2 vel, float spd);
    
    void Update(int leftkey, int rightkey, int upkey,const std::vector<Platform>& platforms, const std::vector<Liquid>& liquids, float screenWidth, float screenHeight);
    
    void Draw();
    bool IsDead() const { return isDead; }
};