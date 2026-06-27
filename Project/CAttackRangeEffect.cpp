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
        pMaterial->m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); // x=부채꼴여부, y=정면각, z=half-angle
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

// CRectMesh(가로 2, 높이 2)는 로컬 XY 평면에 만들어지고, u = (x + 1) / 2, v = (y + 1) / 2로 매핑된다고 가정한다.
// PlaceFlatOnGround()가 RotationX(+90도)로 눕히므로(DirectXMath 행벡터 기준 검증함):
//   로컬 +X -> 월드 +X, 로컬 +Y -> 월드 +Z. 부호 반전 없이 그대로 대응된다.
// 즉 월드 방향(dx, dz)을 로컬 평면 각도로 바꾸려면 atan2(dx, dz)를 그대로 쓰면 된다.
// (만약 실제로 좌우/앞뒤가 뒤집혀 보이면 이 한 줄의 부호만 맞추면 된다.)
float CGroundAttackRangeEffect::WorldDirectionToLocalAngle(const XMFLOAT3 & xmf3Direction)
{
    float dx = xmf3Direction.x;
    float dz = xmf3Direction.z;
    float lenSq = dx * dx + dz * dz;
    if (lenSq < 1e-8f) return 0.0f; // 방향이 없으면 기본값(로컬 +X)

    return atan2f(dx, dz);
}

int CGroundAttackRangeEffect::AcquireSlot()
{
    if (m_vIndicators.empty()) return -1;

    // 비어있는(비활성) 슬롯을 우선 찾고, 없으면 라운드로빈으로 가장 오래된 것을 덮어쓴다.
    for (size_t i = 0; i < m_vIndicators.size(); ++i)
    {
        if (!m_vIndicators[i].bActive) return (int)i;
    }

    int useIndex = m_nNextIndex;
    m_nNextIndex = (m_nNextIndex + 1) % (int)m_vIndicators.size();
    return useIndex;
}

void CGroundAttackRangeEffect::Spawn(const XMFLOAT3& xmf3Center, float fRadius, float fWarmupTime, const XMFLOAT4& xmf4Color)
{
    int useIndex = AcquireSlot();
    if (useIndex < 0) return;

    Indicator& ind = m_vIndicators[useIndex];
    if (!ind.pObject) return;

    ind.bActive = true;
    ind.fTimer = 0.0f;
    ind.fWarmupTime = max(0.05f, fWarmupTime);
    ind.xmf3Color = XMFLOAT3(xmf4Color.x, xmf4Color.y, xmf4Color.z);
    ind.bSector = false; // 원형
    ind.fFacingAngle = 0.0f;
    ind.fHalfAngle = XM_PI;

    PlaceFlatOnGround(ind.pObject, xmf3Center, fRadius);
    ind.pObject->SetVisible(true);
}

void CGroundAttackRangeEffect::Spawn(const XMFLOAT3& xmf3Center, float fRadius, float fWarmupTime,
    const XMFLOAT3& xmf3Direction, float fHalfAngleDeg, const XMFLOAT4& xmf4Color)
{
    int useIndex = AcquireSlot();
    if (useIndex < 0) return;

    Indicator& ind = m_vIndicators[useIndex];
    if (!ind.pObject) return;

    ind.bActive = true;
    ind.fTimer = 0.0f;
    ind.fWarmupTime = max(0.05f, fWarmupTime);
    ind.xmf3Color = XMFLOAT3(xmf4Color.x, xmf4Color.y, xmf4Color.z);
    ind.bSector = true;
    ind.fFacingAngle = WorldDirectionToLocalAngle(xmf3Direction);
    ind.fHalfAngle = XMConvertToRadians(max(1.0f, fHalfAngleDeg));

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

            // 부채꼴 파라미터를 emissive 채널에 실어서 PSGroundRange로 전달
            // x = 부채꼴 모드(1)/원형(0), y = 정면각(라디안), z = half-angle(라디안)
            pMat->m_xmf4EmissiveColor = XMFLOAT4(
                ind.bSector ? 1.0f : 0.0f,
                ind.fFacingAngle,
                ind.fHalfAngle,
                0.0f);
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