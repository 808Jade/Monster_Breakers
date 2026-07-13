#pragma once
#include "Object.h"
#include "Hpbar.h"
#include "Shader.h"

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

    // 서버가 SC_P_BOSS_PATTERN / SC_P_BOSS_MOVE / SC_P_BOSS_DEATH 패킷으로
    // 상태를 결정해서 보내준다. 클라이언트는 그 상태에 맞는 애니메이션 트랙만 재생.
    void       TransitionTo(BossState newState);
    BossState  GetState() const { return m_eState; }

    // 공격 패턴 재생: 애니메이션 전환 + (Attack01/Attack02/Taunt 트랙이면) 공격범위 이펙트까지
    // 한 번에 처리한다. 이펙트 모양(원형/부채꼴)·색상·웜업은 newState에 따라 보스가 직접 결정한다.
    // xmf3Center/fRadius/fSweepAngleDeg : 서버 패킷(sc_packet_boss_pattern)에서 그대로 전달.
    // fSweepAngleDeg는 부채꼴(Taunt=SWEEP)에서만 사용되고, 원형 패턴에서는 무시된다.
    void PlayAttackPattern(BossState newState, const XMFLOAT3& xmf3Center, const XMFLOAT3& look, float fRadius, float fSweepAngleDeg = 0.0f);

    void  TakeDamage(float damage);
    float GetHP()      const { return m_fMonsterHP; }
    float GetHPRatio() const { return m_fHpRatio; }
    bool  IsDead()     const { return m_fMonsterHP <= 0.0f; }

    void SetLookDirection(const XMFLOAT3& xmf3Look)
    {
        XMFLOAT3 pos = GetPosition();
        XMFLOAT3 target = XMFLOAT3(pos.x + xmf3Look.x, pos.y + xmf3Look.y, pos.z + xmf3Look.z);
        XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
        LookAt(target, up);
    }

    void SetPlayer(CPlayer* p) { m_pPlayer = p; }
    void SetMonsterID(int id) { m_nMonsterID = id; }
    int  GetMonsterID() const { return m_nMonsterID; }

    // 바닥 공격범위(텔레그래프) 이펙트 풀. 소유권은 Scene에 있고, 보스는 포인터만 받아서 Spawn()만 호출한다.
    void SetGroundAttackRangeEffect(CGroundAttackRangeEffect* pEffect) { m_pGroundAttackRangeEffect = pEffect; }

    CTextureToScreenShader* m_pBossHpbar = NULL;

    void SetMaxHP(float hp)
    {
        m_fMaxHP = hp;
    }

    void SetHP(float hp)
    {
        m_fMonsterHP = hp;
        m_fHpRatio = m_fMonsterHP / m_fMaxHP;
    }

    virtual void Update(float fTimeElapsed);

    // 플레이어 HP바와 동일한 방식으로, 비율(newWidth)에 맞춰 hpbar 메시를 새로 만들어 교체한다.
    void SetHPWidth(float newWidth);
    bool IsHpbarVisible() const { return m_bHpbarVisible; }
    void SetHpbarVisible(bool bVisible) { m_bHpbarVisible = bVisible; }
    void ToggleHpbarVisible() { m_bHpbarVisible = !m_bHpbarVisible; }
private:
    int TrackOf(BossState s) const;

    // newState(애니메이션 트랙)에 따라 공격범위 이펙트를 스폰한다.
    // Idle/Walk/Death 등 비공격 상태에서는 아무 일도 하지 않는다.
    void SpawnAttackEffectFor(BossState newState, const XMFLOAT3& xmf3Center, float fRadius, float fSweepAngleDeg);

    CGroundAttackRangeEffect* m_pGroundAttackRangeEffect = nullptr;

    CPlayer* m_pPlayer = nullptr;
    int          m_nMonsterID = -1;

    float        m_fMonsterHP = 50000.f;
    float        m_fMaxHP = 50000.f;
    float        m_fHpRatio = 1.0f;

    BossState    m_eState = BossState::Idle;

    // hpbar 메시 재생성용으로 보관해두는 device/cmdList (플레이어 SetHPWidth와 동일한 방식)
    ID3D12Device* m_pd3dDevice = nullptr;
    ID3D12GraphicsCommandList* m_pd3dCommandList = nullptr;

    // 매 프레임 불필요한 mesh 재생성을 막기 위한 이전 폭 캐시
    float m_fPrevHpbarWidth = -1.0f;

    bool m_bHpbarVisible = false;

    float m_fWalkSoundTimer = 0.0f; // 걷는 소리 타이머
};