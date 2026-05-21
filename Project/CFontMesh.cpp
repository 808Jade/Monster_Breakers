#include "CFontMesh.h"

CFontMesh::CFontMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const std::wstring& text, float x, float y) : CMesh(pd3dDevice, pd3dCommandList)
{
    m_nStride = sizeof(CTexturedVertex);
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    m_text = text;

    const int count = static_cast<int>(m_text.length());
    if (count == 0) return;

    // 아래 때문에 오류 발생 의심
/*    if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();
    if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();*/

    const float charSize = 0.1f;
    const float spacing = 0.025f;
    m_nVertices = 6 * count;

    CTexturedVertex* vertices = new CTexturedVertex[m_nVertices];

    for (int i = 0; i < count; ++i)
    {
        wchar_t ch = m_text[i];
        int ascii = (int)ch;

        float u = (ascii % 16) / 16.0f;
        float v = (ascii / 16) / 16.0f;
        float uw = 1.0f / 16.0f;
        float vh = 1.0f / 16.0f;

        float fx = x + spacing * i;
        float fy = y;

        int vi = i * 6;
        vertices[vi + 0] = CTexturedVertex(XMFLOAT3(fx, fy, 0), XMFLOAT2(u, v));
        vertices[vi + 1] = CTexturedVertex(XMFLOAT3(fx + charSize, fy, 0), XMFLOAT2(u + uw, v));
        vertices[vi + 2] = CTexturedVertex(XMFLOAT3(fx + charSize, fy - charSize, 0), XMFLOAT2(u + uw, v + vh));
        vertices[vi + 3] = CTexturedVertex(XMFLOAT3(fx, fy, 0), XMFLOAT2(u, v));
        vertices[vi + 4] = CTexturedVertex(XMFLOAT3(fx + charSize, fy - charSize, 0), XMFLOAT2(u + uw, v + vh));
        vertices[vi + 5] = CTexturedVertex(XMFLOAT3(fx, fy - charSize, 0), XMFLOAT2(u, v + vh));
    }
    UINT64 bufferSize = sizeof(CTexturedVertex) * m_nVertices;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC bufDesc = {};
    bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufDesc.Width = bufferSize;
    bufDesc.Height = 1;
    bufDesc.DepthOrArraySize = 1;
    bufDesc.MipLevels = 1;
    bufDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufDesc.SampleDesc.Count = 1;
    bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    HRESULT hr = pd3dDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr, IID_PPV_ARGS(&m_pd3dPositionBuffer));

    if (FAILED(hr) || !m_pd3dPositionBuffer) {
        delete[] vertices;
        return;
    }

    void* pMapped = nullptr;
    m_pd3dPositionBuffer->Map(0, nullptr, &pMapped);
    // vertices is a raw pointer, not a container; use vertices directly instead of vertices.data()
    memcpy(pMapped, vertices, static_cast<size_t>(bufferSize));
    m_pd3dPositionBuffer->Unmap(0, nullptr);

    m_pd3dPositionUploadBuffer = nullptr; // 별도 업로드 버퍼 불필요
/*    m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, vertices, sizeof(CTexturedVertex) * m_nVertices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);*/

    m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
    m_d3dPositionBufferView.StrideInBytes = m_nStride;
    m_d3dPositionBufferView.SizeInBytes = static_cast<UINT>(sizeof(CTexturedVertex) * m_nVertices);

    delete[] vertices;
}

CFontMesh::~CFontMesh()
{
}