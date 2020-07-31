#pragma once
#include "Wrapper/Texture.hpp"
#include <vector>
#include <cstdint>

namespace h2r
{

	inline RGBQUAD *GetTexel(HostTextureMipLevel& mipLevel, uint32_t col, uint32_t row)
	{
		const uint32_t index = row * mipLevel.width + col;
		const size_t offset = index * sizeof(RGBQUAD);
		return (RGBQUAD *)(mipLevel.data + offset);
	}

	inline const RGBQUAD *GetTexel(HostTextureMipLevel const& mipLevel, uint32_t col, uint32_t row)
	{
		const uint32_t index = row * mipLevel.width + col;
		const size_t offset = index * sizeof(RGBQUAD);
		return (const RGBQUAD *)(mipLevel.data + offset);
	}

	inline void BoxDownsample(HostTextureMipLevel& nextMip, HostTextureMipLevel const& currMip)
	{
		for (uint32_t j = 0; j < currMip.height; j += 2)
		{
			for (uint32_t i = 0; i < currMip.width; i += 2)
			{
				const uint32_t txIndex = j * currMip.width + i;
				const RGBQUAD *tx0 = GetTexel(currMip, i, j);
				const RGBQUAD *tx1 = GetTexel(currMip, i + 1, j);
				const RGBQUAD *tx2 = GetTexel(currMip, i, j + 1);
				const RGBQUAD *tx3 = GetTexel(currMip, i + 1, j + 1);
				RGBQUAD *dst = GetTexel(nextMip, i >> 1, j >> 1);
				dst->rgbRed = (tx0->rgbRed + tx1->rgbRed + tx2->rgbRed + tx3->rgbRed) >> 2;
				dst->rgbGreen = (tx0->rgbGreen + tx1->rgbGreen + tx2->rgbGreen + tx3->rgbGreen) >> 2;
				dst->rgbBlue = (tx0->rgbBlue + tx1->rgbBlue + tx2->rgbBlue + tx3->rgbBlue) >> 2;
				dst->rgbReserved = (tx0->rgbReserved + tx1->rgbReserved + tx2->rgbReserved + tx3->rgbReserved) >> 2;
			}
		}
	}

	inline std::vector<HostTextureMipLevel> CalculateMipChain(HostTexture const& image)
	{
		HostTextureMipLevel mipLevel;

		mipLevel.width = image.width;
		mipLevel.height = image.height;
		mipLevel.lodIndex = 0;
		mipLevel.byteSize = mipLevel.width * mipLevel.height * sizeof(RGBQUAD);
		mipLevel.data = nullptr;

		std::vector<HostTextureMipLevel> mipChain;

		while (true)
		{
			mipChain.push_back(mipLevel);
			if ((1 == mipLevel.width) && (1 == mipLevel.height))
				break;

			if (mipLevel.width > 1)
				mipLevel.width >>= 1;
			if (mipLevel.height > 1)
				mipLevel.height >>= 1;
			++mipLevel.lodIndex;
			mipLevel.byteSize = mipLevel.width * mipLevel.height * sizeof(RGBQUAD);
		}

		return mipChain;
	}

	inline bool GenerateMipmap(HostTexture& image)
	{
		if (!image.pixels)
			return false;

		if (image.mipChain.size() > 1)
			return false;

		auto mipChain = CalculateMipChain(image);

		uint32_t mipMappedImageSize = 0;
		for (auto const& mip : mipChain)
			mipMappedImageSize += mip.byteSize;

		uint8_t *mipPixels = new uint8_t[mipMappedImageSize];
		if (!mipPixels)
		{
			printf("Failed to allocate memory for mip maps");
			return false;
		}

		image.mipChain = std::move(mipChain);
		HostTextureMipLevel& firstMip = image.mipChain.front();
		memcpy(mipPixels, image.pixels, firstMip.byteSize);
		firstMip.data = mipPixels;

		for (size_t i = 1; i < image.mipChain.size(); ++i)
		{
			const HostTextureMipLevel& currMip = image.mipChain[i - 1];
			HostTextureMipLevel& nextMip = image.mipChain[i];
			nextMip.data = currMip.data + currMip.byteSize;
			BoxDownsample(nextMip, currMip);
		}

		delete[] image.pixels;
		image.pixels = mipPixels;
		return true;
	}

} // namespace h2r
