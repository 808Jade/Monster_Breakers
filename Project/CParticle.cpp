#include "CParticle.h"
#include "CCubeMesh.h"
#include "CCubeShader.h"
#include "Scene.h"
#include <random>

CParticle::CParticle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
    m_bActive = false;

    CCubeMesh *pMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 0.1f, 0.2f, 0.1f);
    SetMesh(pMesh);

    CreateShaderVariables(pd3dDevice, pd3dCommandList);

    CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
    pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/hp.dds", RESOURCE_TEXTURE2D, 0);

    CCubeShader *pShader = new CCubeShader();
    pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
    pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

    CScene::CreateShaderResourceViews(pd3dDevice, pTexture, 0, 15);

    CMaterial *pMaterial = new CMaterial(1);
    pMaterial->SetTexture(pTexture);
    pMaterial->SetShader(pShader);

    SetMaterial(0, pMaterial);
}

void CParticle::Activate(XMFLOAT3 pos, XMFLOAT3 direction)
{
    XMVECTOR vDir = XMVector3Normalize(XMLoadFloat3(&direction));
    const float speed = 15.0f;
    XMStoreFloat3(&m_velocity, vDir * XMVectorReplicate(speed));

	SetPosition(pos);
    m_lifetime = 0.0f;
    m_bActive = true;
}

void CParticle::Animate(float fTimeElapsed)
{
    if (!m_bActive) return;

    m_lifetime += fTimeElapsed;
    if (m_lifetime >= m_maxLifetime)
    {
        m_bActive = false;
        return;
    }

	XMFLOAT3 position = GetPosition();
    position.x += m_velocity.x * fTimeElapsed;
    position.y += m_velocity.y * fTimeElapsed;
    position.z += m_velocity.z * fTimeElapsed;
    SetPosition(position);
}

void CParticle::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* camera)
{
    if (!m_bActive) return;

    UpdateTransform(nullptr);
    CGameObject::Render(pd3dCommandList, camera);
}

////////////////////////////////////////////////////////////////////////////////
