//-----------------------------------------------------------------------------
// File: DamageNumber.h
//
// 플레이어 공격이 몬스터에 명중했을 때, 명중 지점에 데미지 숫자가
// 위로 떠오르며 페이드아웃되는 월드 스페이스 빌보드 파티클.
//
// 기존 코드베이스의 CHpbarShader / CInteractPromptShader와 완전히 동일한
// 빌보드 기법(Shaders.hlsl의 VSHpbar: gmtxView의 행벡터로 카메라 right/up을
// 구해 로컬 XY를 밀어주는 방식)을 재사용한다. 새 정점 셰이더는 필요 없고,
// 색상 틴트를 위한 픽셀 셰이더 PSDamageNumber 하나만 Shaders.hlsl에 추가한다.
//
// 텍스처 바인딩도 CInteractPromptShader와 동일한 경로(픽셀 셰이더에서
// gtxtAlbedoTexture(t6) 샘플링, 루트 파라미터 인덱스 3에 SRV 테이블 바인딩)를
// 그대로 재사용하므로, 이미 검증된 바인딩 경로라 안전하다.
//
// ※ 가정: FontAtlas.dds(512x512, 밉맵 10)가 16x16 그리드(칸당 32px)의
//   ASCII 비트맵 폰트라고 가정하고 숫자 '0'~'9'의 UV 좌표를 계산한다.
//   실제 CText.cpp에 glyph rect 계산 함수가 있다면 그 상수와
//   FONT_ATLAS_COLS / FONT_ATLAS_ROWS 를 맞춰서 바꿔주면 된다.
//-----------------------------------------------------------------------------

#pragma once

#include "Object.h"
#include "Shader.h"

/////////////////////////////////////////////////////////////////////////////
// 폰트 아틀라스 그리드 상수 (가정값 - CText.cpp의 실제 값과 다르면 여기만 수정)
#define FONT_ATLAS_COLS		16
#define FONT_ATLAS_ROWS		16

/////////////////////////////////////////////////////////////////////////////
//
class CDamageNumberMesh : public CMesh
{
public:
	// pstrText: 화면에 표시할 문자열(숫자만 지원, 필요시 확장 가능)
	// fGlyphWidth/Height: 글자 하나의 로컬 스페이스 크기(월드 유닛)
	// fAdvance: 글자 사이 이동 폭(커닝, 보통 fGlyphWidth보다 살짝 작게)
	CDamageNumberMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		const std::string& strText, float fGlyphWidth = 0.4f, float fGlyphHeight = 0.6f, float fAdvance = 0.34f);
	virtual ~CDamageNumberMesh() {}

private:
	static void GetGlyphUV(char c, float& u0, float& v0, float& u1, float& v1);
};

/////////////////////////////////////////////////////////////////////////////
//
class CDamageNumberShader : public CShader
{
public:
	CDamageNumberShader() {}
	virtual ~CDamageNumberShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	void SetTexture(CTexture* pTexture);

private:
	CTexture* m_pTexture = NULL;
};

/////////////////////////////////////////////////////////////////////////////
// 하나의 떠오르는 데미지 숫자 인스턴스
class CDamageNumber : public CGameObject
{
public:
	CDamageNumber(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		CDamageNumberShader* pSharedShader, const XMFLOAT3& xmf3WorldPos,
		int nValue, const XMFLOAT4& xmf4Color, float fLifeTime = 1.0f, float fRiseSpeed = 1.3f);
	virtual ~CDamageNumber() {}

	void Tick(float fTimeElapsed);
	bool IsDead() const { return m_fElapsed >= m_fLifeTime; }

private:
	float m_fElapsed = 0.0f;
	float m_fLifeTime = 1.0f;
	float m_fRiseSpeed = 1.3f;
	XMFLOAT4 m_xmf4BaseColor = XMFLOAT4(1, 1, 1, 1);
};

/////////////////////////////////////////////////////////////////////////////
// 데미지 넘버 풀 관리자. CCollisionManager가 소유한다.
class CDamageNumberSystem
{
public:
	CDamageNumberSystem(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature);
	~CDamageNumberSystem();

	// xmf3HitPosition: 명중 지점(월드 좌표). 몬스터 머리 위쯤 오프셋을 주고 호출하면 된다.
	// bCritical: true면 색상/크기를 다르게 표시(선택 사항).
	void Spawn(const XMFLOAT3& xmf3HitPosition, int nDamage, bool bCritical = false);

	void Update(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

private:
	ID3D12Device* m_pd3dDevice = nullptr;
	ID3D12GraphicsCommandList* m_pd3dCommandList = nullptr;

	CDamageNumberShader* m_pShader = nullptr;
	std::vector<CDamageNumber*> m_Active;

	static const size_t MAX_ACTIVE = 128; // 과도한 스폰 방지용 상한
};