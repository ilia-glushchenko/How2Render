#pragma once

#include "Application.hpp"
#include <d3d11.h>
#include <tuple>

struct Texture
{
	ID3D11Texture2D *texture;
	ID3D11ShaderResourceView *shaderResourceView;
	ID3D11RenderTargetView *renderTargetView;
	ID3D11SamplerState *pSamplerLinear;
};

struct RenderTargets
{
	Texture first;
	Texture second;
};

void ReleaseTexture(Texture &texture)
{
	if (texture.texture != nullptr)
	{
		texture.texture->Release();
	}
	if (texture.shaderResourceView != nullptr)
	{
		texture.shaderResourceView->Release();
	}
	if (texture.renderTargetView != nullptr)
	{
		texture.renderTargetView->Release();
	}
	if (texture.pSamplerLinear != nullptr)
	{
		texture.pSamplerLinear->Release();
	}
}

std::tuple<bool, Texture> CreateTexture(Application const &app, Window const &window)
{
	Texture result{nullptr, nullptr, nullptr};

	glm::ivec2 windowSize = GetWindowSize(window);

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = windowSize.x;
	desc.Height = windowSize.y;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	auto hr = app.pd3dDevice->CreateTexture2D(&desc, NULL, &result.texture);
	if (FAILED(hr))
	{
		printf("Failed to create texture.");
		ReleaseTexture(result);
		return {false, result};
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = desc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = desc.MipLevels;
	if (FAILED(app.pd3dDevice->CreateShaderResourceView(result.texture, &SRVDesc, &result.shaderResourceView)))
	{
		printf("Failed to create shader resource view");
		ReleaseTexture(result);
		return {false, result};
	}

	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
	RTVDesc.Format = desc.Format;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;
	if (FAILED(app.pd3dDevice->CreateRenderTargetView(result.texture, &RTVDesc, &result.renderTargetView)))
	{
		printf("Failed to create render target view.");
		ReleaseTexture(result);
		return {false, result};
	}

	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = app.pd3dDevice->CreateSamplerState(&sampDesc, &result.pSamplerLinear);
	if (FAILED(hr))
	{
		printf("Failed to create render target view.");
		ReleaseTexture(result);
		return {false, result};
	}

	return {true, result};
}