#pragma once
#include "Object.h"

class CTerrainPlayer;

class CWeaponThrowSystem
{
public:
    CWeaponThrowSystem() = default;
    ~CWeaponThrowSystem();

    void Create(ID3D12Device* pd3dDevice,
        ID3D12GraphicsCommandList* pd3dCommandList,
        ID3D12RootSignature* pd3dRootSignature);

    // pWeaponFrame : 플레이어의 SM_Weapon_01 프레임 (숨기기/복원용)
    void Emit(const XMFLOAT3& pos, const XMFLOAT3& dir, float speed,
        CGameObject* pWeaponFrame);

    bool IsActive() const { return m_bActive; }

    void Animate(float fTimeElapsed);
    void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

private:
    static constexpr float MAX_LIFETIME = 3.0f;
    static constexpr float ROTATION_SPEED = 360.0f;

    // 발사체 상태
    bool      m_bActive = false;
    XMFLOAT3  m_Position = {};
    XMFLOAT3  m_Direction = {};
    float     m_Speed = 0.0f;
    float     m_Lifetime = 0.0f;
    float     m_fRotationY = 0.0f;

    // 플레이어 손의 SM_Weapon_01 참조 (수명 만료 시 복원)
    CGameObject* m_pWeaponFrame = nullptr;

    // 날아가는 무기 렌더 오브젝트
    CGameObject* m_pWeaponObj = nullptr;
};