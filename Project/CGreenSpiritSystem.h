#pragma once
#include "stdafx.h"
#include "Shader.h"
#include "Mesh.h"
#include "CFireballSystem.h"   // FireballParticleData 구조체 재사용

class CGreenSpiritSystem
{
public:
    static const int MAX_PARTICLES = 256;

    CGreenSpiritSystem(
        ID3D12Device* pd3dDevice,
        ID3D12GraphicsCommandList* pd3dCommandList,
        ID3D12RootSignature* pd3dRootSignature);

    ~CGreenSpiritSystem();

    // position: 플레이어 발 위치 (바닥)
    void Emit(XMFLOAT3 position);
    void Animate(float fTimeElapsed);
    void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
    void ReleaseUploadBuffers() {}

private:
    void CreateQuadMesh(ID3D12Device* pd3dDevice);
    void CreateParticleBuffer(ID3D12Device* pd3dDevice);
    void UploadToGPU(ID3D12GraphicsCommandList* pd3dCommandList);

private:
    FireballParticleData  m_Particles[MAX_PARTICLES] = {};
    int   m_nNextSlot = 0;
    bool  m_bNeedUpload = false;

    ID3D12Resource* m_pParticleUploadBuffer = nullptr;
    FireballParticleData* m_pMappedData = nullptr;
    ID3D12Resource* m_pParticleDefaultBuffer = nullptr;

    ID3D12Resource* m_pQuadVB = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_QuadVBView = {};

    CShader* m_pShader = nullptr;
    CTexture* m_pTexture = nullptr;
};