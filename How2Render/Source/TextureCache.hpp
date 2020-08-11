#pragma once
#include "Wrapper/Texture.hpp"
#include <map>
#include <string>
#include <tuple>

namespace h2r
{
	struct TextureCache
	{
		std::map<std::string, DeviceTexture> textures;
	};

	std::tuple<bool, DeviceTexture> FindCachedTexture(TextureCache const& cache, std::string const& name)
	{
		auto it = cache.textures.find(name);
		if (it != cache.textures.end())
			return {true, it->second};

		return {false, DeviceTexture{}};
	}

	void CacheTexture(TextureCache& cache, std::string const& name, DeviceTexture& texture)
	{
		cache.textures.emplace(name, texture);
	}

	void FreeTextureCache(TextureCache& cache)
	{
		for (auto& tex : cache.textures)
			ReleaseTexture(tex.second);

		cache.textures.clear();
	}
} // namespace h2r
