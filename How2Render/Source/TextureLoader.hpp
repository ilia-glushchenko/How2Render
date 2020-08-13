#pragma once
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb_image.h"
#endif
#include "Wrapper/Texture.hpp"
#include "Helpers/MipmapGenerator.hpp"
#include <cstdio>
#include <map>
#include <string>
#include <tuple>

namespace h2r
{
	struct TextureLoader
	{
		Context context;
		std::string baseDir = ".";
		bool flipVertically = true;
		bool generateMipmap = true;
		std::map<std::string, DeviceTexture> textureCache;
	};

	TextureLoader CreateTextureLoader(Context const& context, bool flipVertically, bool generateMipmap,
		const std::string& baseDir = ".")
	{
		TextureLoader loader;

		loader.context = context;
		loader.flipVertically = flipVertically;
		loader.generateMipmap = generateMipmap;
		loader.baseDir = baseDir;

		return loader;
	}

	void FreeTextureLoader(TextureLoader& loader)
	{
		for (auto& tex : loader.textureCache)
			ReleaseTexture(tex.second);

		loader.textureCache.clear();
	}

	std::tuple<bool, DeviceTexture> FindCachedTexture(TextureLoader const& loader, std::string const& name)
	{
		auto it = loader.textureCache.find(name);
		if (it != loader.textureCache.end())
			return {true, it->second};

		return {false, DeviceTexture{}};
	}

	std::tuple<bool, HostTexture> LoadHostTextureFromFile(TextureLoader const& loader, const std::string& path)
	{
		HostTexture hostTexture;
		int width, height, comp;

		stbi_set_flip_vertically_on_load(loader.flipVertically);
		auto *pixels = stbi_load(path.c_str(), &width, &height, &comp, STBI_rgb_alpha);
		if (!pixels)
			return {false, hostTexture};

		hostTexture.pixels = (uint8_t *)pixels;
		hostTexture.width = width;
		hostTexture.height = height;
		hostTexture.format = DXGI_FORMAT_R8G8B8A8_UNORM;

		uint32_t const imageSize = (uint32_t)(width * height * sizeof(RGBQUAD));
		hostTexture.mipChain.push_back(
			HostTextureMipLevel{
				hostTexture.width,
				hostTexture.height,
				0U,
				imageSize,
				hostTexture.pixels
			}
		);

		return {true, hostTexture};
	}

	void CleanupHostTexture(HostTexture& hostTexture)
	{
		if (hostTexture.pixels != nullptr)
		{
			stbi_image_free(hostTexture.pixels);
			hostTexture.pixels = nullptr;
		}

		hostTexture.width = hostTexture.height = 0;
		hostTexture.mipChain.clear();
	}

	std::tuple<bool, DeviceTexture> LoadDeviceTextureFromFile(TextureLoader& loader, std::string const& fileName)
	{
		if (fileName.empty())
			return {false, DeviceTexture{}};

		std::string const filePath = loader.baseDir + "\\" + fileName;
		auto [found, cachedTexture] = FindCachedTexture(loader, filePath);
		if (found)
			return {true, cachedTexture};

		printf("Load texture %s\n", filePath.c_str());

		auto [loadResult, hostTexture] = LoadHostTextureFromFile(loader, filePath);
		if (!loadResult)
			return {false, DeviceTexture{}};

		if (loader.generateMipmap)
			GenerateMipmap(hostTexture);

		auto [textureResult, deviceTexture] = CreateTexture(loader.context, hostTexture);
		if (!textureResult)
			return {false, DeviceTexture{}};

		CleanupHostTexture(hostTexture);
		loader.textureCache.emplace(filePath, deviceTexture);
		return {true, deviceTexture};
	}
} // namespace h2r
