#pragma once
#include "stdafx.h"

struct GroundCrackVertex
{
    XMFLOAT3 position;   
    XMFLOAT2 uv;         
    float     alpha;     
};

struct CrackSegment
{
    XMFLOAT3 dir;        
    XMFLOAT3 perp;       
    float     maxLen;   
    float     halfWidth; 
    float     startT;
};

class CCamera;

class CGroundCrackEffect
{
public:
    CGroundCrackEffect() = default;
    ~CGroundCrackEffect();

    void Create(ID3D12Device* pd3dDevice,
        ID3D12GraphicsCommandList* pd3dCommandList,
        ID3D12RootSignature* pd3dGraphicsRootSignature);

    // Q키 스킬 발동 시 호출
    void Trigger(const XMFLOAT3& playerPos, const XMFLOAT3& playerLook);

    void Update(float fTimeElapsed);

    void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

    bool IsActive() const { return m_bActive; }

private:
    void InitCrackSegments(const XMFLOAT3& origin, const XMFLOAT3& look);

    void UpdateVertices(float t);

    // 이펙트 상태
    bool      m_bActive = false;
    float     m_fTimer = 0.0f;    
    float     m_fDuration = 1.5f; 
    XMFLOAT3  m_xmf3Origin = {};

    // 균열 데이터
    static constexpr int MAIN_CRACK_COUNT = 8; 
    static constexpr int BRANCH_CRACK_COUNT = 8;  
    static constexpr int TOTAL_CRACKS = MAIN_CRACK_COUNT + BRANCH_CRACK_COUNT;

    // 각 세그먼트 = 쿼드 (4 verts, 6 indices)
    static constexpr int VERTS_PER_SEG = 4;
    static constexpr int INDEX_PER_SEG = 6;
    static constexpr int MAX_VERTS = TOTAL_CRACKS * VERTS_PER_SEG;
    static constexpr int MAX_INDICES = TOTAL_CRACKS * INDEX_PER_SEG;

    CrackSegment m_Cracks[TOTAL_CRACKS] = {};

    // D3D12 리소스
    ID3D12Resource* m_pVertexUploadBuffer = nullptr; 
    GroundCrackVertex* m_pMappedVerts = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_VBView = {};

    ID3D12Resource* m_pIndexBuffer = nullptr;
    D3D12_INDEX_BUFFER_VIEW  m_IBView = {};

    ID3D12PipelineState* m_pPipelineState = nullptr;

    // 루트 시그니처 참조 (CreateShader 후 PSO에서만 사용, 보관 불필요)
    // 단, 동일 루트 시그니처를 Render 시 명시적으로 다시 Set할 필요 없음
    // (Scene 레벨에서 이미 SetGraphicsRootSignature 되어 있음)
};