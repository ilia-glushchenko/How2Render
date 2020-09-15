#pragma once

#include "Helpers/MipmapGenerator.hpp"
#include "Helpers/TextureCache.hpp"
#include "ThirdParty/stb_image.h"
#include "Wrapper/Texture.hpp"
#include <cstdio>
#include <filesystem>
#include <string>
#include <tuple>

namespace h2r
{

	using TextureLoadFlags = uint32_t;
	constexpr TextureLoadFlags TEX_LOAD_FLAG_FLIP_VERTICALLY = 1;
	constexpr TextureLoadFlags TEX_LOAD_FLAG_GEN_MIPMAP = 2;

	inline std::tuple<bool, HostTexture> LoadTextureFromFile(
		TextureCache &cache,
		std::filesystem::path path,
		TextureLoadFlags flags)
	{
		if (path.empty() || !path.has_filename())
		{
			return {false, HostTexture{}};
		}

		auto [cacheFound, cachedTexture] = FindCachedHostTexture(cache, path);
		if (HasCachedHostTexture(cache, path))
		{
			return {true, cachedTexture};
		}

		stbi_set_flip_vertically_on_load(flags & TEX_LOAD_FLAG_FLIP_VERTICALLY);

		int32_t width, height, comp;
		auto *pixels = stbi_load(path.string().c_str(), &width, &height, &comp, STBI_rgb_alpha);
		if (!pixels)
		{
			wprintf(L"Failed to load texture: '%s'\n", path.c_str());
			return {false, HostTexture{}};
		}
		wprintf(L"Loaded texture %s\n", path.filename().c_str());

		HostTexture::Descriptor desc;
		desc.path = path;
		desc.pixels = (uint8_t *)pixels;
		desc.width = width;
		desc.height = height;
		desc.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		HostTexture hostTexture = CreateHostTexture(desc);
		if (flags & TEX_LOAD_FLAG_GEN_MIPMAP)
		{
			GenerateMipmap(hostTexture);
		}

		CacheHostTexture(cache, hostTexture);

		return {true, hostTexture};
	}

} // namespace h2r
