#pragma once
#include "stdafx.h"
#include "Shader.h"

class CDebugShader : public CShader
{
public:
    CDebugShader();
    virtual ~CDebugShader();

    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
    virtual D3D12_BLEND_DESC CreateBlendState() override;
    virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;

    virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
    virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
};

