#pragma once
#include "Object.h"

class Hpbar : public CGameObject {
public:
	Hpbar(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~Hpbar();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	void SetHpRatio(float ratio) { m_fHpRatio = ratio; }
private:
	float m_fHpRatio = 1.0f;
};