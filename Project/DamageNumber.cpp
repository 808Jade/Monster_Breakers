#include "stdafx.h"
#include "DamageNumber.h"
#include "Scene.h"

void CDamageNumberMesh::GetGlyphUV(char c, float& u0, float& v0, float& u1, float& v1)
{
	const float fCell = 1.0f / float(FONT_ATLAS_COLS); // FONT_ATLAS_COLS == FONT_ATLAS_ROWS 가정(정사각 그리드)

	unsigned char nIndex = (unsigned char)c;
	int nCol = nIndex % FONT_ATLAS_COLS;
	int nRow = nIndex / FONT_ATLAS_COLS;

	u0 = nCol * fCell;
	v0 = nRow * fCell;
	u1 = u0 + fCell;
	v1 = v0 + fCell;
}

CDamageNumberMesh::CDamageNumberMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	const std::string& strText, float fGlyphWidth, float fGlyphHeight, float fAdvance)
	: CMesh(pd3dDevice, pd3dCommandList)
{
	int nChars = (int)strText.length();
	if (nChars <= 0) nChars = 1; // 방어적으로 최소 1글자(공백) 처리

	m_nVertices = nChars * 6;
	m_nStride = sizeof(CTexturedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	std::vector<CTexturedVertex> vVertices;
	vVertices.reserve(m_nVertices);

	float fTotalWidth = (nChars - 1) * fAdvance + fGlyphWidth;
	float fStartX = -fTotalWidth * 0.5f;
	float fY0 = -fGlyphHeight * 0.5f;
	float fY1 = fGlyphHeight * 0.5f;

	for (int i = 0; i < nChars; ++i)
	{
		char c = (i < (int)strText.length()) ? strText[i] : ' ';

		float u0, v0, u1, v1;
		GetGlyphUV(c, u0, v0, u1, v1);

		float fX0 = fStartX + i * fAdvance;
		float fX1 = fX0 + fGlyphWidth;

		vVertices.push_back(CTexturedVertex(XMFLOAT3(fX0, fY1, 0.0f), XMFLOAT2(u0, v0))); // TL
		vVertices.push_back(CTexturedVertex(XMFLOAT3(fX1, fY1, 0.0f), XMFLOAT2(u1, v0))); // TR
		vVertices.push_back(CTexturedVertex(XMFLOAT3(fX1, fY0, 0.0f), XMFLOAT2(u1, v1))); // BR

		vVertices.push_back(CTexturedVertex(XMFLOAT3(fX0, fY1, 0.0f), XMFLOAT2(u0, v0))); // TL
		vVertices.push_back(CTexturedVertex(XMFLOAT3(fX1, fY0, 0.0f), XMFLOAT2(u1, v1))); // BR
		vVertices.push_back(CTexturedVertex(XMFLOAT3(fX0, fY0, 0.0f), XMFLOAT2(u0, v1))); // BL
	}

	UINT nBufferBytes = m_nStride * m_nVertices;

	// UPLOAD 힙에 바로 만들고 Map()으로 CPU에서 직접 써넣는다(커맨드 리스트에
	// 아무 것도 기록하지 않으므로, Update() 도중 아무 때나 생성해도 안전하다).
	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, nBufferBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	void* pMappedData = NULL;
	m_pd3dPositionBuffer->Map(0, NULL, &pMappedData);
	memcpy(pMappedData, vVertices.data(), nBufferBytes);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = m_nStride;
	m_d3dPositionBufferView.SizeInBytes = nBufferBytes;
}

CDamageNumberShader::~CDamageNumberShader()
{
	if (m_pTexture) m_pTexture->Release();
}

D3D12_INPUT_LAYOUT_DESC CDamageNumberShader::CreateInputLayout()
{
	UINT n = 2;
	D3D12_INPUT_ELEMENT_DESC* p = new D3D12_INPUT_ELEMENT_DESC[n];

	p[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	p[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d;
	d.pInputElementDescs = p;
	d.NumElements = n;
	return d;
}

D3D12_DEPTH_STENCIL_DESC CDamageNumberShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d;
	::ZeroMemory(&d, sizeof(d));
	d.DepthEnable = TRUE;
	d.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	d.StencilEnable = FALSE;
	return d;
}

D3D12_BLEND_DESC CDamageNumberShader::CreateBlendState()
{
	D3D12_BLEND_DESC d;
	::ZeroMemory(&d, sizeof(d));
	d.RenderTarget[0].BlendEnable = TRUE;
	d.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	return d;
}

D3D12_RASTERIZER_DESC CDamageNumberShader::CreateRasterizerState()
{
	auto rs = CShader::CreateRasterizerState();
	rs.CullMode = D3D12_CULL_MODE_NONE;
	return rs;
}

D3D12_SHADER_BYTECODE CDamageNumberShader::CreateVertexShader()
{
	return CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSInteractPrompt", "vs_5_1", &m_pd3dVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CDamageNumberShader::CreatePixelShader()
{
	return CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSDamageNumber", "ps_5_1", &m_pd3dPixelShaderBlob);
}

void CDamageNumberShader::SetTexture(CTexture* pTexture)
{
	if (m_pTexture) m_pTexture->Release();
	m_pTexture = pTexture;
	if (m_pTexture) m_pTexture->AddRef();
}

void CDamageNumberShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);

	if (m_pTexture) m_pTexture->UpdateShaderVariable(pd3dCommandList, 0, 0);
}

CDamageNumber::CDamageNumber(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	CDamageNumberShader* pSharedShader, const XMFLOAT3& xmf3WorldPos,
	int nValue, const XMFLOAT4& xmf4Color, float fLifeTime, float fRiseSpeed)
	: CGameObject(1)
{
	m_fLifeTime = fLifeTime;
	m_fRiseSpeed = fRiseSpeed;
	m_xmf4BaseColor = xmf4Color;

	std::string strText = std::to_string(nValue);

	CDamageNumberMesh* pMesh = new CDamageNumberMesh(pd3dDevice, pd3dCommandList, strText);
	SetMesh(pMesh);

	CMaterial* pMaterial = new CMaterial(0);
	pMaterial->SetShader(pSharedShader); // AddRef() 걸림. 시스템도 별도로 AddRef()를 갖고 있으므로
	// 이 숫자가 소멸해도 공용 셰이더는 살아남는다.
	pMaterial->m_xmf4AlbedoColor = xmf4Color; // .a = 1.0로 시작, Tick()에서 서서히 감소
	SetMaterial(0, pMaterial);

	SetPosition(xmf3WorldPos);
}

void CDamageNumber::Tick(float fTimeElapsed)
{
	m_fElapsed += fTimeElapsed;

	XMFLOAT3 xmf3Pos = GetPosition();
	xmf3Pos.y += m_fRiseSpeed * fTimeElapsed;
	SetPosition(xmf3Pos);

	float t = m_fElapsed / m_fLifeTime;
	const float fFadeStart = 0.55f;
	float fAlpha = 1.0f;
	if (t > fFadeStart)
		fAlpha = 1.0f - (t - fFadeStart) / (1.0f - fFadeStart);
	if (fAlpha < 0.0f) fAlpha = 0.0f;

	CMaterial* pMat = GetMaterial(0);
	if (pMat) pMat->m_xmf4AlbedoColor = XMFLOAT4(m_xmf4BaseColor.x, m_xmf4BaseColor.y, m_xmf4BaseColor.z, fAlpha);
}

CDamageNumberSystem::CDamageNumberSystem(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_pd3dDevice = pd3dDevice;
	m_pd3dCommandList = pd3dCommandList;

	m_pShader = new CDamageNumberShader();
	// 중요: 시스템 자신의 소유권 몫으로 AddRef()를 걸어야 한다.
	// 그렇지 않으면 셰이더의 참조 카운트는 오직 "현재 화면에 떠 있는 CDamageNumber들의
	// 머티리얼이 걸어둔 AddRef()"에만 의존하게 되고, 활성 데미지 숫자가 0개가 되는
	// 순간(마지막 숫자가 소멸하며 그 머티리얼이 Release() -> 0) 셰이더가 delete되어
	// 버린다. 그 상태에서 다음 타격이 들어오면 이미 죽은 셰이더/텍스처를 다시 쓰다가
	// 드라이버 크래시가 난다. (실제로 이 버그였음 - 로그로 확인됨)
	m_pShader->AddRef();
	m_pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pFontTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pFontTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/FontAtlas.dds", RESOURCE_TEXTURE2D, 0);

	CScene::CreateShaderResourceViews(pd3dDevice, pFontTexture, 0, 3);

	m_pShader->SetTexture(pFontTexture);
	// 주의: CTexture도 new 직후 참조 카운트가 0부터 시작하고 SetTexture()의 AddRef()로
	// 비로소 1이 된다. 여기서 추가로 Release()를 호출하면 안 된다.
}

CDamageNumberSystem::~CDamageNumberSystem()
{
	for (CDamageNumber* p : m_Active) delete p;
	m_Active.clear();

	if (m_pShader) m_pShader->Release(); // 생성자에서 걸어둔 시스템 몫의 AddRef()를 해제
}

void CDamageNumberSystem::Spawn(const XMFLOAT3& xmf3HitPosition, int nDamage, bool bCritical)
{
	if (m_Active.size() >= MAX_ACTIVE)
	{
		delete m_Active.front();
		m_Active.erase(m_Active.begin());
	}

	XMFLOAT4 xmf4Color = bCritical ? XMFLOAT4(1.0f, 0.55f, 0.05f, 1.0f)
		: XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	CDamageNumber* pNumber = new CDamageNumber(m_pd3dDevice, m_pd3dCommandList, m_pShader,
		xmf3HitPosition, nDamage, xmf4Color, /*fLifeTime*/1.0f, /*fRiseSpeed*/1.3f);

	m_Active.push_back(pNumber);
}

void CDamageNumberSystem::Update(float fTimeElapsed)
{
	for (size_t i = 0; i < m_Active.size(); )
	{
		m_Active[i]->Tick(fTimeElapsed);

		if (m_Active[i]->IsDead())
		{
			delete m_Active[i];
			m_Active.erase(m_Active.begin() + i);
		}
		else
		{
			++i;
		}
	}
}

void CDamageNumberSystem::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	for (CDamageNumber* pNumber : m_Active)
	{
		pNumber->Render(pd3dCommandList, pCamera);
	}
}