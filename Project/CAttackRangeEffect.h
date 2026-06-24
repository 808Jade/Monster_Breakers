#pragma once
#include "Object.h"

class CGroundAttackRangeEffect
{
public:
    CGroundAttackRangeEffect();
    ~CGroundAttackRangeEffect();

    void Create(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
        ID3D12RootSignature* pd3dGraphicsRootSignature, int nPoolSize = 4);
    void Release();
    void ReleaseUploadBuffers();

    // xmf3Center   : 바닥 중심 위치(보통 공격자의 현재 위치, Y는 지면 높이)
    // fRadius      : 공격범위 반지름
    // fWarmupTime  : 경고가 표시된 후 실제로 타격되기까지 걸리는 시간(초). 이 시간 동안 원이 가운데서부터 차오름
    // xmf4Color    : 경고 색상(RGB). A는 내부적으로 페이드용으로 덮어쓰므로 의미 없음
    void Spawn(const XMFLOAT3& xmf3Center, float fRadius, float fWarmupTime,
        const XMFLOAT4& xmf4Color = XMFLOAT4(1.0f, 0.15f, 0.05f, 1.0f));

    void Animate(float fTimeElapsed);
    void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

private:
    struct Indicator
    {
        CGameObject* pObject = nullptr;
        bool         bActive = false;
        float        fTimer = 0.0f;
        float        fWarmupTime = 1.0f;
        float        fFadeOutTime = 0.18f; // 타격 순간 이후 사라지는 시간
        XMFLOAT3     xmf3Color = XMFLOAT3(1.0f, 0.15f, 0.05f);
    };

    void PlaceFlatOnGround(CGameObject* pObject, const XMFLOAT3& xmf3Center, float fRadius);

    std::vector<Indicator> m_vIndicators;
    int                     m_nNextIndex = 0; // 풀이 가득 찼을 때 다음에 재사용할 인덱스(라운드로빈)
};