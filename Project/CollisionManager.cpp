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
    player->CalculateBoundingBox();

    CTerrainPlayer* tp = dynamic_cast<CTerrainPlayer*>(player);
    if (!tp) return;

    AnimationState curAnim = tp->m_currentAnim;
    bool isAttacking = (curAnim == AnimationState::ATTACK);  // 좌클릭 - 모든 직업 공통
    bool isSkill1 = (curAnim == AnimationState::SKILL1);  // 우클릭 - level[0]
    bool isSkill2 = (curAnim == AnimationState::SKILL2);  // Q      - level[1]
    bool isSkill3 = (curAnim == AnimationState::SKILL3);  // E      - level[2]
    bool isMage = (player->m_ePlayerClass == PlayerClass::MAGE);
    bool isRogue = (player->m_ePlayerClass == PlayerClass::ROGUE);
    bool isKnight = (player->m_ePlayerClass == PlayerClass::KNIGHT);

    // ── 파이어볼 충돌 (법사 SKILL1) ──────────────────
    if (isMage && m_pFireballSystem && m_pMonsters)
    {
        auto activeParticles = m_pFireballSystem->GetActiveParticles();
        for (CMonster* monster : *m_pMonsters)
        {
            if (!monster || monster->IsDead()) continue;
            monster->CalculateBoundingBox();
            BoundingBox monsterBox = monster->GetBoundingBox();

            for (auto& [idx, fpos] : activeParticles)
            {
                BoundingSphere sphere(fpos, 1.5f);
                if (sphere.Intersects(monsterBox))
                {
                    int dmg = player->damage + player->level[0] * 10;
                    cout << "Fireball hit: " << monster->GetFrameName() << " dmg=" << dmg << endl;
                    monster->TakeDamage(dmg);
                    m_pFireballSystem->DeactivateAt(idx);  // 파이어볼 소멸
                    break;
                }
            }
        }
    }

    // ── 투척 무기 충돌 (도적 SKILL1) ─────────────────
    if (isRogue && m_pWeaponThrowSystem && m_pWeaponThrowSystem->IsActive() && m_pMonsters)
    {
        BoundingSphere throwSphere(
            m_pWeaponThrowSystem->GetPosition(),
            m_pWeaponThrowSystem->GetHitRadius());

        for (CMonster* monster : *m_pMonsters)
        {
            if (!monster || monster->IsDead()) continue;
            monster->CalculateBoundingBox();
            if (throwSphere.Intersects(monster->GetBoundingBox()))
            {
                int dmg = player->damage + player->level[0] * 10;
                cout << "Throw hit: " << monster->GetFrameName() << " dmg=" << dmg << endl;
                monster->TakeDamage(dmg);
                m_pWeaponThrowSystem->Deactivate();  // 무기 소멸 + 손 무기 복원
                break;
            }
        }
    }

    // ── 무기 충돌 (weapon BoundingBox 기반) ──────────────
        // 좌클릭(ATTACK): 모든 직업 공통, 기본 damage
        // 기사  Q(SKILL2): level[1] 보너스
        // 도적  Q(SKILL2): level[1] 보너스
        // 법사는 weapon 충돌 없음

    int weaponDmg = 0;
    bool isWeaponSwing = false;

    if (isAttacking)
    {
        isWeaponSwing = true;
        weaponDmg = player->damage;  // 기본 공격 - 레벨 보너스 없음
    }
    else if (!isMage && isSkill2)   // Q - 기사/도적
    {
        isWeaponSwing = true;
        weaponDmg = player->damage + player->level[1] * 10;  // Q 스킬 레벨
    }

    if (isWeaponSwing && !m_bHitProcessed && m_pMonsters)
    {
        BoundingBox weaponBox = player->GetWeaponAttackBoundingBox();
        for (CMonster* monster : *m_pMonsters)
        {
            if (!monster || monster->IsDead()) continue;
            monster->CalculateBoundingBox();
            if (weaponBox.Intersects(monster->GetBoundingBox()))
            {
                cout << "Weapon hit: " << monster->GetFrameName() << " dmg=" << weaponDmg << endl;
                monster->TakeDamage(weaponDmg);
                m_bHitProcessed = true;
            }
        }
    }

    // 공격 애니메이션 종료 시 히트 플래그 리셋
    if (!isWeaponSwing)
        m_bHitProcessed = false;

    // ── 환경 충돌 (쿼드트리) ─────────────────────────
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
        //if (std::string::npos != ObjectFrameName.find("SalamanderPA") && obj != player && player->GetSwordAttackBoundingBox().Intersects(obj->GetBoundingBox()))
        //{
        //    HandleCollision(player, obj);
        //}
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

void CCollisionManager::InitializeDebugObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
    if (m_pDebugCube != nullptr) return;

    m_pDebugCube = new CGameObject();

    CCubeMesh* pUnitCubeMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 1.0f, 1.0f, 1.0f);
    m_pDebugCube->SetMesh(pUnitCubeMesh);

    CDebugShader* pWireframeShader = new CDebugShader();
    pWireframeShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
    m_pDebugCube->SetShader(pWireframeShader);
}

void CCollisionManager::RenderDebug(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (!m_pDebugCube) return;

    // 와이어프레임(선)으로 그리기 위한 파이프라인(PSO) 세팅
    //pd3dCommandList->SetPipelineState(m_pWireframePSO);

    for (const ColliderInfo& col : m_colliderinfos)
    {
        switch (col.type)
        {
        case ColliderType::AABB:
        {
            // 1. 충돌체의 중심(Center)과 크기(Extents)를 바탕으로 월드 행렬을 만듭니다.
            // Extents는 절반 크기이므로 2배를 곱해 스케일을 맞춥니다.
            DirectX::XMMATRIX matScale = DirectX::XMMatrixScaling(col.aabb.Extents.x * 2.0f, col.aabb.Extents.y * 2.0f, col.aabb.Extents.z * 2.0f);
            DirectX::XMMATRIX matTrans = DirectX::XMMatrixTranslation(col.aabb.Center.x, col.aabb.Center.y, col.aabb.Center.z);

            DirectX::XMFLOAT4X4 worldMat;
            DirectX::XMStoreFloat4x4(&worldMat, matScale * matTrans);

            // 2. 만들어둔 단일 객체의 월드 행렬을 지금 검사 중인 충돌체 위치로 '순간이동' 시킵니다.
            m_pDebugCube->m_xmf4x4World = worldMat;

            m_pDebugCube->Render(pd3dCommandList, pCamera);
            break;
        }

        case ColliderType::OBB:
        {
            using namespace DirectX;

            // 1. 크기 (Scale): Extents는 절반 크기이므로 2배를 곱해줍니다.
            XMMATRIX matScale = XMMatrixScaling(col.obb.Extents.x * 2.0f,
                col.obb.Extents.y * 2.0f,
                col.obb.Extents.z * 2.0f);

            // 2. 회전 (Rotation) [핵심!]: OBB가 가진 쿼터니언(Orientation) 정보를 회전 행렬로 변환합니다.
            XMVECTOR quat = XMLoadFloat4(&col.obb.Orientation);
            XMMATRIX matRot = XMMatrixRotationQuaternion(quat);

            // 3. 이동 (Translation): 중심점 위치
            XMMATRIX matTrans = XMMatrixTranslation(col.obb.Center.x,
                col.obb.Center.y,
                col.obb.Center.z);

            // 4. 월드 행렬 조합 (★반드시 Scale -> Rotation -> Translation 순서로 곱해야 합니다★)
            XMFLOAT4X4 worldMat;
            XMStoreFloat4x4(&worldMat, matScale * matRot * matTrans);

            // 5. AABB 그릴 때 쓰던 그 큐브 객체를 그대로 재사용해서 그립니다!
            m_pDebugCube->m_xmf4x4World = worldMat;
            m_pDebugCube->Render(pd3dCommandList, pCamera);

            break;
        }

        case ColliderType::Sphere:
        {
            /*
            XMMATRIX matScale = XMMatrixScaling(col.sphere.Radius, col.sphere.Radius, col.sphere.Radius);
            XMMATRIX matTrans = XMMatrixTranslation(col.sphere.Center.x, col.sphere.Center.y, col.sphere.Center.z);

            XMFLOAT4X4 worldMat;
            XMStoreFloat4x4(&worldMat, matScale * matTrans);

            m_pDebugSphere->SetWorldMatrix(worldMat);
            m_pDebugSphere->Render(pd3dCommandList, pCamera);
            */
            break;
        }
        }
    }

    
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
    if (dynamic_cast<CMonster*>(obj) != nullptr) return;
    std::string ObjectFrameName = obj->GetFrameName();

    //if (m_frameCounter++ % 60 == 0)
    //    cout << "ObjectFrameName: " << ObjectFrameName << endl;

    bool isMonster = (dynamic_cast<CMonster*>(obj) != nullptr);
    bool isAttacking = (dynamic_cast<CTerrainPlayer*>(player)->m_currentAnim == AnimationState::ATTACK);

  //  bool isSwordHit = player->GetSwordAttackBoundingBox().Intersects(obj->GetBoundingBox());
//
  //  if (isMonster && isAttacking && isSwordHit)
  //  {
//
  //      //std::cout << "Sword hit ! - " << ObjectFrameName << std::endl;
		//dynamic_cast<CMonster*>(obj)->TakeDamage(player->damage);
  //      return;
  //  }

    BoundingBox swordBox = player->GetWeaponAttackBoundingBox();
    bool isValidBox = (swordBox.Extents.x > 0.0f || swordBox.Extents.y > 0.0f || swordBox.Extents.z > 0.0f);
    bool isSwordHit = isValidBox && swordBox.Intersects(obj->GetBoundingBox());

    if (isMonster && isAttacking && isSwordHit)
    {
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
        player->SetPosition(playerPos);
        player->SetVelocity({ 0.0f, 0.0f, 0.0f });
        player->CalculateBoundingBox();
        playerBox = player->GetBoundingBox();
    }
}

void CCollisionManager::HandleCollision(CPlayer* player, const ColliderInfo& colinfo)
{
    DirectX::XMFLOAT3 playerPos = player->GetPosition();
    BoundingBox playerBox = player->GetBoundingBox();

    switch (colinfo.type)
    {
    case ColliderType::AABB:
    {
        BoundingBox objBox = colinfo.aabb;
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

        DirectX::XMFLOAT3 overlap;
        overlap.x = std::min(playerMax.x, objMax.x) - std::max(playerMin.x, objMin.x);
        overlap.z = std::min(playerMax.z, objMax.z) - std::max(playerMin.z, objMin.z);

        if (overlap.x < overlap.z) {
            if (playerPos.x < objBox.Center.x) playerPos.x = objMin.x - playerBox.Extents.x;
            else                               playerPos.x = objMax.x + playerBox.Extents.x;
        }
        else {
            if (playerPos.z < objBox.Center.z) playerPos.z = objMin.z - playerBox.Extents.z;
            else                               playerPos.z = objMax.z + playerBox.Extents.z;
        }
        break;
    }

    case ColliderType::Sphere:
    {
        float dx = playerBox.Center.x - colinfo.sphere.Center.x;
        float dz = playerBox.Center.z - colinfo.sphere.Center.z;
        float distance = std::sqrt(dx * dx + dz * dz);

        if (distance > 0.0001f) // 중심이 완벽히 겹쳤을 때의 0 나누기 방지
        {
            // 플레이어를 대략적인 원통(Cylinder)으로 간주하여 충돌 반경 계산
            float playerRadius = std::max(playerBox.Extents.x, playerBox.Extents.z);
            float safeDistance = playerRadius + colinfo.sphere.Radius;

            if (distance < safeDistance)
            {
                float overlap = safeDistance - distance;
                float nx = dx / distance;
                float nz = dz / distance;

                playerPos.x += nx * overlap;
                playerPos.z += nz * overlap;
            }
        }
        break;
    }

    case ColliderType::OBB:
    {
        // 장애물 중심 -> 플레이어 중심 방향으로 부드럽게 밀어내기
        using namespace DirectX;

        XMVECTOR vPlayerCenter = XMLoadFloat3(&playerBox.Center);
        XMVECTOR vObbCenter = XMLoadFloat3(&colinfo.obb.Center);

        // Y축(높이)은 무시하고 X, Z 평면에서의 방향 벡터만 계산
        XMVECTOR vDir = XMVectorSubtract(vPlayerCenter, vObbCenter);
        vDir = XMVectorSetY(vDir, 0.0f);

        float distance = XMVectorGetX(XMVector3Length(vDir));

        if (distance > 0.0001f)
        {
            vDir = XMVector3Normalize(vDir);

            float playerRadius = std::max(playerBox.Extents.x, playerBox.Extents.z);
            float obbRadius = std::sqrt(colinfo.obb.Extents.x * colinfo.obb.Extents.x +
                colinfo.obb.Extents.z * colinfo.obb.Extents.z);

            float safeDistance = playerRadius + obbRadius;

            if (distance < safeDistance)
            {
                float overlap = safeDistance - distance;

                playerPos.x += XMVectorGetX(vDir) * overlap;
                playerPos.z += XMVectorGetZ(vDir) * overlap;
            }
        }
        break;
    }
    }

    player->SetPosition(playerPos);
    player->SetVelocity({ 0.0f, 0.0f, 0.0f });
    player->CalculateBoundingBox();
}
