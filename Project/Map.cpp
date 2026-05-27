#include "stdafx.h"
#include "Map.h"
#include "Shader.h"

Map::Map(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_pInstancedShader = new CInstancedStandardShader();
	m_pInstancedShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);

	LoadMapObjectsFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	LoadGeometryFromFile();
	SetInstanceData();
	BuildInstanceBuffers(pd3dDevice, pd3dCommandList);
	BuildWorldBoundingBoxes();

	//cout << "[ m_vLoadedModelInfo ]" << endl;
	//for (auto& a : m_vLoadedModelInfo) {
	//    cout << a->GetFrameName() << " (" << a->GetPosition().x << ", " << a->GetPosition().z << ")" << endl;
	//}
	//cout << "[ m_vObjectInstances ]" << endl;
	//for (auto& a : m_vObjectInstances) {
	//    cout << "[" << a.modelIndex << "] " << a.objectName << "(" << a.position.x << ", " << a.position.z << ")" << endl;
	//}
	//cout << "[ m_vpMapObjects ]" << endl;
	//for (const auto& a : m_vMapObjects) {
	//	cout << a->GetFrameName() << " | " << a->GetPosition().x << ", " << a->GetPosition().z << " | " << endl;
	//}
}

Map::~Map()
{
}

void Map::ReleaseUploadBuffers()
{
	for (auto& pUploadBuffer : m_vUploadBuffers) {
		if (pUploadBuffer) pUploadBuffer->Release();
	}
	m_vUploadBuffers.clear();
}

string Map::ReadString(std::ifstream& inFile)
{
	uint8_t length;
	inFile.read(reinterpret_cast<char*>(&length), sizeof(uint8_t));
	std::string str(length, '\0');
	inFile.read(&str[0], length);
	return str;
}

// Model/Map 안의 모든 .bin 파일을 불러와 m_vLoadedModelInfo에 저장
void Map::LoadMapObjectsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	std::filesystem::path path{ "Model/Map" };

	if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
		std::cerr << "Error: Directory not found -> " << path << endl;
		return;
	}

	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		if (entry.path().extension() == ".bin") {
			CLoadedModelInfo* pModelInfo = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, entry.path(), m_pInstancedShader);

			if (pModelInfo) {
				m_vLoadedModelInfo.push_back(pModelInfo->m_pModelRootObject);
			}
		}
	}
}

// setter.bin 을 읽어와 m_vObjectInstances에 인스턴싱 데이터 저장
void Map::LoadGeometryFromFile()
{
	std::ifstream inFile("Model/Map/Setter/Map_objects_instances_setter.bin", std::ios::binary);
	if (!inFile)
	{
		std::cerr << "Error: Could not open Map_objects_instances_setter.bin" << std::endl;
		return;
	}

	m_vObjectInstances.clear();

	// RootObject 정보 읽기
	std::string frameTag = ReadString(inFile);
	if (frameTag != "<Frame>:")
	{
		std::cerr << "Error: Invalid format (expected <Frame>:)" << std::endl;
		return;
	}
	std::string rootObjectName = ReadString(inFile);
	std::string transformTag = ReadString(inFile);

	XMFLOAT3 rootPosition, rootRotation, rootScale;
	XMFLOAT4 rootQuaternion;
	float rootMatrix[16];

	inFile.read(reinterpret_cast<char*>(&rootPosition), sizeof(DirectX::XMFLOAT3));
	inFile.read(reinterpret_cast<char*>(&rootRotation), sizeof(DirectX::XMFLOAT3));
	inFile.read(reinterpret_cast<char*>(&rootScale), sizeof(DirectX::XMFLOAT3));
	inFile.read(reinterpret_cast<char*>(&rootQuaternion), sizeof(DirectX::XMFLOAT4));

	std::string matrixTag = ReadString(inFile);
	inFile.read(reinterpret_cast<char*>(rootMatrix), sizeof(float) * 16);
	std::string childrenTag = ReadString(inFile);

	int childCount;
	inFile.read(reinterpret_cast<char*>(&childCount), sizeof(int));

	// 자식 오브젝트 읽기 (Level 1)
	for (int i = 0; i < childCount; i++)
	{
		std::string childFrameTag = ReadString(inFile);
		if (childFrameTag != "<Frame>:")
		{
			std::cerr << "Error: Invalid format (expected <Frame>:)" << std::endl;
			return;
		}

		std::string objectName = ReadString(inFile);
		std::string childTransformTag = ReadString(inFile);

		DirectX::XMFLOAT3 position, rotation, scale;
		DirectX::XMFLOAT4 quaternion;
		inFile.read(reinterpret_cast<char*>(&position), sizeof(DirectX::XMFLOAT3));
		inFile.read(reinterpret_cast<char*>(&rotation), sizeof(DirectX::XMFLOAT3));
		inFile.read(reinterpret_cast<char*>(&scale), sizeof(DirectX::XMFLOAT3));
		inFile.read(reinterpret_cast<char*>(&quaternion), sizeof(DirectX::XMFLOAT4));

		std::string childMatrixTag = ReadString(inFile);

		float matrix[16];
		inFile.read(reinterpret_cast<char*>(matrix), sizeof(float) * 16);

		std::string endFrameTag = ReadString(inFile);
		if (endFrameTag != "</Frame>")
		{
			std::cerr << "Error: Invalid format (expected </Frame>)" << std::endl;
			return;
		}

		int index = -1;

		// 오브젝트 이름에서 순수한 모델 이름 추출
		std::string targetModelName = objectName;

		size_t suffixPos = objectName.rfind(" (");

		if (suffixPos != std::string::npos)
		{
			targetModelName = objectName.substr(0, suffixPos);
		}

		// 이름 비교
		for (int j = 0; j < m_vLoadedModelInfo.size(); j++)
		{
			string modelName = m_vLoadedModelInfo[j]->GetFrameName();

			if (targetModelName == modelName)
			{
				index = static_cast<int>(j);
				break;
			}
		}

		if (index == -1)
		{
			std::cout << "Warning: Could not find matching model for object: " << objectName
				<< " (Parsed: " << targetModelName << ")" << std::endl;
		}

		//if (objectName.find("trailway") != std::string::npos || objectName.find("Tileway") != std::string::npos)
		//{
		//		cout << std::left
		//	<< std::setw(20) << objectName << " [ "
		//	<< std::setw(4) << index << "] ( "
		//	<< std::setw(10) << position.x << ", "
		//	<< std::setw(10) << position.y << ", "
		//	<< std::setw(10) << position.z << " )  |"
		//	<< std::setw(10) << scale.x << " , " 
		//	<< std::setw(10) << scale.y << " , " 
		//	<< std::setw(10) << scale.z << " | " << endl;
		//}

		m_vObjectInstances.emplace_back(index, objectName, position, rotation, scale, quaternion, matrix);
	}

	for (int i = 0; i < m_vLoadedModelInfo.size(); i++)
	{
		CGameObject* pModel = m_vLoadedModelInfo[i];

		pModel->UpdateTransform(NULL);
	}

	inFile.close();
}

// m_vLoadedModelInfo과 m_vObjectInstances를 참조하여 m_mInstanceGroups에 <index : 행렬 데이터> 쌍으로 리스트에 담는다
void Map::SetInstanceData()
{
	m_mInstanceGroups.clear();

	// 1. 모든 인스턴스 데이터를 순회하며 그룹별로 묶기
	for (const auto& instance : m_vObjectInstances)
	{
		int modelIdx = instance.modelIndex;

		if (instance.modelIndex < 0 || instance.modelIndex >= m_vLoadedModelInfo.size())
		{
			std::cerr << "Warning: Invalid modelIndex (" << instance.modelIndex << ") for object " << instance.objectName << std::endl;
			continue;
		}

		// 해당 모델의 그룹이 아직 없다면 초기화
		if (m_mInstanceGroups.find(modelIdx) == m_mInstanceGroups.end())
		{
			m_mInstanceGroups[modelIdx].pModel = m_vLoadedModelInfo[modelIdx];
			m_mInstanceGroups[modelIdx].nInstances = 0;
		}

		// 2. 알짜배기(행렬) 데이터만 추출해서 리스트에 담기
		VS_INSTANCE_DATA gpuData;

		// 파일에 저장된 matrix[16]을 XMFLOAT4X4로 복사
		XMFLOAT4X4 tempMat;
		//memcpy(&gpuData.worldMatrix, instance.transformMatrix, sizeof(float) * 16);
		memcpy(&tempMat, instance.transformMatrix, sizeof(float) * 16);
		XMMATRIX xmMat = XMLoadFloat4x4(&tempMat);
		xmMat = XMMatrixTranspose(xmMat);
		XMStoreFloat4x4(&gpuData.worldMatrix, xmMat);

		m_mInstanceGroups[modelIdx].vInstanceData.push_back(gpuData);
		m_mInstanceGroups[modelIdx].nInstances++;
	}
}

// CPU 인스턴스 데이터(m_mInstanceGroups)를 GPU버퍼로 만든다
void Map::BuildInstanceBuffers(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (auto& pair : m_mInstanceGroups)
	{
		InstanceGroup& group = pair.second;

		if (group.nInstances == 0 || group.vInstanceData.empty())
			continue;

		// 전체 버퍼 크기 계산
		UINT nBufferSize = group.nInstances * sizeof(VS_INSTANCE_DATA);

		// GPU 메모리에 pInstanceBuffer 생성 및 데이터 복사
		ID3D12Resource* pUploadBuffer = NULL;

		group.pInstanceBuffer = ::CreateBufferResource(
			pd3dDevice,
			pd3dCommandList,
			group.vInstanceData.data(),							// CPU 원본 데이터의 시작 주소 (명부)
			nBufferSize,										// 복사할 총 바이트 수
			D3D12_HEAP_TYPE_DEFAULT,							// 목적지: GPU 전용 초고속 메모리
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,	// 용도: 정점/상수 버퍼용
			&pUploadBuffer										// 배달부: 임시 업로드 버퍼
		);

		// 생성된 업로드 버퍼를 벡터에 안전하게 보관
		if (pUploadBuffer) {
			m_vUploadBuffers.push_back(pUploadBuffer);
		}

		// GPU에게 이 버퍼를 설명해 줄 View 작성
		group.instanceBufferView.BufferLocation = group.pInstanceBuffer->GetGPUVirtualAddress(); // 주소
		group.instanceBufferView.StrideInBytes = sizeof(VS_INSTANCE_DATA);                       // 1개당 크기 (64바이트)
		group.instanceBufferView.SizeInBytes = nBufferSize;                                      // 전체 크기
	}
}

// m_mInstanceGroups 를 이용해 인스턴스 렌더링
void Map::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	m_pInstancedShader->OnPrepareRender(pd3dCommandList);

	for (auto& pair : m_mInstanceGroups)
	{
		InstanceGroup& group = pair.second;

		if (group.nInstances > 0 && group.pModel)
		{
			group.pModel->RenderInstanced(pd3dCommandList, pCamera, group.nInstances, group.pInstanceBuffer, &group.instanceBufferView);
		}
	}
}

void Map::BuildWorldBoundingBoxes()
{
	for (auto& [idx, group] : m_mInstanceGroups)
	{
		if (!group.pModel) continue;
		group.vWorldBoundingBoxes.clear();
		group.vWorldBoundingBoxes.reserve(group.nInstances);

		// 모델 계층 전체를 순회해 로컬 공간 AABB를 머지
		BoundingBox localBox{};
		bool hasBox = false;

		std::vector<CGameObject*> queue = { group.pModel };
		while (!queue.empty())
		{
			CGameObject* pObj = queue.back();
			queue.pop_back();

			if (pObj->m_pMesh)
			{
				BoundingBox lb = pObj->m_pMesh->GetBoundingBox();
				BoundingBox wb;
				lb.Transform(wb, XMLoadFloat4x4(&pObj->m_xmf4x4World));
				if (!hasBox) { localBox = wb; hasBox = true; }
				else          BoundingBox::CreateMerged(localBox, localBox, wb);
			}
			if (pObj->m_pChild)
			{
				CGameObject* child = pObj->m_pChild;
				queue.push_back(child);
				while (child->m_pSibling) { child = child->m_pSibling; queue.push_back(child); }
			}
		}

		if (!hasBox) continue;

		// 각 인스턴스의 월드 행렬로 변환 후 캐시에 저장
		for (const auto& inst : group.vInstanceData)
		{
			// gpuData.worldMatrix는 GPU용으로 이미 transpose됐으므로 원래 행렬로 복원
			XMMATRIX worldMat = XMMatrixTranspose(XMLoadFloat4x4(&inst.worldMatrix));
			BoundingBox worldBox;
			localBox.Transform(worldBox, worldMat);
			group.vWorldBoundingBoxes.push_back(worldBox);
		}
	}
}

//// 플레이어 XZ 위치에서 가장 높은 오브젝트 상단 Y를 반환
//float Map::GetHeight(float x, float z) const
//{
//	float maxHeight = 0.0f;
//	bool  found = false;
//
//	for (const auto& [idx, group] : m_mInstanceGroups)
//	{
//		for (const BoundingBox& wb : group.vWorldBoundingBoxes)
//		{
//			float minX = wb.Center.x - wb.Extents.x;
//			float maxX = wb.Center.x + wb.Extents.x;
//			float minZ = wb.Center.z - wb.Extents.z;
//			float maxZ = wb.Center.z + wb.Extents.z;
//
//			if (x >= minX && x <= maxX && z >= minZ && z <= maxZ)
//			{
//				float topY = wb.Center.y + wb.Extents.y;
//				if (!found || topY > maxHeight) { maxHeight = topY; found = true; }
//			}
//		}
//	}
//	return maxHeight;
//}
