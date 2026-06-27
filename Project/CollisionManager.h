#pragma once

#include <vector>
#include "QuadTree.h"
#include "CFireballSystem.h"
#include "CWeaponThrowSystem.h"
#include "CDebugShader.h"
#include "CCubeMesh.h"

class CGameObject;
class CPlayer;
class CMonster;

class CCollisionManager
{
private:
    CQuadTree* m_pQuadTree = NULL;
    std::vector<CGameObject*> m_objects;
    std::vector<ColliderInfo> m_colliderinfos;
    std::vector<CMonster*>* m_pMonsters = nullptr; 

    CFireballSystem* m_pFireballSystem = nullptr;
    CWeaponThrowSystem* m_pWeaponThrowSystem = nullptr;
  
    int frameCounter = 0;
    bool m_bHitProcessed = false;

    CGameObject* m_pDebugCube;
    CGameObject* m_pDebugSphere;

public:
    CCollisionManager();
    ~CCollisionManager();

    void Build(const BoundingBox& worldBounds, int maxObjectsPerNode, int maxDepth);
    
    void InsertObject(CGameObject* object);
    void InsertCollider(const ColliderInfo& collider);

    void PrintTree();
  
    bool CheckIntersection(const BoundingBox& bounds, const ColliderInfo& col);
  
    void SetMonsters(std::vector<CMonster*>* monsters) { m_pMonsters = monsters; }
    void SetFireballSystem(CFireballSystem* p) { m_pFireballSystem = p; }
    void SetWeaponThrowSystem(CWeaponThrowSystem* p) { m_pWeaponThrowSystem = p; }
  
    void Update(CPlayer* player);

    bool IsColliding(const BoundingBox& box1, const BoundingBox& box2);

    void InitializeDebugObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
    void RenderDebug(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

private:
    void CollectNearbyObjects(QuadTreeNode* node, const BoundingBox& aabb,
        std::vector<CGameObject*>& outDynamics,
        std::vector<ColliderInfo>& outStatics);
    void HandleCollision(CPlayer* player, CGameObject* obj);
    void HandleCollision(CPlayer* player, const ColliderInfo& colinfo);
};