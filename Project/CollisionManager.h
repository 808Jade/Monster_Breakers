#pragma once

#include <vector>
#include "QuadTree.h"
#include "CFireballSystem.h"
#include "CWeaponThrowSystem.h"

class CGameObject;
class CPlayer;
class CMonster;

class CCollisionManager
{
private:
    CQuadTree* m_pQuadTree = NULL;
    std::vector<CGameObject*> m_collisions;
    std::vector<CMonster*>* m_pMonsters = nullptr; 
    CFireballSystem* m_pFireballSystem = nullptr;
    CWeaponThrowSystem* m_pWeaponThrowSystem = nullptr;
    int frameCounter = 0;
    bool m_bHitProcessed = false;

public:
    CCollisionManager();
    ~CCollisionManager();

    void Build(const BoundingBox& worldBounds, int maxObjectsPerNode, int maxDepth);
    void InsertObject(CGameObject* object);
    void PrintTree();
    void SetMonsters(std::vector<CMonster*>* monsters) { m_pMonsters = monsters; }
    void SetFireballSystem(CFireballSystem* p) { m_pFireballSystem = p; }
    void SetWeaponThrowSystem(CWeaponThrowSystem* p) { m_pWeaponThrowSystem = p; }
    void Update(CPlayer* player);

    bool IsColliding(const BoundingBox& box1, const BoundingBox& box2);

private:
    void CollectNearbyObjects(QuadTreeNode* node, const BoundingBox& aabb, std::vector<CGameObject*>& collisions);
    void HandleCollision(CPlayer* player, CGameObject* obj);
};