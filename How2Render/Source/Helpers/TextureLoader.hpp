#include "Wrapper/Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb_image.h"

#include <cstdint>
#include <string>
#include <tuple>

namespace h2r
{

	inline std::tuple<bool, HostTexture> LoadHostTextureFromFile(const std::string &path)
	{
		HostTexture hostTexture;

		int width, height, comp;
		auto *pixels = stbi_load(path.c_str(), &width, &height, &comp, STBI_rgb_alpha);
		if (!pixels)
		{
			return {false, hostTexture};
		}

		hostTexture.pixels = pixels;
		hostTexture.width = width;
		hostTexture.height = height;
		hostTexture.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		uint32_t const imageSize = (uint32_t)(width * height * sizeof(RGBQUAD));
		hostTexture.mipChain.push_back(
			HostTextureMipLevel{
				hostTexture.width,
				hostTexture.height,
				0U,
				imageSize,
				hostTexture.pixels});

		return {true, hostTexture};
	}

	inline void CleanupHostTexture(HostTexture &hostTexture)
	{
		if (hostTexture.pixels != nullptr)
		{
			stbi_image_free(hostTexture.pixels);
			hostTexture.pixels = nullptr;
		}

		hostTexture.width = hostTexture.height = 0;
		hostTexture.mipChain.clear();
	}

} // namespace h2r
