#include "player.h"
#include "platforms.h"
#include "raylib.h"
#include <cmath>
#include <algorithm>
#include <vector>

const float GRAVITY = 0.6f;
const float JUMP_FORCE = -18.0f;
const float MAX_FALL_SPEED = 25.0f;
const float COLLISION_MARGIN = 3.0f;
const float STEP_SIZE = 0.2f; 
const float SLIDE_ACCELERATION = 0.15f;
const float SLIDE_FRICTION = 0.95f;
const float SURFACE_STICKINESS = 0.8f;
const int JUMP_INPUT_BUFFER = 6;

Player::Player(PlayerType t, Color c, const std::string& n, Vector2 pos, Vector2 sz, Vector2 vel, float spd)
    : color(c), name(n), position(pos), size(sz), velocity(vel), speed(spd),
      isOnGround(false), canJump(true), isDead(false), type(t), jumpInputBuffer(0) {}

static bool PointInPolygon(Vector2 p, const std::vector<Vector2>& poly) 
{
    int count = 0;
    for (size_t i = 0; i < poly.size(); ++i) 
    {
        Vector2 p1 = poly[i];
        Vector2 p2 = poly[(i + 1) % poly.size()];
        if ((p1.y <= p.y && p.y < p2.y) || (p2.y <= p.y && p.y < p1.y)) 
        {
            float xinters = (p2.x - p1.x) * (p.y - p1.y) / (p2.y - p1.y) + p1.x;
            if (p.x < xinters) count++;
        }
    }
    return count % 2 == 1;
}

static Vector2 ClosestPointOnSegment(Vector2 p, Vector2 a, Vector2 b) 
{
    Vector2 ab = {b.x - a.x, b.y - a.y};
    Vector2 ap = {p.x - a.x, p.y - a.y};
    float len2 = ab.x * ab.x + ab.y * ab.y;
    if (len2 < 0.0001f) return a;
    float t = (ap.x * ab.x + ap.y * ab.y) / len2;
    t = std::clamp(t, 0.0f, 1.0f);
    return {a.x + t * ab.x, a.y + t * ab.y};
}

static float Distance(Vector2 a, Vector2 b) 
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

struct EdgeCollision 
{
    bool hasCollision;
    int direction;
    Vector2 normal;
    Vector2 pushPoint;
    float distance;
    Vector2 edgeStart;
    Vector2 edgeEnd;
};

static EdgeCollision CheckEdgeCollision(const Vector2& pos, const Vector2& size, Vector2 p1, Vector2 p2) 
{
    EdgeCollision result = {false, -1, {0, 0}, {0, 0}, 1e9f, p1, p2};
    
    Vector2 center = {pos.x + size.x / 2.0f, pos.y + size.y / 2.0f};
    
    Vector2 edge = {p2.x - p1.x, p2.y - p1.y};
    Vector2 normal = {-edge.y, edge.x};
    
    float normalLen = std::sqrt(normal.x * normal.x + normal.y * normal.y);
    if (normalLen > 0.0001f) 
    {
        normal.x /= normalLen;
        normal.y /= normalLen;
    }
    
    Vector2 closest = ClosestPointOnSegment(center, p1, p2);
    float dist = Distance(center, closest);
    float radius = std::max(size.x, size.y) / 2.0f + COLLISION_MARGIN;
    float pushDist = radius - dist;
    
    if (pushDist <= 0) return result;
    
    result.distance = dist;
    result.pushPoint = closest;
    
    float dotProduct = (center.x - closest.x) * normal.x + (center.y - closest.y) * normal.y;
    
    if (dotProduct < 0) 
    {
        normal.x = -normal.x;
        normal.y = -normal.y;
    }
    
    result.normal = normal;
    result.hasCollision = true;
    
    float angle = std::atan2(normal.y, normal.x);
    
    if (angle > -2.356f && angle < -0.785f) 
    {
        result.direction = 0;
    }
    else if (angle > 0.785f && angle < 2.356f) 
    {
        result.direction = 1;
    }
    else if (angle > 2.356f || angle < -2.356f) 
    {
        result.direction = 2;
    }
    else 
    {
        result.direction = 3;
    }
    
    return result;
}

static int GetBestCollisionDirection(const Vector2& pos, const Vector2& size, const std::vector<Vector2>& poly, Vector2& pushPoint, Vector2& pushNormal, Vector2& edgeStart, Vector2& edgeEnd) 
{
    EdgeCollision bestCollision = {false, -1, {0, 0}, {0, 0}, 1e9f, {0, 0}, {0, 0}};
    
    std::vector<Vector2> corners = {
        {pos.x, pos.y},
        {pos.x + size.x, pos.y},
        {pos.x + size.x, pos.y + size.y},
        {pos.x, pos.y + size.y}
    };
    
    bool anyCornerInside = false;
    for (const auto& corner : corners) 
    {
        if (PointInPolygon(corner, poly)) 
        {
            anyCornerInside = true;
            break;
        }
    }
    
    if (!anyCornerInside) return -1;
    
    for (size_t i = 0; i < poly.size(); ++i) 
    {
        Vector2 p1 = poly[i];
        Vector2 p2 = poly[(i + 1) % poly.size()];
        
        EdgeCollision collision = CheckEdgeCollision(pos, size, p1, p2);
        
        if (collision.hasCollision && collision.distance < bestCollision.distance) 
        {
            bestCollision = collision;
        }
    }
    
    if (!bestCollision.hasCollision) return -1;
    
    pushPoint = bestCollision.pushPoint;
    pushNormal = bestCollision.normal;
    edgeStart = bestCollision.edgeStart;
    edgeEnd = bestCollision.edgeEnd;
    
    return bestCollision.direction;
}


void Player::Update(int leftkey, int rightkey, int upkey, const std::vector<Platform>& platforms, const std::vector<Liquid>& liquids, float screenWidth, float screenHeight) 
{
    if (isDead) return;

    Vector2 moveDir = {0, 0};
    if (IsKeyDown(leftkey)) moveDir.x = -1;
    if (IsKeyDown(rightkey)) moveDir.x = 1;
    velocity.x = moveDir.x * speed;
    
    float newX = position.x + velocity.x;
    float stepX = (velocity.x > 0) ? STEP_SIZE : -STEP_SIZE;
    
    if (velocity.x != 0) 
    {
        while ((velocity.x > 0 && position.x < newX) || (velocity.x < 0 && position.x > newX)) 
        {
            position.x += stepX;
            bool hitWall = false;
            
            for (const auto& plat : platforms) 
            {
                if (plat.type != ShapeType::Polygon) continue;
                
                Vector2 pushPoint;
                Vector2 pushNormal;
                Vector2 edgeStart, edgeEnd;
                int dir = GetBestCollisionDirection(position, size, plat.points, pushPoint, pushNormal, edgeStart, edgeEnd);
                
                if (dir == 2 || dir == 3) 
                {
                    const float STEP_UP_MAX = 8.0f;
                    bool steppedUp = false;
                    float baseY = position.y;
                    
                    for (float h = 1.0f; h <= STEP_UP_MAX; h += 1.0f) 
                    {
                        Vector2 testPos = { position.x, baseY - h };
                        Vector2 tmpPoint;
                        Vector2 tmpNormal;
                        Vector2 tmpEdgeStart, tmpEdgeEnd;
                        int dir2 = GetBestCollisionDirection(testPos, size, plat.points, tmpPoint, tmpNormal, tmpEdgeStart, tmpEdgeEnd);   
                        if (dir2 == 0) 
                        {
                            position = testPos;
                            steppedUp = true;
                            break;
                        }
                    }
                    
                    if (!steppedUp) 
                    {
                        position.x -= stepX;
                        velocity.x = 0;
                        hitWall = true;
                    }
                    break;
                }
            }
            if (hitWall) break;
        }
    }

    velocity.y += GRAVITY;
    if (velocity.y > MAX_FALL_SPEED) velocity.y = MAX_FALL_SPEED;
    
    float newY = position.y + velocity.y;
    float stepY = (velocity.y > 0) ? STEP_SIZE : -STEP_SIZE;
    
    bool wasOnGround = isOnGround;
    isOnGround = false;
    
    bool hitFloor = false;
    Vector2 slideNormal = {0, 0};
    Vector2 slideEdgeStart = {0, 0};
    Vector2 slideEdgeEnd = {0, 0};
    
    if (velocity.y != 0) {
        while ((velocity.y > 0 && position.y < newY) || (velocity.y < 0 && position.y > newY)) 
        {
            position.y += stepY;
            
            for (const auto& plat : platforms) 
            {
                if (plat.type != ShapeType::Polygon) continue;
                
                Vector2 pushPoint;
                Vector2 pushNormal;
                Vector2 edgeStart, edgeEnd;
                int dir = GetBestCollisionDirection(position, size, plat.points, pushPoint, pushNormal, edgeStart, edgeEnd);
                
                if (dir == 0 && velocity.y > 0) 
                {
                    position.y -= stepY;
                    float pullDistance = COLLISION_MARGIN * SURFACE_STICKINESS * 1.5f;
                    position.x += pushNormal.x * pullDistance;
                    position.y += pushNormal.y * pullDistance;
                    
                    velocity.y = 0;
                    isOnGround = true;
                    canJump = true;
                    hitFloor = true;
                    slideNormal = pushNormal;
                    slideEdgeStart = edgeStart;
                    slideEdgeEnd = edgeEnd;
                    break;
                } 
                else if (dir == 1 && velocity.y < 0) 
                {
                    position.y -= stepY;
                    velocity.y = 0;
                    hitFloor = true;
                    break;
                } 
                else if ((dir == 2 || dir == 3) && velocity.y > 0) 
                {
 
                    float pushDistance = std::max(size.x, size.y) * 0.6f + COLLISION_MARGIN;
                    position.x += pushNormal.x * pushDistance;
                    position.y += pushNormal.y * pushDistance;
                    
                    velocity.y = 0;
                    isOnGround = true;
                    canJump = true;
                    hitFloor = true;
                    slideNormal = pushNormal;
                    slideEdgeStart = edgeStart;
                    slideEdgeEnd = edgeEnd;
                    break;
                }
            }
            if (hitFloor) break;
        }
    }

    for (const auto& plat : platforms) 
    {
        if (plat.type != ShapeType::Polygon) continue;
        
        Vector2 pushPoint;
        Vector2 pushNormal;
        Vector2 edgeStart, edgeEnd;
        int dir = GetBestCollisionDirection(position, size, plat.points, pushPoint, pushNormal, edgeStart, edgeEnd);
        
        if (dir >= 0) 
        {
            float pushDistance = std::max(size.x, size.y) / 2.0f + COLLISION_MARGIN + 1.0f;
            position.x += pushNormal.x * pushDistance;
            position.y += pushNormal.y * pushDistance;
            float velDot = velocity.x * pushNormal.x + velocity.y * pushNormal.y;
            if (velDot < 0) 
            {
                velocity.x -= pushNormal.x * velDot;
                velocity.y -= pushNormal.y * velDot;
            }
        }
    }
    

    if (isOnGround && slideNormal.y != 0) 
    {
        Vector2 edgeVector = {slideEdgeEnd.x - slideEdgeStart.x, slideEdgeEnd.y - slideEdgeStart.y};
        float edgeLen = std::sqrt(edgeVector.x * edgeVector.x + edgeVector.y * edgeVector.y);
        
        if (edgeLen > 0.0001f) {
            edgeVector.x /= edgeLen;
            edgeVector.y /= edgeLen;
            
            float isHorizontal = std::abs(edgeVector.y);
            
            if (isHorizontal > 0.2f) 
            {
                if (moveDir.x != 0) 
                {
                    float dotWithEdge = moveDir.x * edgeVector.x + moveDir.x * edgeVector.y;
                    
                    if ((edgeVector.y < 0 && moveDir.x > 0) || (edgeVector.y > 0 && moveDir.x < 0)) 
                    {
                        velocity.x = moveDir.x * speed;
                        velocity.y -= moveDir.x * speed * 0.15f * std::abs(edgeVector.x);
                    } 
                    else 
                    {
                        velocity.x = moveDir.x * speed;
                        velocity.y += moveDir.x * speed * 0.1f * std::abs(edgeVector.x);
                    }
                } 
                velocity.x *= SLIDE_FRICTION;
            }
        }
    }
    

    if (IsKeyPressed(upkey)) 
    {
        jumpInputBuffer = JUMP_INPUT_BUFFER;
    }
    
    if (jumpInputBuffer > 0) 
    {
        jumpInputBuffer--;
    }
    
    if (jumpInputBuffer > 0 && canJump) 
    {
        velocity.y = JUMP_FORCE;
        isOnGround = false;
        canJump = false;
        jumpInputBuffer = 0;
    }
    
    if (!isOnGround) 
    {
        canJump = false;
    }

    LiquidType liquidType = Liquids::CheckLiquidCollision(liquids,position, size);
    if (liquidType != static_cast<LiquidType>(-1)) 
    {
        bool shouldDie = false;
        
        if (type == PlayerType::Water) 
        {
            if (liquidType == LiquidType::Lava || liquidType == LiquidType::Poison) 
            {
                shouldDie = true;
            }
        } 
        else if (type == PlayerType::Fire) 
        {
            if (liquidType == LiquidType::Water || liquidType == LiquidType::Poison) 
            {
                shouldDie = true;
            }
        }
        
        if (shouldDie) 
        {
            isDead = true;
        }
    }
    
    if (position.y < 0 || position.y + size.y > screenHeight) 
    {
        isDead = true;
    }
}

void Player::Draw() 
{
    if (isDead) 
    {
        DrawRectangle(static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(size.x), static_cast<int>(size.y), GRAY);
    } 
    else 
    {
        DrawRectangle(static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(size.x), static_cast<int>(size.y), color);
    }
}