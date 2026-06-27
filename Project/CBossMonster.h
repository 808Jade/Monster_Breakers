#pragma once
#include "Object.h"
#include "Hpbar.h"

class CPlayer;

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

    void  TakeDamage(float damage);
    float GetHP()      const { return m_fMonsterHP; }
    float GetHPRatio() const { return m_fHpRatio; }
    bool  IsDead()     const { return m_fMonsterHP <= 0.0f; }

    void SetPlayer(CPlayer* p) { m_pPlayer = p; }
    void SetMonsterID(int id) { m_nMonsterID = id; }
    int  GetMonsterID() const { return m_nMonsterID; }

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
    int TrackOf(BossState s) const;

    CPlayer* m_pPlayer = nullptr;
    int          m_nMonsterID = -1;

    float        m_fMonsterHP = 50000.f;
    float        m_fMaxHP = 50000.f;
    float        m_fHpRatio = 1.0f;

    BossState    m_eState = BossState::Idle;
};