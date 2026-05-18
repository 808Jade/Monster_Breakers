#pragma once

#include "stdafx.h"
#include "Shader.h"
#include "Camera.h"

struct BeamVertex
{
    XMFLOAT3 m_xmf3Position;
    XMFLOAT2 m_xmf2TexCoord;
};

class CBeamSystem
{
public:
    CBeamSystem();
    ~CBeamSystem();

    void Create(
        ID3D12Device* pd3dDevice,
        ID3D12GraphicsCommandList* pd3dCommandList,
        ID3D12RootSignature* pd3dRootSignature);

    void Emit(const XMFLOAT3& start, const XMFLOAT3& end);
    void Animate(float fTimeElapsed);
    void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

private:
    void CreateVertexBuffer(ID3D12Device* pd3dDevice);

private:
    ID3D12Resource* m_pVertexUploadBuffer = nullptr;
    BeamVertex* m_pMappedVertices = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView{};

    CShader* m_pShader = nullptr;

    XMFLOAT3 m_Start{};
    XMFLOAT3 m_End{};

    bool m_bActive = false;
    float m_fLifeTime = 0.0f;
    float m_fMaxLifeTime = 2.5f;
    float m_fWidth = 0.25f;
};

