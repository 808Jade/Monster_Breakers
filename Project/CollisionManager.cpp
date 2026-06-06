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

void CCollisionManager::PrintTree()
{
    m_pQuadTree->PrintTree();
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
    m_collisions.clear();
    CollectNearbyObjects(m_pQuadTree->root, player->GetBoundingBox(), m_collisions);
    for (CGameObject* obj : m_collisions)
    {
        if (dynamic_cast<CMonster*>(obj)) continue;
        if (player->GetBoundingBox().Intersects(obj->GetBoundingBox()))
            HandleCollision(player, obj);
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

void CCollisionManager::CollectNearbyObjects(QuadTreeNode* node, const BoundingBox& aabb, std::vector<CGameObject*>& collisions)
{
    if (!node || !node->bounds.Intersects(aabb)) return;

    // 리프 노드면 오브젝트 수집
    if (node->isLeaf)
    {
        for (CGameObject* obj : node->objects)
        {
            if (obj) collisions.push_back(obj);
        }
        return;
    }

    // 내부 노드면 자식 재귀 탐색
    for (int i = 0; i < 4; i++)
    {
        if (node->children[i])
            CollectNearbyObjects(node->children[i], aabb, collisions);
    }
}

void CCollisionManager::HandleCollision(CPlayer* player, CGameObject* obj)
{
    // 몬스터는 여기 들어오지 않지만 방어적으로 차단
    if (dynamic_cast<CMonster*>(obj) != nullptr) return;
    std::string ObjectFrameName = obj->GetFrameName();

    //if (frameCounter % 60 == 0)
    //    cout << "ObjectFrameName: " << ObjectFrameName << endl;

    if (std::string::npos != ObjectFrameName.find("Map_wall_window")
        || std::string::npos != ObjectFrameName.find("Map_wall_plain")
        || std::string::npos != ObjectFrameName.find("Map_wall_baydoor")
        )
    {
        // 플레이어와 벽의 경계 상자 가져오기
        BoundingBox playerBox = player->GetBoundingBox();
        BoundingBox wallBox = obj->GetBoundingBox();

        // 플레이어와 벽의 중심 간 차이
        XMFLOAT3 playerCenter = playerBox.Center;
        XMFLOAT3 wallCenter = wallBox.Center;
        XMFLOAT3 diff(playerCenter.x - wallCenter.x, playerCenter.y - wallCenter.y, playerCenter.z - wallCenter.z);

        //// 플레이어 BoundingBox의 4개 꼭짓점 계산
        XMFLOAT3 playerExtents = playerBox.Extents;
        XMFLOAT3 playerVertices[4] = {
            XMFLOAT3(playerCenter.x - playerExtents.x, playerCenter.y, playerCenter.z - playerExtents.z), // 우상단
            XMFLOAT3(playerCenter.x + playerExtents.x, playerCenter.y, playerCenter.z - playerExtents.z), // 좌상단
            XMFLOAT3(playerCenter.x + playerExtents.x, playerCenter.y, playerCenter.z + playerExtents.z), // 좌하단
            XMFLOAT3(playerCenter.x - playerExtents.x, playerCenter.y, playerCenter.z + playerExtents.z), // 우하단
        };

        // 벽 BoundingBox의 4개 꼭짓점 계산
        XMFLOAT3 wallExtents = wallBox.Extents;
        XMFLOAT3 wallVertices[4] = {
            XMFLOAT3(wallCenter.x - wallExtents.x, playerCenter.y, wallCenter.z - wallExtents.z), // 우상단
            XMFLOAT3(wallCenter.x + wallExtents.x, playerCenter.y, wallCenter.z - wallExtents.z), // 좌상단
            XMFLOAT3(wallCenter.x + wallExtents.x, playerCenter.y, wallCenter.z + wallExtents.z), // 좌하단
            XMFLOAT3(wallCenter.x - wallExtents.x, playerCenter.y, wallCenter.z + wallExtents.z), // 우하단
        };

        // 벽이 밀어낼 방향 과 거리 찾기
        XMFLOAT3 pushDirection(0, 0, 0);
        float pushDistance{ 0.0f };
        float pushMargin{ 0.0f };
        float maxExtent = std::max(wallExtents.x, wallExtents.z);
        if (maxExtent == wallExtents.x)
        {
            if (diff.z < 0)
            {
                // 위로 밀어야 함
                pushDirection = XMFLOAT3(0, 0, -1);
                pushDistance = playerVertices[2].z - wallVertices[1].z + pushMargin;
            }
            else
            {
                // 아래로 밀어야 함
                pushDirection = XMFLOAT3(0, 0, 1);
                pushDistance = wallVertices[2].z - playerVertices[1].z + pushMargin;
            }
        }
        else
        {
            if (diff.x < 0)
            {
                // 오른쪽으로 밀어야 함
                pushDirection = XMFLOAT3(-1, 0, 0);
                pushDistance = playerVertices[1].x - wallVertices[0].x + pushMargin;
            }
            else
            {
                // 왼쪽으로 밀어야 함
                pushDirection = XMFLOAT3(1, 0, 0);
                pushDistance = wallVertices[1].x - playerVertices[0].x + pushMargin;
            }
        }

        XMFLOAT3 pushVector(
            pushDirection.x * pushDistance,
            0,
            pushDirection.z * pushDistance
        );

        XMFLOAT3 shift = pushVector;
        player->SetVelocity(XMFLOAT3(0, 0, 0));
        player->Move(shift, false);
        player->CalculateBoundingBox();
        playerBox = player->GetBoundingBox();
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