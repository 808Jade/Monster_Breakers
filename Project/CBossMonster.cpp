#include "stdafx.h"
#include "CBossMonster.h"
#include "CAttackRangeEffect.h"
#include "Player.h"
#include "Network.h"   // g_monsters / send_hit_damage
#include <iostream>

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

    for (int i = 0; i < BOSS_ANIMATION_TRACKS; ++i)
        m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);

    m_pSkinnedAnimationController->SetTrackType(TRACK_IDLE, ANIMATION_TYPE_LOOP);
    m_pSkinnedAnimationController->SetTrackType(TRACK_WALK, ANIMATION_TYPE_LOOP);
    m_pSkinnedAnimationController->SetTrackType(TRACK_ATTACK01, ANIMATION_TYPE_ONCE);
    m_pSkinnedAnimationController->SetTrackType(TRACK_ATTACK02, ANIMATION_TYPE_ONCE);
    m_pSkinnedAnimationController->SetTrackType(TRACK_TAUNT, ANIMATION_TYPE_ONCE);
    m_pSkinnedAnimationController->SetTrackType(TRACK_DEATH, ANIMATION_TYPE_ONCE);

    // 처음엔 IDLE 트랙만 활성화
    for (int i = 0; i < BOSS_ANIMATION_TRACKS; ++i)
        m_pSkinnedAnimationController->SetTrackEnable(i, i == TRACK_IDLE);

    SetScale(1.0f, 1.0f, 1.0f);

    m_pHpbar = new Hpbar(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);

    /*
    SetMonsterID(id);
    g_monsters[id] = this;
    */

    CreateShaderVariables(pd3dDevice, pd3dCommandList);

    TransitionTo(BossState::Idle);
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

float CBossMonster::GetAnimationLength(int nAnimationSet) const
{
    if (!m_pSkinnedAnimationController || !m_pSkinnedAnimationController->m_pAnimationSets)
        return 1.0f;

    CAnimationSets* pSets = m_pSkinnedAnimationController->m_pAnimationSets;
    if (nAnimationSet < 0 || nAnimationSet >= pSets->m_nAnimationSets)
        return 1.0f;

    return pSets->m_pAnimationSets[nAnimationSet]->m_fLength;
}

void CBossMonster::ScheduleSlams(BossState newState)
{
    m_vPendingSlams.clear();
    m_nNextSlamIndex = 0;

    if (newState == BossState::Attack01)
    {
        // 한 번 내리꽂기: 클립의 70% 지점에서 타격되도록, 그 0.6초 전부터 경고가 차오름
        const float fWarmup = 0.6f;
        float fImpactAt = m_fStateDuration * 0.7f;
        m_vPendingSlams.push_back({ max(0.0f, fImpactAt - fWarmup), fWarmup });
    }
    else if (newState == BossState::Taunt)
    {
        // 여러 번 내리꽂기: 클립을 4등분해서 25%, 50%, 75% 지점마다 타격
        const float fWarmup = 0.4f;
        for (int i = 1; i <= 3; ++i)
        {
            float fImpactAt = m_fStateDuration * (i / 4.0f);
            m_vPendingSlams.push_back({ max(0.0f, fImpactAt - fWarmup), fWarmup });
        }
    }
    // Attack02(투사체)는 추후 구현 - 지금은 바닥 범위 표시 없음

    std::cout << "[Boss] ScheduleSlams: state=" << (int)newState
        << ", m_fStateDuration=" << m_fStateDuration
        << ", slams=" << m_vPendingSlams.size() << std::endl;
    for (size_t i = 0; i < m_vPendingSlams.size(); ++i)
    {
        std::cout << "  [" << i << "] spawnAt=" << m_vPendingSlams[i].fSpawnAt
            << " warmup=" << m_vPendingSlams[i].fWarmup << std::endl;
    }

}

void CBossMonster::TriggerSlam(float fWarmup)
{
    XMFLOAT3 pos = GetPosition();
    std::cout << "[Boss] TriggerSlam: warmup=" << fWarmup
        << ", pos=(" << pos.x << "," << pos.y << "," << pos.z << ")"
        << ", effect=" << (void*)m_pGroundAttackRangeEffect << std::endl;


    if (!m_pGroundAttackRangeEffect) return;

    m_pGroundAttackRangeEffect->Spawn(GetPosition(), m_fSlamRadius, fWarmup, m_xmf4SlamWarnColor);

    // TODO: 실제 데미지 판정은 fWarmup초 후(워밍업이 끝나 경고가 가득 찬 시점)에
    // GetPosition()을 중심으로 m_fSlamRadius 안에 플레이어가 있는지 검사해서 처리하세요.
}

int CBossMonster::ChooseRandomAttack() const
{
    // 가중치는 자유롭게 조절 가능: Attack01 45%, Attack02 45%, Taunt 10%
    int r = rand() % 100;
    if (r < 45) return 2; // Attack01
    if (r < 90) return 3; // Attack02
    return 4;             // Taunt
}

void CBossMonster::TransitionTo(BossState newState)
{
    if (m_eState == BossState::Death && newState != BossState::Death) return;
    if (!m_pSkinnedAnimationController) return;

    int oldTrack = TrackOf(m_eState);
    m_pSkinnedAnimationController->SetTrackEnable(oldTrack, false);

    m_eState = newState;
    int newTrack = TrackOf(newState);

    m_pSkinnedAnimationController->SetTrackPosition(newTrack, 0.0f);
    m_pSkinnedAnimationController->SetTrackEnable(newTrack, true);

    m_fStateTimer = 0.0f;

    if (newState == BossState::Idle)
        m_fStateDuration = m_fIdleDuration;
    else if (newState == BossState::Walk)
        // 지금은 자동 루프에서 호출되지 않음. 추후 기사 스킬 연동 시 외부에서 TransitionTo(Walk) 호출 용도
        m_fStateDuration = m_fWalkDuration;
    else
        // 공격/포효/죽음은 해당 클립의 실제 길이만큼 재생 후 다음 상태로 전환
        m_fStateDuration = GetAnimationLength(newTrack);

    ScheduleSlams(newState);
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

    if (m_eState == BossState::Death) return;

    m_fStateTimer += fTimeElapsed;

    // 이번 상태(Attack01/Taunt)에 예약된 슬램(바닥 공격범위) 트리거 체크
    // - early return보다 먼저 와야 함: 슬램은 상태가 끝나기 전(클립 70% 등) 중간에 터져야 하니까
    while (m_nNextSlamIndex < m_vPendingSlams.size() && m_fStateTimer >= m_vPendingSlams[m_nNextSlamIndex].fSpawnAt)
    {
        TriggerSlam(m_vPendingSlams[m_nNextSlamIndex].fWarmup);
        ++m_nNextSlamIndex;
    }

    if (m_fStateTimer < m_fStateDuration) return;

    if (m_eState == BossState::Idle)
    {
        // 3초 Idle가 끝나면 랜덤 패턴으로 공격(혹은 포효)
        switch (ChooseRandomAttack())
        {
        case 2: TransitionTo(BossState::Attack01); break;
        case 3: TransitionTo(BossState::Attack02); break;
        case 4: TransitionTo(BossState::Taunt);   break;
        }

        // 공격 판정/플레이어 데미지 처리는 여기서 트랙 전환 시점에 걸기
    }
    else
    {
        // 공격/포효 애니메이션이 끝났으면 다시 Idle로 복귀
        TransitionTo(BossState::Idle);
    }
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