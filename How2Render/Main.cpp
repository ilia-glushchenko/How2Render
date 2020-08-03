#include "Wrapper/Texture.hpp"
#include "Wrapper/Sampler.hpp"
#include "Wrapper/ConstantBuffer.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "Input.hpp"
#include "Application.hpp"
#include "Math.hpp"
#include "Image.hpp"
#include "MipmapGenerator.hpp"
#include "SphereMesh.hpp"

#include <directxcolors.h>

struct RenderObject
{
	Mesh mesh;
	Texture texture;
	ID3D11SamplerState *sampler;
	XMMATRIX world;
};

RenderObject CreateRenderObject(Context const& context)
{
	auto[loadResult, image] = LoadImageFromFile("Images/earth.bmp");
	assert(loadResult);

	GenerateMipmap(image);

	auto[textureResult, texture] = CreateTexture(context, image);
	assert(textureResult);

	CleanupImage(image);

	RenderObject object;

	object.mesh = GenerateSphere(context, 10.f, 64);
	object.texture = texture;
	object.sampler = CreateSampler(context, eFilterType::Trilinear);
	object.world = XMMatrixRotationY(XMConvertToRadians(20.f)); // Look at Europe

	return object;
}

void CleanupRenderObject(RenderObject& renderObject)
{
	CleanupMesh(renderObject.mesh);
	ReleaseTexture(renderObject.texture);

	if (renderObject.sampler)
	{
		renderObject.sampler->Release();
		renderObject.sampler = nullptr;
	}
}

void Render(
	HostConstantBuffer const& constantBuffer,
	Application const& app,
	Camera const& camera,
	RenderObject const& renderObject
)
{
	app.context.pImmediateContext->ClearRenderTargetView(app.swapchain.pRenderTargetView,
		DirectX::Colors::AliceBlue);

	// Update variables
	app.context.pImmediateContext->UpdateSubresource(
		app.shaders.pConstantBuffer, 0, nullptr, &constantBuffer, 0, 0);

	// Setup vertex/pixel shader and bind sampler/texture
	app.context.pImmediateContext->VSSetShader(app.shaders.pVertexShader, nullptr, 0);
	app.context.pImmediateContext->VSSetConstantBuffers(0, 1, &app.shaders.pConstantBuffer);
	app.context.pImmediateContext->PSSetShader(app.shaders.pPixelShader, nullptr, 0);
	app.context.pImmediateContext->PSSetSamplers(0, 1, &renderObject.sampler);
	app.context.pImmediateContext->PSSetShaderResources(0, 1, &renderObject.texture.shaderResourceView);

	constexpr uint32_t stride = sizeof(DirectX::GeometricPrimitive::VertexType);
	constexpr uint32_t offset = 0;
	const Mesh& sphere = renderObject.mesh;

	// Render sphere
	app.context.pImmediateContext->IASetVertexBuffers(0, 1, &sphere.vertexBuffer, &stride, &offset);
	app.context.pImmediateContext->IASetIndexBuffer(sphere.indexBuffer, sphere.indexFormat, 0);
	app.context.pImmediateContext->IASetPrimitiveTopology(sphere.topology);
	app.context.pImmediateContext->DrawIndexed(sphere.numIndices, 0, 0);

	ID3D11ShaderResourceView* const pSRV[1] = {nullptr};
	app.context.pImmediateContext->PSSetShaderResources(0, 1, pSRV);

	// Present the information rendered to the back buffer to the front buffer (the screen)
	app.swapchain.pSwapChain->Present(0, 0);
}

int main(int argc, char *args[])
{
	Window window = CreateNewWindow(640, 640);
	Camera camera = CreateDefaultCamera();
	InputEvents inputEvents = CreateDefaultInputEvents();
	Application application = CreateApplication(window);
	RenderObject renderObject = CreateRenderObject(application.context);

	HostConstantBuffer constBuffer;

	while (!inputEvents.quit)
	{
		UpdateInput(inputEvents);
		UpdateCamera(camera, inputEvents, window);
		constBuffer.worldViewProj = renderObject.world * camera.viewProj;

		Render(
			constBuffer,
			application,
			camera,
			renderObject
		);
	}

	CleanupRenderObject(renderObject);
	CleanupApplication(application);
	DestroyWindow(window);

	return 0;
}
