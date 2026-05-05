#pragma once
#include "stdafx.h"
#include "object.h"

class CParticle : public CGameObject
{
public:
    bool     m_bActive = false;
    XMFLOAT3 m_velocity = {};
    float    m_lifetime = 0.0f;
    float    m_maxLifetime = 2.0f;

    CParticle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
    void Activate(XMFLOAT3 pos, XMFLOAT3 direction);
    virtual void Animate(float fTimeElapsed);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* camera) override;
};

////////////////////////////////////////////////////////