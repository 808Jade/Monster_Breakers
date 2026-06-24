#pragma once
#include "Object.h"
#include "Hpbar.h"

class CPlayer;
class CGroundAttackRangeEffect;

enum class BossState
{
    Idle,
    Walk,
    Attack01,
    Attack02,
    Taunt,
    Death
};

class CBossMonster : public CGameObject
{
public:
    CBossMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
        ID3D12RootSignature* pd3dGraphicsRootSignature,
        const char* pstrModelPath,
        CLoadedModelInfo* pModel = nullptr, float fMaxHP = 50000.f, int id = -1);
    virtual ~CBossMonster();

    virtual void Animate(float fTimeElapsed) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL) override;

    void       TransitionTo(BossState newState);
    BossState  GetState() const { return m_eState; }

    void  TakeDamage(float damage);
    float GetHP()      const { return m_fMonsterHP; }
    float GetHPRatio() const { return m_fHpRatio; }
    bool  IsDead()     const { return m_fMonsterHP <= 0.0f; }

    void SetPlayer(CPlayer* p) { m_pPlayer = p; }
    void SetMonsterID(int id) { m_nMonsterID = id; }
    int  GetMonsterID() const { return m_nMonsterID; }

    // 바닥 공격범위(슬램 경고) 이펙트 풀. Scene에서 만들어서 넘겨준다.
    void SetGroundAttackRangeEffect(CGroundAttackRangeEffect* pEffect) { m_pGroundAttackRangeEffect = pEffect; }

    // 내리꽂기(슬램) 공격범위 반지름 / 경고 색상 - 필요하면 외부에서 조절
    void SetSlamRadius(float fRadius) { m_fSlamRadius = fRadius; }
    void SetSlamWarnColor(const XMFLOAT4& c) { m_xmf4SlamWarnColor = c; }

    // Idle 유지 시간(초). 기본 3초 - 이 시간 동안 대기하다가 랜덤 공격으로 전환됨.
    void  SetIdleDuration(float fSeconds) { m_fIdleDuration = fSeconds; }
    float GetIdleDuration() const { return m_fIdleDuration; }

    // Walk 유지 시간(초). 지금은 자동 루프에서 쓰이지 않음 - 추후 기사 스킬 사용 시
    // TransitionTo(BossState::Walk)를 외부에서 호출하는 용도로 미리 준비해 둔 것.
    void  SetWalkDuration(float fSeconds) { m_fWalkDuration = fSeconds; }
    float GetWalkDuration() const { return m_fWalkDuration; }

    Hpbar* m_pHpbar = nullptr;

    void ResetHP()
    {
        m_fMonsterHP = m_fMaxHP;
        m_fHpRatio = 1.0f;
        TransitionTo(BossState::Idle);
    }

    void SetHP(float hp)
    {
        m_fMonsterHP = hp;
        m_fHpRatio = m_fMonsterHP / m_fMaxHP;
    }

private:
    int   TrackOf(BossState s) const;
    int   ChooseRandomAttack() const;          // 0=Attack01, 1=Attack02, 2=Taunt
    float GetAnimationLength(int nAnimationSet) const;

    // 한 상태(Attack01/Taunt) 안에서 "언제 바닥을 내리찍는지" 예약 정보
    struct SlamSchedule
    {
        float fSpawnAt;  // 상태 진입 후 이 시간(초)에 경고 범위 표시 시작
        float fWarmup;   // 경고가 뜬 뒤 실제 타격까지 걸리는 시간(초)
    };
    void ScheduleSlams(BossState newState);    // TransitionTo에서 호출, m_vPendingSlams 채움
    void TriggerSlam(float fWarmup);           // 현재 위치에 바닥 공격범위 이펙트 하나 생성

    std::vector<SlamSchedule> m_vPendingSlams;
    size_t                     m_nNextSlamIndex = 0;

    CGroundAttackRangeEffect* m_pGroundAttackRangeEffect = nullptr;
    float                      m_fSlamRadius = 4.0f;
    XMFLOAT4                   m_xmf4SlamWarnColor = XMFLOAT4(1.0f, 0.2f, 0.05f, 1.0f);

    CPlayer* m_pPlayer = nullptr;
    int          m_nMonsterID = -1;

    float        m_fMonsterHP = 50000.f;
    float        m_fMaxHP = 50000.f;
    float        m_fHpRatio = 1.0f;

    BossState    m_eState = BossState::Idle;

    float        m_fStateTimer = 0.0f;     // 현재 상태에 머문 시간
    float        m_fStateDuration = 0.0f;  // 현재 상태가 끝나는 시간(Idle/Walk=고정 대기시간, 공격/포효=클립 길이)
    float        m_fIdleDuration = 3.0f;   // Idle 유지 시간 (대기 후 랜덤 공격)
    float        m_fWalkDuration = 3.0f;   // Walk 유지 시간 (지금은 미사용 - 추후 기사 스킬 연동용)
};