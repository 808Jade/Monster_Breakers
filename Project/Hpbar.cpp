#include "stdafx.h"
#include "Hpbar.h"
#include "Shader.h"
#include "CRectMesh.h"

Hpbar::Hpbar(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
    CMesh* pMesh = new CRectMesh(pd3dDevice, pd3dCommandList, 1.0f, 0.15f, 0.0f, 0.0f, 0.0f);
    SetMesh(pMesh);

    CMaterial* pMaterial = new CMaterial(0);
    pMaterial->SetShader(CMaterial::m_pHpbarShader);
    pMaterial->m_xmf4AlbedoColor = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	SetMaterial(0, pMaterial);
}

Hpbar::~Hpbar()
{}

void Hpbar::Render(ID3D12GraphicsCommandList * pd3dCommandList, CCamera * pCamera)
{
    XMFLOAT4X4 saved = m_xmf4x4World;

    m_xmf4x4World._11 *= m_fHpRatio;
    float offset = (1.0f - m_fHpRatio) * 0.5f;
    m_xmf4x4World._41 -= offset;
    CGameObject::Render(pd3dCommandList, pCamera);

    m_xmf4x4World = saved;
}
