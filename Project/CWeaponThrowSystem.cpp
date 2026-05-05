#include "stdafx.h"
#include "CWeaponThrowSystem.h"
#include "Scene.h"

CWeaponThrowSystem::~CWeaponThrowSystem()
{
    if (m_pWeaponObj) { m_pWeaponObj->Release(); m_pWeaponObj = nullptr; }
}

void CWeaponThrowSystem::Create(
    ID3D12Device* pd3dDevice,
    ID3D12GraphicsCommandList* pd3dCommandList,
    ID3D12RootSignature* pd3dRootSignature)
{
    CLoadedModelInfo* pInfo = CGameObject::LoadGeometryAndAnimationFromFile(
        pd3dDevice, pd3dCommandList, pd3dRootSignature,
        "Model/Thief.bin", nullptr);

    if (pInfo)
    {
        // SM_Weapon_01 노드만 분리해서 사용
        m_pWeaponObj = pInfo->m_pModelRootObject->FindFrame("SM_Weapon_01");
        if (m_pWeaponObj) m_pWeaponObj->AddRef();
        delete pInfo;
    }
}

void CWeaponThrowSystem::Emit(const XMFLOAT3& pos, const XMFLOAT3& dir, float speed, CGameObject* pWeaponFrame)
{
    // 이미 날아가는 중이면 무시
    if (m_bActive) return;

    m_Position = pos;
    XMVECTOR vDir = XMVector3Normalize(XMLoadFloat3(&dir));
    XMStoreFloat3(&m_Direction, vDir);
    m_Speed = speed;
    m_Lifetime = MAX_LIFETIME;
    m_fRotationY = 0.0f;
    m_bActive = true;

    // 플레이어 손의 무기 숨기기
    m_pWeaponFrame = pWeaponFrame;
    if (m_pWeaponFrame)
        m_pWeaponFrame->SetVisible(false);
}

void CWeaponThrowSystem::Animate(float fTimeElapsed)
{
    if (!m_bActive) return;

    // 전진
    XMVECTOR vPos = XMLoadFloat3(&m_Position);
    XMVECTOR vDir = XMLoadFloat3(&m_Direction);
    vPos = XMVectorAdd(vPos, XMVectorScale(vDir, m_Speed * fTimeElapsed));
    XMStoreFloat3(&m_Position, vPos);
    
    // 회전
    m_fRotationY += ROTATION_SPEED * fTimeElapsed;
    if (m_fRotationY >= 360.0f) m_fRotationY -= 360.0f;

    // 수명 감소
    m_Lifetime -= fTimeElapsed;
    if (m_Lifetime <= 0.0f)
    {
        m_bActive = false;

        // 플레이어 손의 무기 복원
        if (m_pWeaponFrame)
        {
            m_pWeaponFrame->SetVisible(true);
            m_pWeaponFrame = nullptr;
        }
    }
}

void CWeaponThrowSystem::Render(
    ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (!m_bActive || !m_pWeaponObj) return;

    m_pWeaponObj->SetPosition(m_Position);
    m_pWeaponObj->UpdateTransform(nullptr);

    XMMATRIX matWorld = XMLoadFloat4x4(&m_pWeaponObj->m_xmf4x4World);
    XMMATRIX matRot = XMMatrixRotationX(XMConvertToRadians(m_fRotationY));
    XMStoreFloat4x4(&m_pWeaponObj->m_xmf4x4World, matRot * matWorld);

    m_pWeaponObj->Render(pd3dCommandList, pCamera);
}