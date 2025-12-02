#include "Monster.h"

Monster::Monster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks) : CGameObject(1)
{
	CLoadedModelInfo* pSpiderModel = pModel;
	if (!pSpiderModel) pSpiderModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Spider.bin", NULL);

	SetChild(pSpiderModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pSpiderModel);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0); // idle
	m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1); // walk
	m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2); // attack
	m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3); // attack
	m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4); // death
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);

	//Hpbar *pHpbar = new Hpbar(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	//m_pHpbar = pHpbar;
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

}

Monster::~Monster()
{
}

void Monster::Animate(float fTimeElapsed)
{
	// rotate

	CGameObject::Animate(fTimeElapsed);
}

void Monster::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}