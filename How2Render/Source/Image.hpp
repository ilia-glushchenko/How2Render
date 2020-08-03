#include "Wrapper/Texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::tuple<bool, Image> LoadImageFromFile(const std::string& fileName)
{
	Image image;

	int width, height, comp;
	auto *pixels = stbi_load(fileName.c_str(), &width, &height, &comp, STBI_rgb_alpha);
	if (!pixels)
		return {false, image};

	image.pixels = pixels;
	image.width = width;
	image.height = height;
	image.format = DXGI_FORMAT_R8G8B8A8_UNORM;

	const uint32_t imageSize = (uint32_t)(width * height * sizeof(RGBQUAD));
	MipLevel mipLevel{image.width, image.height, 0U, imageSize, image.pixels};
	image.mipChain.push_back(mipLevel);
	return {true, image};
}

void CleanupImage(Image& image)
{
	if (image.pixels != nullptr)
	{
		stbi_image_free(image.pixels);
		image.pixels = nullptr;
	}
	image.width = image.height = 0;
	image.mipChain.clear();
}
