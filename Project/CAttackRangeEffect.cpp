#include "stdafx.h"
#include "CAttackRangeEffect.h"
#include "CRectMesh.h"

CGroundAttackRangeEffect::CGroundAttackRangeEffect()
{}

CGroundAttackRangeEffect::~CGroundAttackRangeEffect()
{
    Release();
}

void CGroundAttackRangeEffect::Create(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
    ID3D12RootSignature* pd3dGraphicsRootSignature, int nPoolSize)
{
    m_vIndicators.clear();
    m_vIndicators.resize(nPoolSize);

    for (int i = 0; i < nPoolSize; ++i)
    {
        // -1..1 범위의 평평한 사각 메쉬. PlaceFlatOnGround에서 SetScale(radius)로 실제 반지름을 맞춘다.
        CMesh* pMesh = new CRectMesh(pd3dDevice, pd3dCommandList, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f);

        CGameObject* pObject = new CGameObject(1);
        pObject->SetMesh(pMesh);

        CMaterial* pMaterial = new CMaterial(0);
        pMaterial->SetShader(CMaterial::m_pGroundRangeShader);
        pMaterial->m_xmf4AlbedoColor = XMFLOAT4(1.0f, 0.15f, 0.05f, 0.0f); // 처음엔 안 보이게(alpha=0)
        pMaterial->m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
        pObject->SetMaterial(0, pMaterial);

        pObject->SetVisible(false);

        m_vIndicators[i].pObject = pObject;
        m_vIndicators[i].bActive = false;
    }

    m_nNextIndex = 0;
}

void CGroundAttackRangeEffect::Release()
{
    for (auto& ind : m_vIndicators)
    {
        if (ind.pObject) ind.pObject->Release();
        ind.pObject = nullptr;
    }
    m_vIndicators.clear();
}

void CGroundAttackRangeEffect::ReleaseUploadBuffers()
{
    for (auto& ind : m_vIndicators)
        if (ind.pObject) ind.pObject->ReleaseUploadBuffers();
}

// 매번 ToParent 행렬을 새로 구성한다(SetScale/Rotate는 누적형이라 재사용 풀에는 부적합).
void CGroundAttackRangeEffect::PlaceFlatOnGround(CGameObject* pObject, const XMFLOAT3& xmf3Center, float fRadius)
{
    XMMATRIX mtxScale = XMMatrixScaling(fRadius, 1.0f, fRadius);
    XMMATRIX mtxFlat = XMMatrixRotationX(XMConvertToRadians(90.0f)); // 카메라용 수직 평면 -> 바닥에 눕히기
    XMMATRIX mtxTranslate = XMMatrixTranslation(xmf3Center.x, xmf3Center.y + 0.05f, xmf3Center.z); // z-fighting 방지용 살짝 띄움

    XMMATRIX mtxLocal = mtxScale * mtxFlat * mtxTranslate;
    XMStoreFloat4x4(&pObject->m_xmf4x4ToParent, mtxLocal);

    pObject->UpdateTransform(NULL);
}

void CGroundAttackRangeEffect::Spawn(const XMFLOAT3& xmf3Center, float fRadius, float fWarmupTime, const XMFLOAT4& xmf4Color)
{
    if (m_vIndicators.empty()) return;

    // 비어있는(비활성) 슬롯을 우선 찾고, 없으면 라운드로빈으로 가장 오래된 것을 덮어쓴다.
    int useIndex = -1;
    for (size_t i = 0; i < m_vIndicators.size(); ++i)
    {
        if (!m_vIndicators[i].bActive) { useIndex = (int)i; break; }
    }
    if (useIndex < 0)
    {
        useIndex = m_nNextIndex;
        m_nNextIndex = (m_nNextIndex + 1) % (int)m_vIndicators.size();
    }

    Indicator& ind = m_vIndicators[useIndex];
    if (!ind.pObject) return;

    ind.bActive = true;
    ind.fTimer = 0.0f;
    ind.fWarmupTime = max(0.05f, fWarmupTime);
    ind.xmf3Color = XMFLOAT3(xmf4Color.x, xmf4Color.y, xmf4Color.z);

    PlaceFlatOnGround(ind.pObject, xmf3Center, fRadius);
    ind.pObject->SetVisible(true);
}

void CGroundAttackRangeEffect::Animate(float fTimeElapsed)
{
    for (auto& ind : m_vIndicators)
    {
        if (!ind.bActive || !ind.pObject) continue;

        ind.fTimer += fTimeElapsed;
        float fTotal = ind.fWarmupTime + ind.fFadeOutTime;

        if (ind.fTimer >= fTotal)
        {
            ind.bActive = false;
            ind.pObject->SetVisible(false);
            continue;
        }

        float progress, fade;
        if (ind.fTimer < ind.fWarmupTime)
        {
            progress = ind.fTimer / ind.fWarmupTime;       // 0 -> 1로 차오름
            fade = min(1.0f, progress * 3.0f);             // 등장 시 살짝 페이드인
        }
        else
        {
            progress = 1.0f;
            float t = (ind.fTimer - ind.fWarmupTime) / ind.fFadeOutTime;
            fade = 1.0f - t;                               // 타격 순간 -> 빠르게 페이드아웃
        }

        CMaterial* pMat = ind.pObject->GetMaterial(0);
        if (pMat)
        {
            pMat->m_xmf4AlbedoColor = XMFLOAT4(ind.xmf3Color.x, ind.xmf3Color.y, ind.xmf3Color.z, fade);
            pMat->m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, progress);
        }
    }
}

void CGroundAttackRangeEffect::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    for (auto& ind : m_vIndicators)
    {
        if (ind.bActive && ind.pObject && ind.pObject->GetVisible())
            ind.pObject->Render(pd3dCommandList, pCamera);
    }
}