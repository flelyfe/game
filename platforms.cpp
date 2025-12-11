#include "platforms.h"
#include <fstream>
#include <algorithm>
#include <cmath>
#include "json.hpp"

using json = nlohmann::json;

void Lever::LoadTextures() 
{
    if (!texture1.empty()) 
    {
        cachedTexture1 = new Texture2D(LoadTexture(texture1.c_str()));
    }
    if (!texture2.empty()) 
    {
        cachedTexture2 = new Texture2D(LoadTexture(texture2.c_str()));
    }
}

void Lever::UnloadTextures() 
{
    if (cachedTexture1) 
    {
        UnloadTexture(*cachedTexture1);
        delete cachedTexture1;
        cachedTexture1 = nullptr;
    }
    if (cachedTexture2) 
    {
        UnloadTexture(*cachedTexture2);
        delete cachedTexture2;
        cachedTexture2 = nullptr;
    }
}


bool Lever::CheckCollision(const Vector2& playerPos, const Vector2& playerSize) const 
{
    Rectangle playerRect = {playerPos.x, playerPos.y, playerSize.x, playerSize.y};
    Rectangle leverRect = {position.x, position.y, size.x, size.y};
    return CheckCollisionRecs(playerRect, leverRect);
}

void Lever::Trigger() 
{
    triggerCount++;  
    triggered = (triggerCount % 2 == 1);
}

void Lever::Draw() const 
{
    if (triggered && cachedTexture2) 
    {
        DrawTextureV(*cachedTexture2, position, WHITE);
    } 
    else if (!triggered && cachedTexture1) 
    {
        DrawTextureV(*cachedTexture1, position, WHITE);
    }
}

bool Platforms::LoadFromJSON(const std::string& jsonPath) 
{
    std::ifstream file(jsonPath);
    if (!file.is_open()) return false;
    
    json data;
    file >> data;
    platforms.clear();
    
    if (data.contains("platforms"))
    {
        for (auto& platData : data["platforms"]) 
        {
            Platform plt;
            plt.type = ShapeType::Polygon; 
            plt.isActive = false;  
            plt.progress = 0.0f; 
            for (auto& p : platData["points"]) 
            {
                plt.points.push_back({(float)p[0], (float)p[1]});
            }
            plt.originalPoints = plt.points;
            if (platData.contains("moving")&& platData["moving"].get<bool>())
            {
                plt.isMoving=true;
                plt.startPos={(float)platData["startPos"][0],(float)platData["startPos"][1]};
                plt.endPos={(float)platData["endPos"][0],(float)platData["endPos"][1]};
                plt.linkedLeverId=platData.value("leverId", -1);
            }
            plt.isActive = false;
            platforms.push_back(plt);
        }
        
    }

    if (data.contains("levers"))
    {
        int id=0;
        for (const auto& leverData : data["levers"]) 
        {
            Vector2 pos = {(float)leverData["position"][0], (float)leverData["position"][1]};
            int leverId = leverData.value("id", id);
            std::string texture1=leverData.value("texture1", texture1);
            std::string texture2=leverData.value("texture2", texture2);
            levers.emplace_back(pos, leverId, std::string(texture1), std::string(texture2));
            levers.back().LoadTextures();
            id++;
        }
    }
    
    return true;
}

void Platforms::Update(float deltaTime) 
{
    for (auto& plat : platforms) 
    {
        if (!plat.isMoving || !plat.isActive) continue;
        
        float dx = plat.endPos.x - plat.startPos.x;
        float dy = plat.endPos.y - plat.startPos.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance < 1.0f) continue;
        
        float speedStep = (deltaTime * plat.speed) / distance;
        
        if (plat.movingForward) 
        {
            plat.progress += speedStep;
            if (plat.progress >= 1.0f) 
            {
                plat.progress = 1.0f;
                plat.isActive = false;
            }
        } 
        else 
        {
            plat.progress -= speedStep;
            if (plat.progress <= 0.0f) 
            {
                plat.progress = 0.0f;
                plat.isActive = false;
            }
        }
        
        float offsetX = dx * plat.progress;
        float offsetY = dy * plat.progress;
        
        for (size_t i = 0; i < plat.originalPoints.size(); ++i) 
        {
            plat.points[i].x = plat.originalPoints[i].x + offsetX;
            plat.points[i].y = plat.originalPoints[i].y + offsetY;
        }
    }
}

void Platforms::DrawPlatforms() const
{
    for (size_t idx = 0; idx < platforms.size(); ++idx)
    {
        const auto& plat = platforms[idx];

        if (plat.isMoving && plat.points.size() == 4)  
        {
            float minX = plat.points[0].x;
            float maxX = plat.points[0].x;
            float minY = plat.points[0].y;
            float maxY = plat.points[0].y;
            
            for (const auto& p : plat.points)
            {
                minX = std::min(minX, p.x);
                maxX = std::max(maxX, p.x);
                minY = std::min(minY, p.y);
                maxY = std::max(maxY, p.y);
            }
            
            float width = maxX - minX;
            float height = maxY - minY;
            DrawRectangle((int)minX, (int)minY, (int)width, (int)height, DARKGRAY);
        }
        
    }
}

void Platforms::DrawLevers() const 
{
    for (const auto& lever : levers) 
    {
        lever.Draw();
    }
}
size_t Platforms::size() const 
{
    return platforms.size();
}

void Platforms::CheckLeverInteractions(const Vector2& player1Pos, const Vector2& player1Size, const Vector2& player2Pos, const Vector2& player2Size) 
{
    for (auto& lever : levers) 
    {
        bool player1Hit = lever.CheckCollision(player1Pos, player1Size);
        bool player2Hit = lever.CheckCollision(player2Pos, player2Size);

        static int lastTriggeredId = -999;
        
        if ((player1Hit || player2Hit) && lastTriggeredId != lever.id) 
        {
            lever.Trigger();
            lastTriggeredId = lever.id;
            
            for (auto& plat : platforms) 
            {
                if (plat.isMoving && plat.linkedLeverId == lever.id) 
                {
                    if (lever.triggerCount % 2 == 1) 
                    {
                        plat.movingForward = true;
                        plat.isActive = true;
                    } 
                    else 
                    {
                        plat.movingForward = false;
                        plat.isActive = true;
                    }
                }
            }
        }
        
        if (!player1Hit && !player2Hit && lastTriggeredId == lever.id) 
        {
            lastTriggeredId = -999;
        }
    }
}


bool Liquids::LoadFromJSON(const std::string& jsonPath) 
{
    std::ifstream file(jsonPath);
    if (!file.is_open()) return false;
    
    json data;
    file >> data;
    liquids.clear();
    
    if (!data.contains("liquids")) return true;
    
    for (auto& liqData : data["liquids"]) 
    {
        std::vector<Vector2> pts;
        std::string typeStr = liqData["type"];
        
        LiquidType type = LiquidType::Water;
        if (typeStr == "water") type = LiquidType::Water;
        else if (typeStr == "lava") type = LiquidType::Lava;
        else if (typeStr == "poison") type = LiquidType::Poison;
        for (auto& p : liqData["points"]) 
        {
            pts.push_back({(float)p[0], (float)p[1]});
        }
        
        if (!pts.empty()) 
        {
            liquids.push_back({pts, type});
        }

    }
    
    return true;
}


void Liquids::DrawLiquids() const 
{

    std::vector<std::pair<float, const Liquid*>> sortedLiquids;
    
    for (const auto& liq : liquids) 
    {
        float minY = 1e9f;
        for (const auto& p : liq.points) 
        {
            minY = std::min(minY, p.y);
        }
        sortedLiquids.push_back({minY, &liq});
    }
    
    std::sort(sortedLiquids.begin(), sortedLiquids.end());

    for (const auto& [minY, liq] : sortedLiquids) 
    {
        if (liq->points.size() >= 3) 
        {
            
            Vector2 centerPoint = {0, 0};
            for (const auto& p : liq->points) 
            {
                centerPoint.x += p.x;
                centerPoint.y += p.y;
            }
            centerPoint.x /= liq->points.size();
            centerPoint.y /= liq->points.size();
            
            
            for (size_t i = 0; i < liq->points.size(); ++i) 
            {
                Vector2 p1 = liq->points[i];
                Vector2 p2 = liq->points[(i + 1) % liq->points.size()];
                DrawTriangle(centerPoint, p1, p2, liq->color);
            }
            
            
            for (size_t i = 0; i < liq->points.size(); ++i) 
            {
                Vector2 p1 = liq->points[i];
                Vector2 p2 = liq->points[(i + 1) % liq->points.size()];
                Color outlineColor = {liq->color.r, liq->color.g, liq->color.b, 255};
                DrawLine(static_cast<int>(p1.x), static_cast<int>(p1.y), static_cast<int>(p2.x), static_cast<int>(p2.y), outlineColor);
            }
        }
    }
}

const std::vector<Liquid>& Liquids::GetList() const 
{
    return liquids;
}

bool Liquids::PointInPolygon(const Vector2& point, const std::vector<Vector2>& polygon) const 
{
    int n = polygon.size();
    if (n < 3) return false;
    
    int count = 0;
    for (int i = 0; i < n; ++i) 
    {
        Vector2 p1 = polygon[i];
        Vector2 p2 = polygon[(i + 1) % n];
        
        if ((p1.y <= point.y && point.y < p2.y) || (p2.y <= point.y && point.y < p1.y)) 
        {
            float xinters = (p2.x - p1.x) * (point.y - p1.y) / (p2.y - p1.y) + p1.x;
            if (point.x < xinters) count++;
        }
    }
    
    return count % 2 == 1;
}

LiquidType Liquids::CheckCollision(const Vector2& playerPos, const Vector2& playerSize) const 
{
    Vector2 playerCenter = {playerPos.x + playerSize.x / 2.0f, playerPos.y + playerSize.y / 2.0f};
    
    for (const auto& liq : liquids) 
    {
        if (PointInPolygon(playerCenter, liq.points)) 
        {
            return liq.type;
        }
    }
    
    return static_cast<LiquidType>(-1);
}

LiquidType Liquids::CheckLiquidCollision(const std::vector<Liquid>& liquidsVec, const Vector2& playerPos, const Vector2& playerSize) 
{
    Vector2 playerCenter = {playerPos.x + playerSize.x / 2.0f, playerPos.y + playerSize.y / 2.0f};
    
    for (const auto& liq : liquidsVec) 
    {
        Liquids temp;
        if (temp.PointInPolygon(playerCenter, liq.points)) 
        {
            return liq.type;
        }
    }
    return static_cast<LiquidType>(-1);
}
void Diamond::LoadDiamondTexture()
{
    if (!texture.empty()) 
    {
        cachedtexture = new Texture2D(LoadTexture(texture.c_str()));
    }
}

void Diamond::UnloadDiamondTexture() 
{
    if (cachedtexture) 
    {
        UnloadTexture(*cachedtexture);
        delete cachedtexture;
        cachedtexture = nullptr;
    }
}
bool Diamonds::LoadFromJSON(const std::string& jsonPath)
{
    std::ifstream file(jsonPath);
    if (!file.is_open()) return false;
    
    json data;
    file >> data;
    diamonds.clear();
    
    if (!data.contains("diamonds")) return true;
    
    for (auto& DiamData : data["diamonds"]) 
    {
        std::string typeStr = DiamData["type"];
        
        DiamondType type = DiamondType::Blue;
        if (typeStr == "blue") type = DiamondType::Blue;
        else if (typeStr == "red") type = DiamondType::Red;
        Vector2 pos = {(float)DiamData["position"][0], (float)DiamData["position"][1]};
        std::string texture1=DiamData.value("texture", texture1);
        diamonds.emplace_back(pos, type, std::string(texture1));
        diamonds.back().LoadDiamondTexture();

    }
    
    return true;
}

void Diamond::DrawDiamond() const
{
    if (cachedtexture)
    {
        DrawTextureV(*cachedtexture, position, WHITE);
    }
}

void Diamonds::DrawDiamonds() const
{
    for (const auto& diamond:diamonds)
    {
        diamond.DrawDiamond();
    }
}

bool Diamonds::CheckCircleRectCollision(const Vector2& circlePos, float radius, const Vector2& rectPos, const Vector2& rectSize) const 
{
    float closestX = std::max(rectPos.x, std::min(circlePos.x, rectPos.x + rectSize.x));
    float closestY = std::max(rectPos.y, std::min(circlePos.y, rectPos.y + rectSize.y));

    float distX = circlePos.x - closestX;
    float distY = circlePos.y - closestY;
    float distance = std::sqrt(distX * distX + distY * distY);

    return distance < radius;
}

bool Diamonds::CheckCollisionAndCollect(const Vector2& playerPos, const Vector2& playerSize, int playerType) 
{
    bool collected = false;

    for (auto& diamond : diamonds) 
    {
        if (diamond.collected) continue;

        Vector2 diamondCenter = diamond.position;
        
        if (CheckCircleRectCollision(diamondCenter, diamond.size * 2.0f, playerPos, playerSize)) 
        {
            bool canCollect = false;
            
            if (diamond.type == DiamondType::Blue && playerType == 0) 
            {
                canCollect = true;
                diamond.UnloadDiamondTexture();
            } 
            else if (diamond.type == DiamondType::Red && playerType == 1) 
            {
                canCollect = true;
                diamond.UnloadDiamondTexture();
            }

            if (canCollect) 
            {
                diamond.collected = true;
                collected = true;
                
            }
        }
    }

    return collected;
}

int Diamonds::GetCollectedCount() const 
{
    int count = 0;
    for (const auto& diamond : diamonds) 
    {
        if (diamond.collected) count++;
    }
    return count;
}

int Diamonds::GetCollectedCountByType(DiamondType type) const 
{
    int count = 0;
    for (const auto& diamond : diamonds) 
    {
        if (diamond.collected && diamond.type == type) count++;
    }
    return count;
}

bool Doors::LoadFromJSON(const std::string& jsonPath) 
{
    std::ifstream file(jsonPath);
    if (!file.is_open()) return false;

    json data;
    file >> data;
    file.close();
        
    doors.clear();
        
    for (auto& doorData : data["doors"]) 
    {
        Vector2 pos = {(float)doorData["position"][0], (float)doorData["position"][1]};
        std::string type = doorData.value("type", "water");
        doors.emplace_back(pos, type);
    }   
    return true;
        
}


bool Doors::CheckBothPlayersAtDoors(const Vector2& waterPos, const Vector2& waterSize, const Vector2& firePos, const Vector2& fireSize) const 
{
    bool waterAtDoor = false;
    bool fireAtDoor = false;
    
    for (const auto& door : doors) 
    {
        if (door.playerType == "water" && door.CheckCollision(waterPos, waterSize)) 
        {
            waterAtDoor = true;
        } 
        else if (door.playerType == "fire" && door.CheckCollision(firePos, fireSize)) 
        {
            fireAtDoor = true;
        }
    }
    
    return waterAtDoor && fireAtDoor;
}

bool Doors::IsPlayerAtDoor(const Vector2& playerPos, const Vector2& playerSize, const std::string& playerType) const 
{
    for (const auto& door : doors) 
    {
        if (door.playerType == playerType && door.CheckCollision(playerPos, playerSize)) 
        {
            return true;
        }
    }
    return false;
}

