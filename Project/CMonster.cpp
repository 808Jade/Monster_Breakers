#include "stdafx.h"
#include "CMonster.h"
#include "Player.h"
#include "Network.h"   // g_monsters

CMonster::CMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
    const char* pstrModelPath, int nAnimationTracks, CLoadedModelInfo* pModel, float fMaxHP)
    : CGameObject(1), m_fMaxHP(fMaxHP), m_fMonsterHP(fMaxHP)
{
    CLoadedModelInfo* pMonsterModel = pModel;
    if (!pMonsterModel)
        pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(
            pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pstrModelPath, NULL);

    SetChild(pMonsterModel->m_pModelRootObject, true);

    m_pSkinnedAnimationController = new CAnimationController(
        pd3dDevice, pd3dCommandList, nAnimationTracks, pMonsterModel);

    // 0=Idle, 1=Walk, 2=Attack, 3=GetHit, 4=Death
    for (int i = 0; i < nAnimationTracks; ++i)
        m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);

    // Idle만 활성화
    for (int i = 1; i < nAnimationTracks; ++i)
        m_pSkinnedAnimationController->SetTrackEnable(i, false);

    CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CMonster::~CMonster()
{
    if (m_pHpbar) delete m_pHpbar;
}

std::vector<CMonster*> CMonster::SpawnGroup(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
    ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pstrModelPath,
    int                        count,
    int                        startID,
    float                      fMaxHP,
    float                      fScale)
{
    // 모델은 한 번만 로드, 모든 인스턴스가 공유
    CLoadedModelInfo* pSharedModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pstrModelPath, NULL);

    std::vector<CMonster*> group;
    group.reserve(count);

    for (int i = 0; i < count; ++i)
    {
        CMonster* pMonster = new CMonster(
            pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pstrModelPath, 5, pSharedModel, fMaxHP);
        //pMonster->m_pHpbar = new Hpbar(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
        const int id = startID + i;
        pMonster->SetMonsterID(id);
        pMonster->SetScale(fScale, fScale, fScale);

        g_monsters[id] = pMonster;   // 서버 ID 등록
        group.push_back(pMonster);
    }

    // 공유 모델 정보 객체 해제 (각 인스턴스는 내부적으로 필요한 데이터를 이미 복사)
    delete pSharedModel;

    return group;
}

// -----------------------------------------------------------------------

void CMonster::TakeDamage(float damage)
{
    if (m_eState == MonsterState::Death) return;

    m_fMonsterHP = max(0.0f, m_fMonsterHP - damage);
    m_fHpRatio = m_fMonsterHP / m_fMaxHP;

    TransitionTo(m_fMonsterHP <= 0.0f ? MonsterState::Death : MonsterState::GetHit);
}

void CMonster::TransitionTo(MonsterState newState)
{
    if (m_eState == MonsterState::Death) return;
    if (!m_pSkinnedAnimationController)  return;

    auto toTrack = [](MonsterState s) -> int {
        switch (s) {
        case MonsterState::Idle:   return 0;
        case MonsterState::Walk:   return 1;
        case MonsterState::Attack: return 2;
        case MonsterState::GetHit: return 3;
        case MonsterState::Death:  return 4;
        default:                   return 0;
        }
        };

    m_pSkinnedAnimationController->SetTrackEnable(toTrack(m_eState), false);
    m_eState = newState;

    int newTrack = toTrack(m_eState);
    m_pSkinnedAnimationController->SetTrackPosition(newTrack, 0.0f);
    m_pSkinnedAnimationController->SetTrackEnable(newTrack, true);
}

void CMonster::Animate(float fTimeElapsed)
{
    CGameObject::Animate(fTimeElapsed);
}

void CMonster::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    CGameObject::Render(pd3dCommandList, pCamera);
    if (m_pHpbar && !IsDead())
    {
        XMFLOAT3 pos = GetPosition();

        m_pHpbar->SetPosition(pos.x, pos.y + 2.5f, pos.z);
        m_pHpbar->LookAt(pCamera->GetPosition(), XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_pHpbar->SetHpRatio(m_fHpRatio);

        m_pHpbar->Render(pd3dCommandList, pCamera);
    }
}