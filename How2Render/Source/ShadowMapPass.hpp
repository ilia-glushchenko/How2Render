#pragma once

#include "Application.hpp"
#include "RenderObject.hpp"
#include "SpotLight.hpp"

namespace h2r
{
	void ShadowMapPass(Application const& app, std::vector<RenderObject> const& renderObjects, SpotLight const& spotLight)
	{
		D3D11_VIEWPORT viewport, oldViewport;
		UINT numViewports = 1;
		auto shadowMap = spotLight.shadowMap;

		viewport.Width = (float)shadowMap.texture.width;
		viewport.Height = (float)shadowMap.texture.height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;

		app.context.pImmediateContext->RSGetViewports(&numViewports, &oldViewport);
		app.context.pImmediateContext->RSSetViewports(1, &viewport);
		app.context.pImmediateContext->ClearDepthStencilView(spotLight.shadowMap.pDepthStencilView, D3D11_CLEAR_DEPTH, 1.f, 0);

		app.context.pImmediateContext->VSSetShader(app.shaders.pDepthVertexShader, nullptr, 0);
		app.context.pImmediateContext->VSSetConstantBuffers(0, 1, &app.shaders.pConstantBuffer);
		app.context.pImmediateContext->PSSetShader(app.shaders.pDepthPixelShader, nullptr, 0);
		app.context.pImmediateContext->PSSetConstantBuffers(1, 1, &app.shaders.pMaterialConstants);

		app.context.pImmediateContext->OMSetRenderTargets(0, nullptr, shadowMap.pDepthStencilView);
		app.context.pImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

		TransformConstantBuffer transforms;

		// Not used
		transforms.world = XMMatrixIdentity();
		transforms.worldView = XMMatrixIdentity();
		transforms.normal = XMMatrixIdentity();
		transforms.shadowProj = XMMatrixIdentity();
		transforms.lightViewPos = XMVectorZero();

		for (auto const& object : renderObjects)
		{
			// Update transform
			transforms.worldViewProj = XMMatrixMultiply(object.world, spotLight.viewProj);
			app.context.pImmediateContext->UpdateSubresource(app.shaders.pConstantBuffer, 0, nullptr, &transforms, 0, 0);

			DrawModel(object.model, app.context, app.shaders, app.blendStates);
		}

		app.context.pImmediateContext->RSSetViewports(1, &oldViewport);
	}

} // namespace h2r
