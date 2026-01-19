//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "Scene.h"
#include "Network.h"
#include "GameFramework.h"

extern CGameFramework gGameFramework;

ID3D12DescriptorHeap *CScene::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorNextHandle;

CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::BuildDefaultLightsAndMaterials(bool toggle)
{
	m_nLights = 1;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	//m_xmf4GlobalAmbient = XMFLOAT4(0.08f, 0.08f, 0.08f, 1.0f);
	m_xmf4GlobalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	//m_pLights[0].m_bEnable = true;
	//m_pLights[0].m_nType = POINT_LIGHT;
	//m_pLights[0].m_fRange = 17.0f;
	//m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	//m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	//m_pLights[0].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	//m_pLights[0].m_xmf3Position = XMFLOAT3(3.0f, 5.0f, 30.0f);
	//m_pLights[0].m_xmf3Attenuation = XMFLOAT3(0.5f, 0.05f, 0.0001f);

	m_pLights[0].m_bEnable = toggle;
	m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	XMFLOAT3 lightDir = XMFLOAT3(0.5f, -1.0f, 0.5f); // 약간 대각선 아래로
	XMVECTOR vLightDir = XMLoadFloat3(&lightDir);
	vLightDir = XMVector3Normalize(vLightDir);
	XMStoreFloat3(&m_pLights[0].m_xmf3Direction, vLightDir);
	m_pLights[0].m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights[0].m_fRange = 0.0f;
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_pLights[0].m_fFalloff = 0.0f;
	m_pLights[0].m_fPhi = 0.0f;
	m_pLights[0].m_fTheta = 0.0f;

	//m_pLights[2].m_bEnable = true;
	//m_pLights[2].m_nType = SPOT_LIGHT;
	//m_pLights[2].m_fRange = 600.0f;
	//m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	//m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	//m_pLights[2].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	//m_pLights[2].m_xmf3Position = XMFLOAT3(-43.0f, 10.0f, -45.0f);
	//m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	//m_pLights[2].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	//m_pLights[2].m_fFalloff = 8.0f;
	//m_pLights[2].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	//m_pLights[2].m_fTheta = (float)cos(XMConvertToRadians(30.0f));

	//m_pLights[3].m_bEnable = true;
	//m_pLights[3].m_nType = SPOT_LIGHT;
	//m_pLights[3].m_fRange = 600.0f;
	//m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	//m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f);
	//m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	//m_pLights[3].m_xmf3Position = XMFLOAT3(45.0f, 10.0f, -28.0f);
	//m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	//m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	//m_pLights[3].m_fFalloff = 8.0f;
	//m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	//m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));

	//m_pLights[4].m_bEnable = true;
	//m_pLights[4].m_nType = SPOT_LIGHT;
	//m_pLights[4].m_fRange = 600.0f;
	//m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	//m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.2f, 0.2f, 1.0f);
	//m_pLights[4].m_xmf4Specular = XMFLOAT4(0.6f, 0.2f, 0.2f, 0.0f);
	//m_pLights[4].m_xmf3Position = XMFLOAT3(-63.0f, 10.0f, -14.0f);
	//m_pLights[4].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	//m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	//m_pLights[4].m_fFalloff = 8.0f;
	//m_pLights[4].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	//m_pLights[4].m_fTheta = (float)cos(XMConvertToRadians(30.0f));
}

void CScene::InitializeCollisionSystem()
{
	BoundingBox worldBounds(XMFLOAT3(-20.0f, -10.f, -66.0f), XMFLOAT3(150.0f, 100.0f, 170.0f));
	m_CollisionManager.Build(worldBounds, 35, 4);

	for (auto* obj : m_GameObjects) {
		m_CollisionManager.InsertObject(obj);
	}

	for (auto* obj : m_Monsters) {
		m_CollisionManager.InsertObject(obj);
	}

	for (auto obj : m_pMap->m_vMapObjects) {
		std::string strFrameName = obj->GetFrameName();
		if (std::string::npos != strFrameName.find("floor") || std::string::npos != strFrameName.find("ceiling"))
			continue;
		m_CollisionManager.InsertObject(obj);
	}

	//m_CollisionManager.PrintTree();
}

void CScene::GenerateGameObjectsBoundingBox()
{
	m_pPlayer->CalculateBoundingBox();

	for (auto* obj : m_GameObjects) {
		obj->CalculateBoundingBox();
	}

	for (auto* obj : m_Monsters) {
		obj->CalculateBoundingBox();
	}

	for (auto obj : m_pMap->m_vMapObjects) {
		obj->CalculateBoundingBox(); 
	}
}

void CScene::BuildSimpleUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	struct UIInfo { std::wstring path; float left; float top; float width; float height; };
	std::vector<UIInfo> uiList = {
		{ L"Image/enforce.dds",  0.20f, 0.25f, -0.45f, 0.4f },
		{ L"Image/enforce.dds",  0.45f, 0.25f, -0.45f, 0.4f },
		{ L"Image/enforce.dds", 0.70f, 0.25f, -0.45f, 0.4f },
	};

	for (size_t i = 0; i < uiList.size(); ++i)
	{
		// 텍스처 생성 및 로드
		CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
		pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, const_cast<wchar_t*>(uiList[i].path.c_str()), RESOURCE_TEXTURE2D, 0);

		CScene::CreateShaderResourceViews(pd3dDevice, pTexture, 0, 15);

		CTextureToScreenShader* pShader = new CTextureToScreenShader(1);
		pShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

		CScreenRectMeshTextured* pMesh = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, uiList[i].left, uiList[i].top, uiList[i].width, uiList[i].height);
		pShader->SetMesh(0, pMesh);
		pShader->SetTexture(pTexture);
		pShader->SetVisible(true);

		m_Shaders.push_back(pShader);

		m_UITextures.push_back(pTexture);

		//CText* pLVText = new CText(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"LV. ", uiList[i].left, -0.45f);
		//m_GameObjects.push_back(pLVText);
	}
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 100, 1000);

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature); 

	BuildDefaultLightsAndMaterials(true);

	Device = pd3dDevice;
	Commandlist = pd3dCommandList;

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	m_pMap = new Map(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

//	m_pEffect = new CParticle(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
//
//
//
	
	m_pKnightModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Knight.bin", NULL);
	m_pWizardModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Wizard.bin", NULL);

	//m_pThiefModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Thief.bin", NULL);
	
	m_Monsters.clear();
	m_Monsters.resize(4);
	int monsterIDs[4] = { 10001,10002,10003,10004 };

	CLoadedModelInfo* pSpiderModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SalamanderPA.bin", NULL);
	XMFLOAT3 monsterPos[4] = {
		{-14.4431, 0, 97.5215},
		{-9.05774, 0, 122.508},
		{13.721, 0, 117.925},
		{4.82955, 0, 96.9888}
	};
	for (int i = 0; i < static_cast<int>(m_Monsters.size()); ++i)
	{
		m_Monsters[i] = new CSpider(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSpiderModel, 5);
		m_Monsters[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0); //idle
		m_Monsters[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1); //walk
		m_Monsters[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2); //attack
		m_Monsters[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3); //gethit
		m_Monsters[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4); //death
		m_Monsters[i]->m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_Monsters[i]->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_Monsters[i]->m_pSkinnedAnimationController->SetTrackEnable(3, false);
		m_Monsters[i]->m_pSkinnedAnimationController->SetTrackEnable(4, false);

		m_Monsters[i]->SetPosition(monsterPos[i]);
		m_Monsters[i]->Rotate(0, rand(), 0);
		m_Monsters[i]->SetScale(2, 2, 2);

		//std::string spiderName = "SalamanderPA" + std::to_string(i);
		//m_Monsters[i]->SetFrameName(spiderName.c_str());

		static_cast<CSpider*>(m_Monsters[i])->SetMonsterID(monsterIDs[i]);
		g_monsters[monsterIDs[i]] = static_cast<CSpider*>(m_Monsters[i]);

	}

	if (pSpiderModel) delete pSpiderModel;

	BuildSimpleUI(pd3dDevice, pd3dCommandList);

//
//	m_GameObjects.clear();
//	m_GameObjects.resize(8);
 
//
#pragma region Items
//	long long itemIDs[8] = { 20000, 20001, 20002,
//							 30000, 30001, 30002, 30003, 30004};
//
//	float itemPrices[8] = { 80, 150, 80, 10, 20, 30, 40, 50 };
//
//	XMFLOAT3 positions[8] = {
//		{-2, 0, 19},
//		{-2, 0, 22},
//		{-2, 0, 25},
//		{-2, 0, 28},
//		{-2, 0, 31},
//		{-2, 0, 34},
//		{-2, 0, 37},
//		{-2, 0, 13}
//	};
//
//	CLoadedModelInfo* pFlashlightModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Item/Flashlightgold.bin", NULL);
//	m_GameObjects[0] = new FlashLight(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pFlashlightModel);
//	m_GameObjects[0]->SetScale(3, 3, 3);
//	m_GameObjects[0]->Rotate(-90, 180, 0);
//	m_GameObjects[0]->SetFrameName("FlashLight");
//	m_GameObjects[0]->SetPosition(positions[0]);
//
//	static_cast<Item*>(m_GameObjects[0])->SetUniqueID(itemIDs[0]);
//	static_cast<Item*>(m_GameObjects[0])->SetPrice(itemPrices[0]);
//	g_items[itemIDs[0]] = static_cast<Item*>(m_GameObjects[0]);
//
//	if (pFlashlightModel) delete pFlashlightModel;
//
//	// ============================================================================================================
//
//	CLoadedModelInfo* pShovelModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Item/Shovel.bin", NULL);
//	m_GameObjects[1] = new Shovel(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pShovelModel);
//	m_GameObjects[1]->SetScale(1, 1, 1);
//	m_GameObjects[1]->Rotate(0, -90, 160);
//	m_GameObjects[1]->SetFrameName("Shovel");
//	m_GameObjects[1]->SetPosition(positions[1]);
//
//	static_cast<Item*>(m_GameObjects[1])->SetUniqueID(itemIDs[1]);
//	static_cast<Item*>(m_GameObjects[1])->SetPrice(itemPrices[1]);
//	g_items[itemIDs[1]] = static_cast<Item*>(m_GameObjects[1]);
//
//	if (pShovelModel) delete pShovelModel;
//
//	// ============================================================================================================
//
//
//	CLoadedModelInfo* pWhistleModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Item/Whistle.bin", NULL);
//	m_GameObjects[2] = new Whistle(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pWhistleModel);
//	m_GameObjects[2]->SetScale(1, 1, 1);
//	m_GameObjects[2]->SetFrameName("Whistle");
//	m_GameObjects[2]->SetPosition(positions[2]);
//
//	static_cast<Item*>(m_GameObjects[2])->SetUniqueID(itemIDs[2]);
//	static_cast<Item*>(m_GameObjects[2])->SetPrice(itemPrices[2]);
//	g_items[itemIDs[2]] = static_cast<Item*>(m_GameObjects[2]);
//
//	if (pWhistleModel) delete pWhistleModel;
//
//	// ============================================================================================================
//	// ============================================================================================================
//
//
//	CLoadedModelInfo* pGoldbarModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Item/Goldbar.bin", NULL);
//	m_GameObjects[3] = new Whistle(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGoldbarModel);
//	m_GameObjects[3]->SetScale(1, 1, 1);
//	m_GameObjects[3]->SetFrameName("Goldbar");
//	m_GameObjects[3]->SetPosition(positions[3]);
//
//	static_cast<Item*>(m_GameObjects[3])->SetUniqueID(itemIDs[3]);
//	static_cast<Item*>(m_GameObjects[3])->SetPrice(itemPrices[3]);
//	g_items[itemIDs[3]] = static_cast<Item*>(m_GameObjects[3]);
//
//	if (pGoldbarModel) delete pGoldbarModel;
//
//	// ============================================================================================================
//
//
//	CLoadedModelInfo* pCoinModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Item/Coin.bin", NULL);
//	m_GameObjects[4] = new Whistle(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCoinModel);
//	m_GameObjects[4]->SetScale(2, 2, 2);
//	m_GameObjects[4]->SetFrameName("Coin");
//	m_GameObjects[4]->SetPosition(positions[4]);
//
//	static_cast<Item*>(m_GameObjects[4])->SetUniqueID(itemIDs[4]);
//	static_cast<Item*>(m_GameObjects[4])->SetPrice(itemPrices[4]);
//	g_items[itemIDs[4]] = static_cast<Item*>(m_GameObjects[4]);
//
//	if (pCoinModel) delete pCoinModel;
//
//	// ============================================================================================================
//
//
//	CLoadedModelInfo* pCanister1Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Item/Canisters_01.bin", NULL);
//	m_GameObjects[5] = new Whistle(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCanister1Model);
//	m_GameObjects[5]->SetScale(1, 1, 1);
//	m_GameObjects[5]->SetFrameName("Canisters_01");
//	m_GameObjects[5]->SetPosition(positions[5]);
//
//	static_cast<Item*>(m_GameObjects[5])->SetUniqueID(itemIDs[5]);
//	static_cast<Item*>(m_GameObjects[5])->SetPrice(itemPrices[5]);
//	g_items[itemIDs[5]] = static_cast<Item*>(m_GameObjects[5]);
//
//	if (pCanister1Model) delete pCanister1Model;
//
//	// ============================================================================================================
//
//
//	CLoadedModelInfo* pCanister2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Item/Canisters_02.bin", NULL);
//	m_GameObjects[6] = new Whistle(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCanister2Model);
//	m_GameObjects[6]->SetScale(1, 1, 1);
//	m_GameObjects[6]->SetFrameName("Canisters_02");
//	m_GameObjects[6]->SetPosition(positions[6]);
//
//	static_cast<Item*>(m_GameObjects[6])->SetUniqueID(itemIDs[6]);
//	static_cast<Item*>(m_GameObjects[6])->SetPrice(itemPrices[6]);
//	g_items[itemIDs[6]] = static_cast<Item*>(m_GameObjects[6]);
//
//	if (pCanister2Model) delete pCanister2Model;
//
//	// ============================================================================================================
//
//
//	CLoadedModelInfo* pCanister3Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Item/Canisters_03.bin", NULL);
//	m_GameObjects[7] = new Whistle(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCanister3Model);
//	m_GameObjects[7]->SetScale(1, 1, 1);
//	m_GameObjects[7]->SetFrameName("Canisters_03");
//	m_GameObjects[7]->SetPosition(positions[7]);
//
//	static_cast<Item*>(m_GameObjects[7])->SetUniqueID(itemIDs[7]);
//	static_cast<Item*>(m_GameObjects[7])->SetPrice(itemPrices[7]);
//	g_items[itemIDs[7]] = static_cast<Item*>(m_GameObjects[7]);
//
//	if (pCanister3Model) delete pCanister3Model;
//
//	m_nOtherPlayers = 1;
//	m_ppOtherPlayers = new OtherPlayer * [m_nOtherPlayers];
//	CLoadedModelInfo* pOtherPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Player.bin", NULL);
//	m_ppOtherPlayers[0] = new OtherPlayer(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pOtherPlayerModel);
//	m_ppOtherPlayers[0]->SetPosition(-1000, -1000, -1000);
//	m_ppOtherPlayers[0]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
//	m_ppOtherPlayers[0]->m_pSkinnedAnimationController->SetTrackEnable(1, false);
//	m_ppOtherPlayers[0]->m_pSkinnedAnimationController->SetTrackEnable(2, false);
//	m_ppOtherPlayers[0]->m_pSkinnedAnimationController->SetTrackEnable(3, false);
//	m_ppOtherPlayers[0]->m_pSkinnedAnimationController->SetTrackEnable(4, false);
//	m_ppOtherPlayers[0]->m_pSkinnedAnimationController->SetTrackEnable(5, false);
//	m_ppOtherPlayers[0]->m_pSkinnedAnimationController->SetTrackEnable(6, false);
//
//	m_ppOtherPlayers[0]->m_pSkinnedAnimationController->SetTrackPosition(1, 0.0f);
//	m_ppOtherPlayers[0]->m_pSkinnedAnimationController->SetTrackPosition(2, 0.0f);
//	m_ppOtherPlayers[0]->m_pSkinnedAnimationController->SetTrackPosition(3, 0.0f);
//	m_ppOtherPlayers[0]->m_pSkinnedAnimationController->SetTrackPosition(4, 0.0f);
//	m_ppOtherPlayers[0]->m_pSkinnedAnimationController->SetTrackPosition(5, 0.0f);
//	m_ppOtherPlayers[0]->m_pSkinnedAnimationController->SetTrackPosition(6, 0.0f);
//
//	if (pOtherPlayerModel) delete pOtherPlayerModel;
#pragma endregion
//
#pragma region InventoryUIandShop

//	// 인벤토리 UI 및 상점
//	m_Shaders.clear();
//	m_Shaders.resize(10);
//
//	CTexture* pTextureinven = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
//	pTextureinven->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/inven.dds", RESOURCE_TEXTURE2D, 0);
//	CreateShaderResourceViews(pd3dDevice, pTextureinven, 0, 15);
//	m_textureMap["inven"] = pTextureinven;
//
//	CTexture* pTextureItem1 = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
//	pTextureItem1->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Shovel.dds", RESOURCE_TEXTURE2D, 0);
//	CreateShaderResourceViews(pd3dDevice, pTextureItem1, 0, 15);
//	m_textureMap["Shovel"] = pTextureItem1;
//
//	CTexture* pTextureItem2 = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
//	pTextureItem2->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/FlashLight.dds", RESOURCE_TEXTURE2D, 0);
//	CreateShaderResourceViews(pd3dDevice, pTextureItem2, 0, 15);
//	m_textureMap["FlashLight"] = pTextureItem2;
//
//	CTexture* pTextureItem3 = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
//	pTextureItem3->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Whistle.dds", RESOURCE_TEXTURE2D, 0);
//	CreateShaderResourceViews(pd3dDevice, pTextureItem3, 0, 15);
//	m_textureMap["Whistle"] = pTextureItem3;
//
//	CTexture* pTextureItem4 = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
//	pTextureItem4->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Goldbar.dds", RESOURCE_TEXTURE2D, 0);
//	CreateShaderResourceViews(pd3dDevice, pTextureItem4, 0, 15);
//	m_textureMap["Goldbar"] = pTextureItem4;
//
//	CTexture* pTextureItem5 = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
//	pTextureItem5->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Coin.dds", RESOURCE_TEXTURE2D, 0);
//	CreateShaderResourceViews(pd3dDevice, pTextureItem5, 0, 15);
//	m_textureMap["Coin"] = pTextureItem5;
//
//	CTexture* pTextureItem6 = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
//	pTextureItem6->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Canisters_01.dds", RESOURCE_TEXTURE2D, 0);
//	CreateShaderResourceViews(pd3dDevice, pTextureItem6, 0, 15);
//	m_textureMap["Canisters_01"] = pTextureItem6;
//
//	CTexture* pTextureItem7 = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
//	pTextureItem7->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Canisters_02.dds", RESOURCE_TEXTURE2D, 0);
//	CreateShaderResourceViews(pd3dDevice, pTextureItem7, 0, 15);
//	m_textureMap["Canisters_02"] = pTextureItem7;
//
//	CTexture* pTextureItem8 = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
//	pTextureItem8->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Canisters_03.dds", RESOURCE_TEXTURE2D, 0);
//	CreateShaderResourceViews(pd3dDevice, pTextureItem8, 0, 15);
//	m_textureMap["Canisters_03"] = pTextureItem8;
//
//	CTextureToScreenShader* pTextureItem1Shader = new CTextureToScreenShader(1);
//	pTextureItem1Shader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
//	CScreenRectMeshTextured* pMesh = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, 0.02f, 0.225f * 0.5f, -0.65f, 0.4f * 0.5f);
//	pTextureItem1Shader->SetMesh(0, pMesh);
//	pTextureItem1Shader->SetTexture(pTextureinven);
//	m_Shaders[0] = pTextureItem1Shader;
//
//	CTextureToScreenShader* pTextureItem2Shader = new CTextureToScreenShader(1);
//	pTextureItem2Shader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
//	CScreenRectMeshTextured* pMesh1 = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, 0.02f + 0.125f, 0.225f * 0.5f, -0.65f, 0.4f * 0.5f);
//	pTextureItem2Shader->SetMesh(0, pMesh1);
//	pTextureItem2Shader->SetTexture(pTextureinven);
//	m_Shaders[1] = pTextureItem2Shader;
//
//	CTextureToScreenShader* pTextureItem3Shader = new CTextureToScreenShader(1);
//	pTextureItem3Shader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
//	CScreenRectMeshTextured* pMesh2 = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, 0.02f + 0.25f, 0.225f * 0.5f, -0.65f, 0.4f * 0.5f);
//	pTextureItem3Shader->SetMesh(0, pMesh2);
//	pTextureItem3Shader->SetTexture(pTextureinven);
//	m_Shaders[2] = pTextureItem3Shader;
//
//	CTextureToScreenShader* pTextureItem4Shader = new CTextureToScreenShader(1);
//	pTextureItem4Shader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
//	CScreenRectMeshTextured* pMesh3 = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, 0.02f + 0.375f, 0.225f * 0.5f, -0.65f, 0.4f * 0.5f);
//	pTextureItem4Shader->SetMesh(0, pMesh3);
//	pTextureItem4Shader->SetTexture(pTextureinven);
//	m_Shaders[3] = pTextureItem4Shader;
//
//	CTextureToScreenShader* pInventoryShader = new CTextureToScreenShader(1);
//	pInventoryShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
//	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
//	pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Inventory.dds", RESOURCE_TEXTURE2D, 0);
//	CreateShaderResourceViews(pd3dDevice, pTexture, 0, 15);
//	CScreenRectMeshTextured* pInventoryMesh = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, -0.5f + 0.5f, 1.0f * 0.5f, -0.6f, 0.5f * 0.5f);
//	pInventoryShader->SetMesh(0, pInventoryMesh);
//	pInventoryShader->SetTexture(pTexture);
//	m_Shaders[4] = pInventoryShader;
//
//	//상점
//	CShopShader* pShopShader = new CShopShader(1);
//	pShopShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
//	pShopShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, NULL);
//
//	CTexture* pShopTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
//	pShopTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/shop.dds", RESOURCE_TEXTURE2D, 0);
//	CreateShaderResourceViews(pd3dDevice, pShopTexture, 0, 15);
//	CScreenRectMeshTextured* pShopMesh = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, -0.9f, 0.5f, 0.8f, 0.7f);
//	pShopShader->SetMesh(0, pShopMesh);
//	pShopShader->SetTexture(pShopTexture);
//	pShopShader->SetVisible(false);
//
//	m_Shaders[5] = pShopShader;
//
//	//상점 4칸
//	CTextureToScreenShader* pShopSpace1Shader = new CTextureToScreenShader(1);
//	pShopSpace1Shader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
//	CScreenRectMeshTextured* pShopMesh1 = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, -0.825f, 0.08f, 0.625f, 0.09f);
//	pShopSpace1Shader->SetMesh(0, pShopMesh1);
//	pShopSpace1Shader->SetTexture(pTextureinven);
//	pShopSpace1Shader->SetVisible(false);
//	m_Shaders[6] = pShopSpace1Shader;
//
//	CTextureToScreenShader* pShopSpace2Shader = new CTextureToScreenShader(1);
//	pShopSpace2Shader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
//	CScreenRectMeshTextured* pShopMesh2 = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, -0.825f, 0.08f, 0.505f, 0.09f);
//	pShopSpace2Shader->SetMesh(0, pShopMesh2);
//	pShopSpace2Shader->SetTexture(pTextureinven);
//	pShopSpace2Shader->SetVisible(false);
//	m_Shaders[7] = pShopSpace2Shader;
//
//	CTextureToScreenShader* pShopSpace3Shader = new CTextureToScreenShader(1);
//	pShopSpace3Shader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
//	CScreenRectMeshTextured* pShopMesh3 = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, -0.825f, 0.08f, 0.385f, 0.09f);
//	pShopSpace3Shader->SetMesh(0, pShopMesh3);
//	pShopSpace3Shader->SetTexture(pTextureinven);
//	pShopSpace3Shader->SetVisible(false);
//	m_Shaders[8] = pShopSpace3Shader;
//
//	CTextureToScreenShader* pShopSpace4Shader = new CTextureToScreenShader(1);
//	pShopSpace4Shader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
//	CScreenRectMeshTextured* pShopMesh4 = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, -0.825f, 0.08f, 0.265f, 0.09f);
//	pShopSpace4Shader->SetMesh(0, pShopMesh4);
//	pShopSpace4Shader->SetTexture(pTextureinven);
//	pShopSpace4Shader->SetVisible(false);
//	m_Shaders[9] = pShopSpace4Shader;
#pragma endregion

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();

	for (auto* obj : m_GameObjects)
	{
		if (obj) obj->Release();
	}
	m_GameObjects.clear();

	for (auto* shader : m_Shaders)
	{
		if (!shader) continue;
		shader->ReleaseShaderVariables();
		shader->ReleaseObjects();
		shader->Release();
	}
	m_Shaders.clear();

	if (m_pTerrain) delete m_pTerrain;
	if (m_pSkyBox) delete m_pSkyBox;

	for (auto* monster : m_Monsters)
	{
		if (monster) monster->Release();
	}
	m_Monsters.clear();

	for (auto* pTex : m_UITextures)
	{
		if (pTex) delete pTex;
	}
	m_UITextures.clear();

	ReleaseShaderVariables();

	if (m_pLights) delete[] m_pLights;
}

ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[12];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 6; //t6: gtxtAlbedoTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7; //t7: gtxtSpecularTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 8; //t8: gtxtNormalTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 9; //t9: gtxtMetallicTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 10; //t10: gtxtEmissionTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 11; //t11: gtxtEmissionTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 12; //t12: gtxtEmissionTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 1; //t1: gtxtTerrainBaseTexture
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[9].NumDescriptors = 1;
	pd3dDescriptorRanges[9].BaseShaderRegister = 2; //t2: gtxtTerrainDetailTexture
	pd3dDescriptorRanges[9].RegisterSpace = 0;
	pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[10].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[10].NumDescriptors = 1;
	pd3dDescriptorRanges[10].BaseShaderRegister = 0; //t0: gtxTexture
	pd3dDescriptorRanges[10].RegisterSpace = 0;
	pd3dDescriptorRanges[10].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[11].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[11].NumDescriptors = 1;
	pd3dDescriptorRanges[11].BaseShaderRegister = 3; //t3: gFontTexture
	pd3dDescriptorRanges[11].RegisterSpace = 0;
	pd3dDescriptorRanges[11].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[17];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 33;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[11].Descriptor.ShaderRegister = 7; //Skinned Bone Offsets
	pd3dRootParameters[11].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[12].Descriptor.ShaderRegister = 8; //Skinned Bone Transforms
	pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[14].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[14].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[15].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[15].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[10]);
	pd3dRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[16].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[16].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[16].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[11]);
	pd3dRootParameters[16].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[3];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[2].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[2].MipLODBias = 0;
	pd3dSamplerDescs[2].MaxAnisotropy = 1;
	pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[2].MinLOD = 0;
	pd3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[2].ShaderRegister = 2;
	pd3dSamplerDescs[2].RegisterSpace = 0;
	pd3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

	for (auto* shader : m_Shaders) if (shader) shader->ReleaseUploadBuffers();
	for (auto* obj : m_GameObjects) if (obj) obj->ReleaseUploadBuffers();
	for (auto* monster : m_Monsters) if (monster) monster->ReleaseUploadBuffers();
}

void CScene::CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferViews(ID3D12Device *pd3dDevice, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}

void CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetResource(i);
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			pTexture->SetGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int j = 0; j < nRootParameters; j++) pTexture->SetRootParameterIndex(j, nRootParameterStartIndex + j);
}

void CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	RECT rt[3] =
	{
		{ 0, 0, FRAME_BUFFER_WIDTH / 3, FRAME_BUFFER_HEIGHT },
		{ FRAME_BUFFER_WIDTH / 3, 0, FRAME_BUFFER_WIDTH / 3 * 2, FRAME_BUFFER_HEIGHT },
		{ FRAME_BUFFER_WIDTH / 3 * 2, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT }
	};

	POINT pt;
	GetCursorPos(&pt);        // screen 좌표
	ScreenToClient(hWnd, &pt); // client 좌표로 변환

	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	{
		cout << "Mouse Clicked at (" << pt.x << ", " << pt.y << ")" << endl;

		dynamic_cast<CTerrainPlayer*>(m_pPlayer)->m_currentAnim = AnimationState::SWING;
	}
	break;
	}
}

void CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		default:
			break;
		}
		break;
	default:
		break;
	}
}

bool CScene::ProcessInput(UCHAR *pKeysBuffer)
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (auto* obj : m_GameObjects) {
		if (obj) obj->Animate(fTimeElapsed);
		if (auto* textObj = dynamic_cast<CText*>(obj)) {
			textObj->UpdateText(std::to_wstring(m_pPlayer->level[0]), L"LV. ");
			textObj->UpdateText(std::to_wstring(m_pPlayer->level[1]), L"LV. ");
			textObj->UpdateText(std::to_wstring(m_pPlayer->level[2]), L"LV. ");
		}
	}
	
	for(auto* shader : m_Shaders) if(shader) shader->AnimateObjects(fTimeElapsed);
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights

	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);
	if (m_pMap) m_pMap->Render(pd3dCommandList, pCamera);

	for (auto* obj : m_GameObjects) 
	{
		if (!obj) continue;
		if (obj->GetVisible())
			obj->Render(pd3dCommandList, pCamera);
	}
  
	for (auto* monster : m_Monsters) 
	{
		if (!monster) continue;
		monster->Animate(m_fElapsedTime);
		monster->Render(pd3dCommandList, pCamera);
	}

	m_CollisionManager.Update(m_pPlayer);
	
	if (m_pEffect) m_pEffect->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nOtherPlayers; ++i) 
	{
		//if (m_ppOtherPlayers[i]->isConnedted)
		//m_ppOtherPlayers[i]->Animate(m_ppOtherPlayers[i]->animation, m_fElapsedTime);
		if (m_ppOtherPlayers[i]->visible)m_ppOtherPlayers[i]->Render(pd3dCommandList, pCamera);
	}

	for (auto* shader : m_Shaders)
	{
		if (!shader) continue;
		auto* texShader = dynamic_cast<CTextureToScreenShader*>(shader);
		if (texShader && texShader->visible)
			shader->Render(pd3dCommandList, pCamera);
	}
}


// ==========================================================================================================
// StartScene
// ==========================================================================================================
void CStartScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 100); 

	m_Shaders.clear();
	m_Shaders.resize(1);

	CTextureToScreenShader* pTextureToScreenShader = new CTextureToScreenShader(1);
	pTextureToScreenShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/StartScene.dds", RESOURCE_TEXTURE2D, 0);
	//pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Font.dds", RESOURCE_TEXTURE2D, 1);

	CreateShaderResourceViews(pd3dDevice, pTexture, 0, 15);

	CScreenRectMeshTextured* pMesh = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, -1.0f, 2.0f, 1.0f, 2.0f);
	pTextureToScreenShader->SetMesh(0, pMesh);
	//pMesh = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, -1.f, 0.12f * 36, -0.5f, 0.20f, 1);
	//pTextureToScreenShader->SetMesh(1, pMesh);
	pTextureToScreenShader->SetTexture(pTexture);

	m_Shaders[0] = pTextureToScreenShader;

	m_GameObjects.clear();
	m_GameObjects.resize(2);

	m_pFontID = new CText(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Enter ID : ", 0.75f, -0.55f);
	m_GameObjects[0] = m_pFontID;

	m_pFontIP = new CText(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Enter IP : ", 0.75f, -0.75f);
	m_GameObjects[1] = m_pFontIP;


	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CStartScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	for (auto* shader : m_Shaders)
	{
		if (!shader) continue;
		shader->ReleaseShaderVariables();
		shader->ReleaseObjects();
		shader->Release();
	}
	m_Shaders.clear();

	for (auto* obj : m_GameObjects)
	{
		if (obj) obj->Release();
	}
	m_GameObjects.clear();
}

void CStartScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	for (auto* shader : m_Shaders) if (shader) shader->Render(pd3dCommandList, pCamera);
	for (auto* obj : m_GameObjects) if (obj) obj->Render(pd3dCommandList, pCamera);
}

void CStartScene::AnimateObjects(float fTimeElapsed)
{
	if (m_textDirty && m_inputStep == InputStep::EnterID)
	{
		wstring id;
		id.assign(m_inputID.begin(), m_inputID.end());

		m_pFontID->UpdateText(id, L"Enter ID : ");
		m_textDirty = false;
	}
	if (m_textDirty && m_inputStep == InputStep::EnterIP)
	{
		wstring ip;
		ip.assign(m_inputIP.begin(), m_inputIP.end());

		m_pFontIP->UpdateText(ip, L"Enter IP : ");
		m_textDirty = false;
	}
}

void CStartScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_networkInitialized) return;

	switch (nMessageID)
	{
	case WM_KEYDOWN:
		if (m_inputStep == InputStep::EnterID) {
			if (wParam == VK_RETURN) {
				m_inputStep = InputStep::EnterIP;
				::user_name = m_inputID;
			}
			else if (wParam == VK_BACK && !m_inputID.empty()) {
				m_inputID.pop_back();
				m_textDirty = true;
			}
			else if (isprint(wParam) && m_inputID.length() < MAX_ID_LENGTH - 1) {
				m_inputID.push_back((char)wParam);
				m_textDirty = true;
			}
			
			break;
		}
		if (m_inputStep == InputStep::EnterIP) {
			if (isdigit(wParam) && m_inputIP.length() < 15) {
				m_inputIP.push_back((char)wParam);
				m_textDirty = true;
			}
			else if (wParam == 190) {
				m_inputIP.push_back('.');
				m_textDirty = true;
			}

			else if (wParam == VK_RETURN) {
				m_inputStep = InputStep::Done;
				m_networkInitialized = true;

				char serverIP[16];
				strcpy(serverIP, m_inputIP.c_str());
				std::cout << "Connecting to: " << serverIP << std::endl;
				InitializeNetwork(serverIP); // server IP 전달
				gGameFramework.MoveToNextScene(1);
			}
			else if (wParam == VK_BACK && !m_inputIP.empty()) {
				m_inputIP.pop_back();
			}
		}
		break;
	}
}

// ==========================================================================================================
// SelectScene
// ==========================================================================================================
void CSelectScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 5, 100);

	m_Shaders.clear();
	m_Shaders.resize(2);

	CTextureToScreenShader* pTextureToScreenShader = new CTextureToScreenShader(1);
	pTextureToScreenShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/SelectScene.dds", RESOURCE_TEXTURE2D, 0);

	CreateShaderResourceViews(pd3dDevice, pTexture, 0, 15);

	CScreenRectMeshTextured* pMesh = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, -1.0f, 2.0f, 1.0f, 2.0f);
	pTextureToScreenShader->SetMesh(0, pMesh);
	pTextureToScreenShader->SetTexture(pTexture);

	m_Shaders[0] = pTextureToScreenShader;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	CTextureToScreenShader* pTextureToScreenShader1 = new CTextureToScreenShader(1);
	pTextureToScreenShader1->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	CTexture* pTexture1 = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture1->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/loading.dds", RESOURCE_TEXTURE2D, 0);

	CreateShaderResourceViews(pd3dDevice, pTexture1, 0, 15);

	pTextureToScreenShader1->SetMesh(0, pMesh);
	pTextureToScreenShader1->SetTexture(pTexture1);

	m_Shaders[1] = pTextureToScreenShader1;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CSelectScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	for (auto* shader : m_Shaders)
	{
		if (!shader) continue;
		shader->ReleaseShaderVariables();
		shader->ReleaseObjects();
		shader->Release();
	}
	m_Shaders.clear();
}

void CSelectScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	if (pCamera) {
		pCamera->SetViewportsAndScissorRects(pd3dCommandList);
		pCamera->UpdateShaderVariables(pd3dCommandList);
	}

	UpdateShaderVariables(pd3dCommandList);

	const int idx = (loading ? 1 : 0);

	if (idx >= 0 && idx < (int)m_Shaders.size() && m_Shaders[idx])
	{
		m_Shaders[idx]->Render(pd3dCommandList, pCamera);
		if (loading) m_bLoadingRenderedOnce = true;
	}
}

void CSelectScene::AnimateObjects(float fTimeElapsed)
{
	if (m_SceneId != -1 && m_bLoadingRenderedOnce)
	{
		int next = m_SceneId;
		m_SceneId = -1;                 // 중복 전환 방지(권장)
		gGameFramework.RequestMoveToScene(next);
	}
}

void CSelectScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	RECT rt[3] =
	{
		{ 0, 0, FRAME_BUFFER_WIDTH / 3, FRAME_BUFFER_HEIGHT },
		{ FRAME_BUFFER_WIDTH / 3, 0, FRAME_BUFFER_WIDTH / 3 * 2, FRAME_BUFFER_HEIGHT },
		{ FRAME_BUFFER_WIDTH / 3 * 2, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT }
	};

	POINT pt;
	GetCursorPos(&pt);        // screen 좌표
	ScreenToClient(hWnd, &pt); // client 좌표로 변환

	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		for(int i = 0; i < 3; ++i)
			if (PtInRect(&rt[i], pt)) {
				if (i == 0) gGameFramework.SetSelectedPlayerModel(EPlayerModelType::Knight);
				if (i == 1) gGameFramework.SetSelectedPlayerModel(EPlayerModelType::Wizard);
				if (i == 2) gGameFramework.SetSelectedPlayerModel(EPlayerModelType::Thief);
			}
		break;
	case WM_LBUTTONUP:
		loading = true;
		m_SceneId = 2;
		m_bLoadingRenderedOnce = false;
		break;
	default:
		break;
	}
}

// ==========================================================================================================
// EndScene
// ==========================================================================================================
void CEndScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 100);

	m_Shaders.clear();
	m_Shaders.resize(1);

	CTextureToScreenShader* pTextureToScreenShader = new CTextureToScreenShader(1);
	pTextureToScreenShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/EndScene.dds", RESOURCE_TEXTURE2D, 0);
	//pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Font.dds", RESOURCE_TEXTURE2D, 1);

	CreateShaderResourceViews(pd3dDevice, pTexture, 0, 15);

	CScreenRectMeshTextured* pMesh = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, -1.0f, 2.0f, 1.0f, 2.0f);
	pTextureToScreenShader->SetMesh(0, pMesh);
	//pMesh = new CScreenRectMeshTextured(pd3dDevice, pd3dCommandList, -1.f, 0.12f * 36, -0.5f, 0.20f, 1);
	//pTextureToScreenShader->SetMesh(1, pMesh);
	pTextureToScreenShader->SetTexture(pTexture);

	m_Shaders[0] = pTextureToScreenShader;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CEndScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	for (auto* shader : m_Shaders)
	{
		if (!shader) continue;
		shader->ReleaseShaderVariables();
		shader->ReleaseObjects();
		shader->Release();
	}
	m_Shaders.clear();
}

void CEndScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	if (pCamera) {
		pCamera->SetViewportsAndScissorRects(pd3dCommandList);
		pCamera->UpdateShaderVariables(pd3dCommandList);
	}
	UpdateShaderVariables(pd3dCommandList);

	for(auto* shader : m_Shaders) if(shader) shader->Render(pd3dCommandList, pCamera);
}
