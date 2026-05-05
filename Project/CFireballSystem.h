#pragma once
#include "stdafx.h"  
#include "Shader.h"  
#include "Mesh.h"   

struct FireballParticleData
{
    XMFLOAT3 position;      // 월드 위치
    float     size;         // 빌보드 스케일 (m 단위)
    XMFLOAT3 velocity;      // m/s
    float     lifetime;     // 현재 경과 시간
    float     maxLifetime;  // 수명 상한
    float     uvOffset;     // 스프라이트 시트 V 스크롤 오프셋
    UINT      active;       // 0=비활성, 1=활성 (shader에서 discard 판별)
    float     pad;          // 16바이트 정렬
};

class CFireballSystem
{
public:
    static const int MAX_PARTICLES = 512;

    CFireballSystem(
        ID3D12Device* pd3dDevice,
        ID3D12GraphicsCommandList* pd3dCommandList,
        ID3D12RootSignature* pd3dRootSignature);

    ~CFireballSystem();

    void Emit(XMFLOAT3 position, XMFLOAT3 direction, float speed = 15.0f);
    void Animate(float fTimeElapsed);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
    void ReleaseUploadBuffers();

private:
    void CreateQuadMesh(ID3D12Device* pd3dDevice);
    void CreateParticleBuffer(ID3D12Device* pd3dDevice);
    void UploadToGPU(ID3D12GraphicsCommandList* pd3dCommandList);

private:
    FireballParticleData m_Particles[MAX_PARTICLES] = {};
    int  m_nNextSlot = 0;
    bool m_bNeedUpload = false;

    ID3D12Resource* m_pParticleUploadBuffer = nullptr;
    FireballParticleData* m_pMappedData = nullptr;

    ID3D12Resource* m_pParticleDefaultBuffer = nullptr;

    ID3D12Resource* m_pQuadVB = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_QuadVBView = {};

    CShader* m_pShader = nullptr;
    CTexture* m_pTexture = nullptr;
};