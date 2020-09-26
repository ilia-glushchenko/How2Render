#pragma once

#include "Wrapper/Texture.hpp"
#include <filesystem>
#include <map>

namespace h2r
{

	struct TextureCache
	{
		std::map<std::filesystem::path, HostTexture> hostTextureMap;
		std::map<std::filesystem::path, DeviceTexture> deviceTextureMap;
	};

	inline bool CacheHostTexture(TextureCache &cache, HostTexture texture)
	{
		return cache.hostTextureMap.emplace(texture.path, texture).second;
	}

	inline bool CacheDeviceTexture(TextureCache &cache, DeviceTexture texture)
	{
		return cache.deviceTextureMap.emplace(texture.path, texture).second;
	}

	inline bool HasCachedHostTexture(TextureCache const &cache, std::filesystem::path const &path)
	{
		return cache.hostTextureMap.contains(path);
	}

	inline bool HasCachedDeviceTexture(TextureCache const &cache, std::filesystem::path const &path)
	{
		return cache.deviceTextureMap.contains(path);
	}

	inline std::tuple<bool, HostTexture> FindCachedHostTexture(TextureCache const &cache, std::filesystem::path const &path)
	{
		auto it = cache.hostTextureMap.find(path);
		if (it != cache.hostTextureMap.end())
		{
			return {true, it->second};
		}

		return {false, HostTexture{}};
	}

	inline std::tuple<bool, DeviceTexture> FindCachedDeviceTexture(TextureCache const &cache, std::filesystem::path const &path)
	{
		auto it = cache.deviceTextureMap.find(path);
		if (it != cache.deviceTextureMap.end())
		{
			return {true, it->second};
		}

		return {false, DeviceTexture{}};
	}

	inline void FlushHostTextureCache(TextureCache &cache)
	{
		cache.hostTextureMap.clear();
	}

	inline void FlushDeviceTextureCache(TextureCache &cache)
	{
		for (auto &tex : cache.deviceTextureMap)
		{
			CleanupDeviceTexture(tex.second);
		}
		cache.deviceTextureMap.clear();
	}

	inline void FlushTextureCache(TextureCache &cache)
	{
		FlushHostTextureCache(cache);
		FlushDeviceTextureCache(cache);
	}

} // namespace h2r