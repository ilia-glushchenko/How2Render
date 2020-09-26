#pragma once

#include "Helpers/MipmapGenerator.hpp"
#include "Random.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/Texture.hpp"

namespace h2r
{

	inline DeviceTexture GenerateNoiseTexture(Context const &context, uint32_t width, uint32_t height)
	{
		assert((width % 2 == 0) && (height % 2 == 0));

		HostTexture::Descriptor hostDesc;
		hostDesc.format = DXGI_FORMAT_R8_UNORM;
		hostDesc.height = height;
		hostDesc.width = width;
		hostDesc.path = L"Noise";
		uint64_t const imageByteSize = static_cast<uint64_t>(width) * height * BytesPerPixel(hostDesc.format);
		std::vector<uint8_t> pixels(imageByteSize);
		hostDesc.pixels = pixels.data();

		for (uint8_t &pixel : pixels)
		{
			pixel = static_cast<uint8_t>(UINT8_MAX * GenerateUniformRealDist(0.f, 1.f));
		}

		DeviceTexture::Descriptor devDesc;
		devDesc.hostTexture = CreateHostTexture(hostDesc);
		devDesc.bindFlags = D3D11_BIND_SHADER_RESOURCE;
		devDesc.mipmapFlag = DeviceTexture::Descriptor::eMipMapFlag::NONE;

		return CreateDeviceTexture(context, devDesc).value();
	}

} // namespace h2r