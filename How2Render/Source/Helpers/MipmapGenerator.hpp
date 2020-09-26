#pragma once

#include "Wrapper/Texture.hpp"
#include <cstdint>
#include <vector>

namespace h2r
{

	inline RGBQUAD *GetTexel(
		std::vector<uint8_t> &pixels, HostTexture::MipLevel const &mipLevel, uint32_t col, uint32_t row)
	{
		const uint32_t index = row * mipLevel.width + col;
		const size_t offset = index * sizeof(RGBQUAD);
		return (RGBQUAD *)(pixels.data() + mipLevel.byteOffset + offset);
	}

	//ToDo: This will cause UB if called with non 4 byte pixel size image
	inline void BoxDownsample(
		std::vector<uint8_t> &pixels,
		HostTexture::MipLevel const &nextMip,
		HostTexture::MipLevel const &currMip)
	{
		for (uint32_t j = 0; j < currMip.height; j += 2)
		{
			for (uint32_t i = 0; i < currMip.width; i += 2)
			{
				const uint32_t txIndex = j * currMip.width + i;
				const RGBQUAD *tx0 = GetTexel(pixels, currMip, i, j);
				const RGBQUAD *tx1 = GetTexel(pixels, currMip, i + 1, j);
				const RGBQUAD *tx2 = GetTexel(pixels, currMip, i, j + 1);
				const RGBQUAD *tx3 = GetTexel(pixels, currMip, i + 1, j + 1);
				RGBQUAD *dst = GetTexel(pixels, nextMip, i >> 1, j >> 1);
				dst->rgbRed = (tx0->rgbRed + tx1->rgbRed + tx2->rgbRed + tx3->rgbRed) >> 2;
				dst->rgbGreen = (tx0->rgbGreen + tx1->rgbGreen + tx2->rgbGreen + tx3->rgbGreen) >> 2;
				dst->rgbBlue = (tx0->rgbBlue + tx1->rgbBlue + tx2->rgbBlue + tx3->rgbBlue) >> 2;
				dst->rgbReserved = (tx0->rgbReserved + tx1->rgbReserved + tx2->rgbReserved + tx3->rgbReserved) >> 2;
			}
		}
	}

	inline std::vector<HostTexture::MipLevel> CalculateMipChain(HostTexture const &texture)
	{
		HostTexture::MipLevel mipLevel;
		mipLevel.width = texture.width;
		mipLevel.height = texture.height;
		mipLevel.lodIndex = 0;
		mipLevel.byteSize = mipLevel.width * mipLevel.height * (uint32_t)BytesPerPixel(texture.format);
		mipLevel.byteOffset = 0;

		std::vector<HostTexture::MipLevel> mipChain;
		uint32_t mipLevelByteOffset = mipLevel.byteSize;

		while (true)
		{
			mipChain.push_back(mipLevel);
			if ((1 == mipLevel.width) && (1 == mipLevel.height))
			{
				break;
			}

			if (mipLevel.width > 1)
			{
				mipLevel.width >>= 1;
			}
			if (mipLevel.height > 1)
			{
				mipLevel.height >>= 1;
			}
			++mipLevel.lodIndex;
			mipLevel.byteSize = mipLevel.width * mipLevel.height * (uint32_t)BytesPerPixel(texture.format);
			mipLevel.byteOffset = mipLevelByteOffset;

			mipLevelByteOffset += mipLevel.byteSize;
		}

		return mipChain;
	}

	inline bool GenerateMipmap(HostTexture &texture)
	{
		if (texture.pixels.empty() || texture.mipChain.size() > 1)
		{
			return false;
		}

		texture.mipChain = CalculateMipChain(texture);

		uint32_t mipMappedImageSize = 0;
		for (auto const &mip : texture.mipChain)
		{
			mipMappedImageSize += mip.byteSize;
		}
		texture.pixels.resize(mipMappedImageSize);

		for (size_t i = 1; i < texture.mipChain.size(); ++i)
		{
			BoxDownsample(texture.pixels, texture.mipChain[i], texture.mipChain[i - 1]);
		}

		return true;
	}

} // namespace h2r
