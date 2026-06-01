#pragma once

#include <vector>
#include "QuadTree.h"

class CGameObject;
class CPlayer;
class CMonster;

class CCollisionManager
{
private:
    CQuadTree* m_pQuadTree = NULL;
    std::vector<CGameObject*> m_collisions;
    std::vector<CMonster*>* m_pMonsters = nullptr; 
    int frameCounter = 0;
    bool m_bHitProcessed = false;

public:
    CCollisionManager();
    ~CCollisionManager();

    void Build(const BoundingBox& worldBounds, int maxObjectsPerNode, int maxDepth);
    void InsertObject(CGameObject* object);
    void PrintTree();
    void SetMonsters(std::vector<CMonster*>* monsters) { m_pMonsters = monsters; }
    void Update(CPlayer* player);

    bool IsColliding(const BoundingBox& box1, const BoundingBox& box2);

private:
    void CollectNearbyObjects(QuadTreeNode* node, const BoundingBox& aabb, std::vector<CGameObject*>& collisions);
    void HandleCollision(CPlayer* player, CGameObject* obj);
};