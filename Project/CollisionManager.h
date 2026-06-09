#pragma once

#include <vector>
#include "QuadTree.h"

class CGameObject;
class CPlayer;

class CCollisionManager
{
private:
    CQuadTree* m_pQuadTree = NULL;
    std::vector<CGameObject*> m_objects;
    std::vector<ColliderInfo> m_colliderinfos;
    int m_frameCounter = 0;

public:
    CCollisionManager();
    ~CCollisionManager();

    void Build(const BoundingBox& worldBounds, int maxObjectsPerNode, int maxDepth);
    
    void InsertObject(CGameObject* object);
    void InsertCollider(const ColliderInfo& collider);

    void PrintTree();

    bool CheckIntersection(const BoundingBox& bounds, const ColliderInfo& col);

    void Update(CPlayer* player);

    bool IsColliding(const BoundingBox& box1, const BoundingBox& box2);

private:
    void CollectNearbyObjects(QuadTreeNode* node, const BoundingBox& aabb,
        std::vector<CGameObject*>& outDynamics,
        std::vector<ColliderInfo>& outStatics);
    void HandleCollision(CPlayer* player, CGameObject* obj);
    void HandleCollision(CPlayer* player, const ColliderInfo& colinfo);
};