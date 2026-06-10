#include "stdafx.h"
#include "CollisionManager.h"
#include "Object.h"
#include "Player.h"

CCollisionManager::CCollisionManager()
{
    m_pQuadTree = new CQuadTree();
}

CCollisionManager::~CCollisionManager()
{
    delete m_pQuadTree;
}

void CCollisionManager::Build(const BoundingBox& worldBounds, int maxObjectsPerNode, int maxDepth)
{
    m_pQuadTree->Build(worldBounds, maxObjectsPerNode, maxDepth);
}

void CCollisionManager::InsertObject(CGameObject* object)
{
    m_pQuadTree->Insert(object);
}

void CCollisionManager::InsertCollider(const ColliderInfo& collider)
{
    m_pQuadTree->Insert(collider);
}

void CCollisionManager::PrintTree()
{
    m_pQuadTree->PrintTree();
}

bool CCollisionManager::CheckIntersection(const BoundingBox& bounds, const ColliderInfo& col)
{
    switch (col.type)
    {
    case ColliderType::AABB:    return bounds.Intersects(col.aabb);
    case ColliderType::OBB:     return bounds.Intersects(col.obb);
    case ColliderType::Sphere:  return bounds.Intersects(col.sphere);
    case ColliderType::Segment:
    {
        XMVECTOR start = XMLoadFloat3(&col.segment.start);
        XMVECTOR end = XMLoadFloat3(&col.segment.end);
        XMVECTOR dir = XMVector3Normalize(end - start);
        float len = XMVectorGetX(XMVector3Length(end - start));
        float dist = 0.0f;
        return bounds.Intersects(start, dir, dist) && (dist <= len);
    }
    }
    return false;
}

void CCollisionManager::Update(CPlayer* player)
{
    // 플레이어가 속한 노드 탐색
    QuadTreeNode* playerNode = m_pQuadTree->FindNode(m_pQuadTree->root, player->GetBoundingBox());
    if (!playerNode) return;

    //if (m_frameCounter++ % 60 == 0) // 60 프레임마다 출력
    //    cout << playerNode->bounds.Center.x << ", " << playerNode->bounds.Center.z << endl;

    // 근처 오브젝트 수집
    m_objects.clear();
    m_colliderinfos.clear();
    CollectNearbyObjects(playerNode, player->GetBoundingBox(), m_objects, m_colliderinfos);

    // object 대상 충돌 검사 및 처리
    for (CGameObject* obj : m_objects)
    {
        std::string ObjectFrameName = obj->GetFrameName();

        if (obj != player && player->GetBoundingBox().Intersects(obj->GetBoundingBox()))
        {
            HandleCollision(player, obj);
        }
        // for 검 공격 충돌?
        if (std::string::npos != ObjectFrameName.find("SalamanderPA") && obj != player && player->GetSwordAttackBoundingBox().Intersects(obj->GetBoundingBox()))
        {
            HandleCollision(player, obj);
        }
    }

    // ColliderInfo 대상 충돌 검사 및 처리
    for (const ColliderInfo& col : m_colliderinfos) 
    {
        if (CheckIntersection(player->GetBoundingBox(), col))
        {
            HandleCollision(player, col);
        }
    }
}

bool CCollisionManager::IsColliding(const BoundingBox& box1, const BoundingBox& box2)
{
    // X축 충돌 검사
    if (fabs(box1.Center.x - box2.Center.x) > (box1.Extents.x + box2.Extents.x))
        return false;

    // Z축 충돌 검사
    if (fabs(box1.Center.z - box2.Center.z) > (box1.Extents.z + box2.Extents.z))
        return false;

    return true;
}

void CCollisionManager::CollectNearbyObjects(QuadTreeNode* node, const BoundingBox& playerbb, std::vector<CGameObject*>& outDynamics, std::vector<ColliderInfo>& outStatics) 
{
    if (!node) return;
    if (!node->bounds.Intersects(playerbb)) return;

    for (CGameObject* obj : node->objects)
    {
        if (obj) outDynamics.push_back(obj);
    }

    for (const ColliderInfo& col : node->colliders)
    {
        outStatics.push_back(col);
    }

    if (!node->isLeaf)
    {
        for (int i = 0; i < 4; i++)
        {
            CollectNearbyObjects(node->children[i], playerbb, outDynamics, outStatics);
        }
    }
}

void CCollisionManager::HandleCollision(CPlayer* player, CGameObject* obj)
{
    std::string ObjectFrameName = obj->GetFrameName();

    //if (m_frameCounter++ % 60 == 0)
    //    cout << "ObjectFrameName: " << ObjectFrameName << endl;

    bool isMonster = (dynamic_cast<CMonster*>(obj) != nullptr);
    bool isAttacking = (dynamic_cast<CTerrainPlayer*>(player)->m_currentAnim == AnimationState::ATTACK);
    bool isSwordHit = player->GetSwordAttackBoundingBox().Intersects(obj->GetBoundingBox());

    if (isMonster && isAttacking && isSwordHit)
    {
        //std::cout << "Sword hit ! - " << ObjectFrameName << std::endl;
		dynamic_cast<CMonster*>(obj)->TakeDamage(player->damage);
        return;
    }

    if (std::string::npos != ObjectFrameName.find("Map_barrel")
        || std::string::npos != ObjectFrameName.find("Map_pallet_variation_01")
        || std::string::npos != ObjectFrameName.find("Map_pallet_variation_02")
        || std::string::npos != ObjectFrameName.find("Map_pallet_variation_03")
        || std::string::npos != ObjectFrameName.find("Map_pallet_variation_04")
        || std::string::npos != ObjectFrameName.find("Map_pallet_variation_05")
        || std::string::npos != ObjectFrameName.find("Map_pallet_variation_06")
        || std::string::npos != ObjectFrameName.find("Map_pallet_variation_07")
        || std::string::npos != ObjectFrameName.find("Map_pallet_variation_08")
        || std::string::npos != ObjectFrameName.find("Map_pallet_variation_09")
        || std::string::npos != ObjectFrameName.find("Map_pillar")
        || std::string::npos != ObjectFrameName.find("Map_shelf_variation_01")
        || std::string::npos != ObjectFrameName.find("Map_shelf_variation_02")
        || std::string::npos != ObjectFrameName.find("Map_shelf_variation_03")
        || std::string::npos != ObjectFrameName.find("Map_shelf_variation_04")
        || std::string::npos != ObjectFrameName.find("Map_shelf_variation_05")
        || std::string::npos != ObjectFrameName.find("Map_shelf_variation_06")
        || std::string::npos != ObjectFrameName.find("Map_shelves_empty")
        || std::string::npos != ObjectFrameName.find("Map_crate_long")
        || std::string::npos != ObjectFrameName.find("Map_garbage_bin")
        || std::string::npos != ObjectFrameName.find("Map_duct_vent")
        || std::string::npos != ObjectFrameName.find("Map_duct_elbow_01")
        || std::string::npos != ObjectFrameName.find("Map_duct_elbow_02")
        || std::string::npos != ObjectFrameName.find("Map_duct_tee")
        || std::string::npos != ObjectFrameName.find("Map_crate_long")
        || std::string::npos != ObjectFrameName.find("Map_crate_short")

        //|| std::string::npos != ObjectFrameName.find("Map_wall_window")
        //|| std::string::npos != ObjectFrameName.find("Map_wall_plain")
        //|| std::string::npos != ObjectFrameName.find("Map_wall_baydoor")
        )
    {
        DirectX::XMFLOAT3 playerPos = player->GetPosition();
        BoundingBox playerBox = player->GetBoundingBox();
        BoundingBox objBox = obj->GetBoundingBox();

        // Min, Max 계산
        DirectX::XMFLOAT3 playerMin, playerMax, objMin, objMax;
        playerMin.x = playerBox.Center.x - playerBox.Extents.x;
        playerMin.y = playerBox.Center.y - playerBox.Extents.y;
        playerMin.z = playerBox.Center.z - playerBox.Extents.z;
        playerMax.x = playerBox.Center.x + playerBox.Extents.x;
        playerMax.y = playerBox.Center.y + playerBox.Extents.y;
        playerMax.z = playerBox.Center.z + playerBox.Extents.z;

        objMin.x = objBox.Center.x - objBox.Extents.x;
        objMin.y = objBox.Center.y - objBox.Extents.y;
        objMin.z = objBox.Center.z - objBox.Extents.z;
        objMax.x = objBox.Center.x + objBox.Extents.x;
        objMax.y = objBox.Center.y + objBox.Extents.y;
        objMax.z = objBox.Center.z + objBox.Extents.z;

        // 겹침 크기 계산 (x, z축)
        DirectX::XMFLOAT3 overlap;
        overlap.x = std::min(playerMax.x, objMax.x) - std::max(playerMin.x, objMin.x);
        overlap.z = std::min(playerMax.z, objMax.z) - std::max(playerMin.z, objMin.z);

        // 겹침이 작은 축을 기준으로 플레이어 위치 조정
        if (overlap.x < overlap.z)
        {
            if (playerPos.x < objBox.Center.x)
                playerPos.x = objMin.x - playerBox.Extents.x; // 왼쪽으로 밀어냄
            else
                playerPos.x = objMax.x + playerBox.Extents.x; // 오른쪽으로 밀어냄
        }
        else
        {
            if (playerPos.z < objBox.Center.z)
                playerPos.z = objMin.z - playerBox.Extents.z; // 아래로 밀어냄
            else
                playerPos.z = objMax.z + playerBox.Extents.z; // 위로 밀어냄
        }
        player->SetPosition(playerPos); // 플레이어 위치 갱신
        player->SetVelocity({ 0.0f, 0.0f, 0.0f }); // 속도 정지
        player->CalculateBoundingBox();
        playerBox = player->GetBoundingBox();
    }
}

void CCollisionManager::HandleCollision(CPlayer* player, const ColliderInfo& colinfo)
{
    // 현재 맵 인스턴스들은 모두 AABB로 처리되고 있으므로, AABB일 때만 밀어내기 적용
    // 앞에서 걸러놓고, 여기로 들어오는 애들에 대해서는 그냥 다 AABB 처리를 해버리기
    // string::find() 를 하지 않는다..? 결국 앞에서 하면 똑같은게 아닌가 싶긴 하지만..
    if (colinfo.type != ColliderType::AABB) return;

    DirectX::XMFLOAT3 playerPos = player->GetPosition();
    BoundingBox playerBox = player->GetBoundingBox();
    BoundingBox objBox = colinfo.aabb;

    // Min, Max 계산
    DirectX::XMFLOAT3 playerMin, playerMax, objMin, objMax;

    playerMin.x = playerBox.Center.x - playerBox.Extents.x;
    playerMin.y = playerBox.Center.y - playerBox.Extents.y;
    playerMin.z = playerBox.Center.z - playerBox.Extents.z;
    playerMax.x = playerBox.Center.x + playerBox.Extents.x;
    playerMax.y = playerBox.Center.y + playerBox.Extents.y;
    playerMax.z = playerBox.Center.z + playerBox.Extents.z;

    objMin.x = objBox.Center.x - objBox.Extents.x;
    objMin.y = objBox.Center.y - objBox.Extents.y;
    objMin.z = objBox.Center.z - objBox.Extents.z;
    objMax.x = objBox.Center.x + objBox.Extents.x;
    objMax.y = objBox.Center.y + objBox.Extents.y;
    objMax.z = objBox.Center.z + objBox.Extents.z;

    // 겹침 크기 계산 (x, z축)
    DirectX::XMFLOAT3 overlap;
    overlap.x = std::min(playerMax.x, objMax.x) - std::max(playerMin.x, objMin.x);
    overlap.z = std::min(playerMax.z, objMax.z) - std::max(playerMin.z, objMin.z);

    // 겹침이 작은 축(더 얕게 파고든 축)을 기준으로 플레이어 위치 조정 (AABB Response)
    if (overlap.x < overlap.z)
    {
        if (playerPos.x < objBox.Center.x)
            playerPos.x = objMin.x - playerBox.Extents.x;
        else
            playerPos.x = objMax.x + playerBox.Extents.x;
    }
    else
    {
        if (playerPos.z < objBox.Center.z)
            playerPos.z = objMin.z - playerBox.Extents.z;
        else
            playerPos.z = objMax.z + playerBox.Extents.z;
    }

    // 플레이어 위치 갱신 및 상태 초기화
    player->SetPosition(playerPos);
    player->SetVelocity({ 0.0f, 0.0f, 0.0f });
    player->CalculateBoundingBox();
}
