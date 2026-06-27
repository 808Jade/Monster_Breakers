#include "stdafx.h"
#include "CBossMonster.h"
#include "Player.h"
#include "Network.h"   // g_monsters / send_hit_damage

namespace
{
    constexpr int TRACK_IDLE = 0;
    constexpr int TRACK_WALK = 1;
    constexpr int TRACK_ATTACK01 = 2;
    constexpr int TRACK_ATTACK02 = 3;
    constexpr int TRACK_TAUNT = 4;
    constexpr int TRACK_DEATH = 5;
    constexpr int BOSS_ANIMATION_TRACKS = 6;
}

CBossMonster::CBossMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
    const char* pstrModelPath, CLoadedModelInfo* pModel, float fMaxHP, int id)
    : CGameObject(1), m_fMaxHP(fMaxHP), m_fMonsterHP(fMaxHP)
{
    CLoadedModelInfo* pBossModel = pModel;
    if (!pBossModel)
        pBossModel = CGameObject::LoadGeometryAndAnimationFromFile(
            pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pstrModelPath, NULL);

    SetChild(pBossModel->m_pModelRootObject, true);

    m_pSkinnedAnimationController = new CAnimationController(
        pd3dDevice, pd3dCommandList, BOSS_ANIMATION_TRACKS, pBossModel);

    // 0=Idle, 1=Walk, 2=Attack01, 3=Attack02, 4=Taunt, 5=Death
    for (int i = 0; i < BOSS_ANIMATION_TRACKS; ++i)
        m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);

    m_pSkinnedAnimationController->SetTrackType(TRACK_ATTACK01, ANIMATION_TYPE_ONCE);
    m_pSkinnedAnimationController->SetTrackType(TRACK_ATTACK02, ANIMATION_TYPE_ONCE);
    m_pSkinnedAnimationController->SetTrackType(TRACK_TAUNT, ANIMATION_TYPE_ONCE);
    m_pSkinnedAnimationController->SetTrackType(TRACK_DEATH, ANIMATION_TYPE_ONCE);

    // Idle만 활성화
    for (int i = 1; i < BOSS_ANIMATION_TRACKS; ++i)
        m_pSkinnedAnimationController->SetTrackEnable(i, false);

    SetScale(1.0f, 1.0f, 1.0f);

    m_pHpbar = new Hpbar(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);

/*    SetMonsterID(id);
    g_monsters[id] = this;*/

    CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CBossMonster::~CBossMonster()
{
    if (m_pHpbar) delete m_pHpbar;
}

int CBossMonster::TrackOf(BossState s) const
{
    switch (s)
    {
    case BossState::Idle:     return TRACK_IDLE;
    case BossState::Walk:     return TRACK_WALK;
    case BossState::Attack01: return TRACK_ATTACK01;
    case BossState::Attack02: return TRACK_ATTACK02;
    case BossState::Taunt:    return TRACK_TAUNT;
    case BossState::Death:    return TRACK_DEATH;
    default:                  return TRACK_IDLE;
    }
}

void CBossMonster::TransitionTo(BossState newState)
{
    if (m_eState == BossState::Death) return;
    if (!m_pSkinnedAnimationController)  return;

    auto toTrack = [](BossState s) -> int {
        switch (s) {
        case BossState::Idle:   return 0;
        case BossState::Walk:   return 1;
        case BossState::Attack01: return 2;
        case BossState::Attack02: return 3;
        case BossState::Taunt:  return 4;
        case BossState::Death:  return 5;
        default:                   return 0;
        }
        };

    m_pSkinnedAnimationController->SetTrackEnable(toTrack(m_eState), false);
    m_eState = newState;

    int newTrack = toTrack(m_eState);
    m_pSkinnedAnimationController->SetTrackPosition(newTrack, 0.0f);
    m_pSkinnedAnimationController->SetTrackEnable(newTrack, true);
}

void CBossMonster::TakeDamage(float damage)
{
    if (m_eState == BossState::Death) return;

    m_fMonsterHP = max(0.0f, m_fMonsterHP - damage);
    m_fHpRatio = m_fMonsterHP / m_fMaxHP;

    send_hit_damage(m_nMonsterID, (int)damage);

    if (m_fMonsterHP <= 0.0f)
        TransitionTo(BossState::Death);
}

void CBossMonster::Animate(float fTimeElapsed)
{
    CGameObject::Animate(fTimeElapsed); // 내부에서 애니메이션 트랙 진행(AdvanceTime)
}

void CBossMonster::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    CGameObject::Render(pd3dCommandList, pCamera);

    if (m_pHpbar && !IsDead())
    {
        XMFLOAT3 pos = GetPosition();

        m_pHpbar->SetPosition(pos.x, pos.y + 5.0f, pos.z);
        m_pHpbar->LookAt(pCamera->GetPosition(), XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_pHpbar->SetHpRatio(m_fHpRatio);
        m_pHpbar->Render(pd3dCommandList, pCamera);
    }
}