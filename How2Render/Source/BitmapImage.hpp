#include "Wrapper/Texture.hpp"

std::tuple<bool, Image> LoadBitmapImage(const std::string& fileName)
{
	Image image;

	FILE *file = nullptr;
	fopen_s(&file, fileName.c_str(), "rb");
	if (!file)
		return {false, image};

	// Read the header
	BITMAPFILEHEADER fileHeader;
	fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file);
	if (fileHeader.bfType != MAKEFOURCC('B', 'M', 0, 0))
	{
		fclose(file);
		return {false, image};
	}

	BITMAPINFOHEADER infoHeader;
	fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, file);

	const LONG imageSize = infoHeader.biWidth * infoHeader.biHeight * sizeof(RGBQUAD);
	image.width = infoHeader.biWidth;
	image.height = infoHeader.biHeight;
	image.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	image.pixels = new uint8_t[imageSize];

	switch (infoHeader.biBitCount)
	{
	case 24:
		{
			std::vector<RGBTRIPLE> row(image.width);
			for (int i = image.height - 1; i >= 0; --i)
			{
				fread(row.data(), sizeof(RGBTRIPLE), image.width, file);
				uint8_t *dest = image.pixels + i * image.width * 4;
				for (uint32_t j = 0; j < image.width; ++j)
				{
					*dest++ = row[j].rgbtRed;
					*dest++ = row[j].rgbtGreen;
					*dest++ = row[j].rgbtBlue;
					*dest++ = 255;
				}
			}
		}
		break;

	case 32:
		{
			std::vector<RGBQUAD> row(image.width);
			for (int i = image.height - 1; i >= 0; --i)
			{
				fread(row.data(), sizeof(RGBQUAD), image.width, file);
				uint8_t *dest = image.pixels + i * image.width * 4;
				for (uint32_t j = 0; j < image.width; ++j)
				{
					*dest++ = row[j].rgbRed;
					*dest++ = row[j].rgbGreen;
					*dest++ = row[j].rgbBlue;
					*dest++ = row[j].rgbReserved; // Alpha
				}
			}
		}
		break;

	default:
		fclose(file);
		return {false, image};
	}

	fclose(file);
	MipLevel mipLevel{image.width, image.height, 0U, (uint32_t)imageSize, image.pixels};
	image.mipChain.push_back(mipLevel);
	return {true, image};
}

void CleanupImage(Image& image)
{
	if (image.pixels != nullptr)
	{
		delete[] image.pixels;
		image.pixels = nullptr;
	}
	image.width = image.height = 0;
	image.mipChain.clear();
}
