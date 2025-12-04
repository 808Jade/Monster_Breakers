#pragma once
#include "stdafx.h"
#include "Object.h"

class Monster :
    public CGameObject
{
public:
	Monster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~Monster();

	virtual void Animate(float fTimeElapsed);
	virtual void SetPlayer(CPlayer* p) { pPlayer = p; }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	float MonsterHP = 100.0f;
	float hpRatio = MonsterHP / 100.0f;
	//Hpbar* m_pHpbar = NULL;

	virtual void SetMonsterID(int id) { monsterID = id; }
	int GetMonsterID() const { return monsterID; }

private:
	CPlayer* pPlayer = NULL;
	int monsterID = -1;
};

