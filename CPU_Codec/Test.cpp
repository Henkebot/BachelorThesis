// BachelorThesis.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#define STB_IMAGE_IMPLEMENTATION
#include "Utility/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "Utility/stb_image_write.h"

const char* TestImages[]	 = {"../Resources/8bit/artificial.png",
							"../Resources/8bit/big_building.png",
							"../Resources/8bit/big_tree.png",
							"../Resources/8bit/deer.png",
							"../Resources/8bit/fireworks.png",
							"../Resources/8bit/flower_foveon.png",
							"../Resources/8bit/hdr.png",
							"../Resources/8bit/leaves_iso_1600.png",
							"../Resources/8bit/nightshot_iso_1600.png",
							"../Resources/8bit/spider_web.png"};
const char* TestImagesJPEG[] = {"../Resources/8bit/artificial.jpg",
								"../Resources/8bit/big_building.jpg",
								"../Resources/8bit/big_tree.jpg",
								"../Resources/8bit/deer.jpg",
								"../Resources/8bit/fireworks.jpg",
								"../Resources/8bit/flower_foveon.jpg",
								"../Resources/8bit/hdr.jpg",
								"../Resources/8bit/leaves_iso_1600.jpg",
								"../Resources/8bit/nightshot_iso_1600.jpg",
								"../Resources/8bit/spider_web.jpg"};

// Compression
long long totalBitsRequired = 0;
float DCT_MATRIX[64];
float DCT_MATRIX_TRANSPOSE[64];

void SetupTables();

int CreateBitstream(int& bitBuf,
					int& bitCnt,
					int prevDC,
					float DCT_r[64],
					const unsigned short DC_HT[256][2],
					const unsigned short AC_HT[256][2]);
enum FUNCTION
{
	FUNCTION_GAZE = 0,
	FUNCTION_FOV  = 1
};

struct GAZE_Setting
{
	float radius;
	float innerQuality;
	int function;
};

long long CalculateSize(int height, int width, stbi_uc* imageData, int comp, GAZE_Setting* setting);

unsigned int
CalculateQuality(int currentX, int currentY, int gazeX, int gazeY, GAZE_Setting* setting);

void SetupQuantTables(unsigned char _pLumTable[64],
					  unsigned char _pChromaTable[64],
					  unsigned int quality);

// Entropy Coding

unsigned char fileBuffer[512] = {};
unsigned char* filebufferPtr  = fileBuffer;
namespace Thesis
{
	void writebits(int* bitBufP, int* bitCntP, const unsigned short* bs);
	void calcbits(int val, unsigned short bits[2]);
} // namespace Thesis

int main()
{

	SetupTables();

	int width, height, comp;
	stbi_uc* imageData = nullptr;
	for(int i = 0; i < ARRAYSIZE(TestImages); i++)
	{
		imageData = stbi_load(TestImages[i], &width, &height, &comp, 3);
		stbi_write_jpg(TestImagesJPEG[i], width, height, comp, imageData, 85);
	}
	return 0;
	GAZE_Setting settings[10];

	settings[0].innerQuality = 1.0f;
	settings[0].radius		 = 1.0f;
	settings[0].function	 = FUNCTION_GAZE;

	settings[1].innerQuality = 0.8f;
	settings[1].radius		 = 1.0f;
	settings[1].function	 = FUNCTION_GAZE;

	settings[2].innerQuality = 0.6f;
	settings[2].radius		 = 1.0f;
	settings[2].function	 = FUNCTION_GAZE;

	settings[3].innerQuality = 1.0f;
	settings[3].radius		 = 0.8f;
	settings[3].function	 = FUNCTION_GAZE;

	settings[4].innerQuality = 1.0f;
	settings[4].radius		 = 0.6f;
	settings[4].function	 = FUNCTION_GAZE;

	settings[5].innerQuality = 1.0f;
	settings[5].radius		 = 1.0f;
	settings[5].function	 = FUNCTION_FOV;

	settings[6].innerQuality = 0.8f;
	settings[6].radius		 = 1.0f;
	settings[6].function	 = FUNCTION_FOV;

	settings[7].innerQuality = 0.6f;
	settings[7].radius		 = 1.0f;
	settings[7].function	 = FUNCTION_FOV;

	settings[8].innerQuality = 1.0f;
	settings[8].radius		 = 0.8f;
	settings[8].function	 = FUNCTION_FOV;

	settings[9].innerQuality = 1.0f;
	settings[9].radius		 = 0.6f;
	settings[9].function	 = FUNCTION_FOV;
	std::ofstream output("ResultsFromCompression.txt");

	output
		<< "Image\tLinear/Radius 1.0f/Quality 1.0f\tLinear/Radius 1.0f/Quality 0.8f\tLinear/Radius "
		   "1.0f/Quality "
		   "0.6f\tLinear/Radius 0.8f/Quality 1.0f\tLinear/Radius 0.6f/Quality "
		   "1.0f\tFov/Radius 1.0f/Quality 1.0f\tFov/Radius 1.0f/Quality 0.8f\tFov/Radius "
		   "1.0f/Quality 0.6f\tFov/Radius 0.8f/Quality 1.0f\tFov/Radius 0.6f/Quality 1.0f\tNormal "
		   "JPEG\n";

	for(int j = 0; j < ARRAYSIZE(TestImages); j++)
	{
		imageData = stbi_load(TestImages[j], &width, &height, &comp, 3);
		output << TestImages[j] + 18 << "\t";
		for(int i = 0; i < ARRAYSIZE(settings); i++)
		{
			long long result = CalculateSize(height, width, imageData, comp, &settings[i]);
			output << result << "\t";
		}
		long long result = CalculateSize(height, width, imageData, comp, nullptr);
		output << result << "\n";
		delete[] imageData;
	}
	output.close();
	system("pause");
	return 0;
}

long long CalculateSize(int height, int width, stbi_uc* imageData, int comp, GAZE_Setting* setting)
{
	totalBitsRequired = 0;
	int DCR = 0, DCG = 0, DCB = 0;
	int bitBuf = 0, bitCnt = 0;
	int middleX = width >> 1;
	int middleY = height >> 1;

	for(unsigned int y = 0; y < height; y += 8)
	{
		for(unsigned int x = 0; x < width; x += 8)
		{
			// Every block
			{
				// 1. Calcualte the quality variable
				unsigned int quality = 85;
				if(setting != nullptr)
					quality = CalculateQuality(x, y, middleX, middleY, setting);

				unsigned char tempLumaTable[64];
				unsigned char tempChromaTable[64];

				memcpy(tempLumaTable, Thesis::LuminanceQT, sizeof(unsigned char) * 64);
				memcpy(tempChromaTable, Thesis::ChromaQT, sizeof(unsigned char) * 64);

				SetupQuantTables(tempLumaTable, tempChromaTable, quality);

				float r[64], g[64], b[64];

				for(int blockY = 0; blockY < 8; blockY++)
				{
					for(int blockX = 0; blockX < 8; blockX++)
					{
						// Color convert

						const unsigned char rCol =
							imageData[((x + blockX) + ((y + blockY) * width)) * comp + 0];
						const unsigned char gCol =
							imageData[((x + blockX) + ((y + blockY) * width)) * comp + 1];
						const unsigned char bCol =
							imageData[((x + blockX) + ((y + blockY) * width)) * comp + 2];

						float rr = rCol / 256.0f;
						float gg = gCol / 256.0f;
						float bb = bCol / 256.0f;

						DirectX::XMFLOAT3X3 rgbToYuv = {0.2126,
														0.7152,
														0.0722,
														-0.1145721060573399,
														-0.3854278939426601,
														0.5,
														0.5,
														-0.4541529083058166,
														-0.0458470916941834};

						DirectX::XMVECTOR colorVector = {rr, gg, bb, 1.0f};
						DirectX::XMMATRIX mat		  = DirectX::XMLoadFloat3x3(&rgbToYuv);

						DirectX::XMFLOAT3 outputPixels;
						DirectX::XMStoreFloat3(&outputPixels,
											   DirectX::XMVector3Transform(colorVector, mat));

						// Convert to YUV
						rr = ((outputPixels.x + 0.0f) * 255.0f) - 128.0f;
						gg = ((outputPixels.y + 0.5f) * 255.0f) - 128.0f;
						bb = ((outputPixels.z + 0.5f) * 255.0f) - 128.0f;

						// Dont convert to YUV
						/*	rr = (rr * 255.0f) - 128.0f;
						gg = (bb * 255.0f) - 128.0f;
						bb = (bb * 255.0f) - 128.0f;*/

						r[(blockX + blockY * 8)] = rr;
						g[(blockX + blockY * 8)] = gg;
						b[(blockX + blockY * 8)] = bb;
					}
				}

				float DCT_r[64], DCT_rTemp[64], DCT_g[64], DCT_gTemp[64], DCT_b[64], DCT_bTemp[64];

				memset(DCT_r, 0.0f, sizeof(float) * 64);
				memset(DCT_rTemp, 0.0f, sizeof(float) * 64);

				memset(DCT_g, 0.0f, sizeof(float) * 64);
				memset(DCT_gTemp, 0.0f, sizeof(float) * 64);

				memset(DCT_b, 0.0f, sizeof(float) * 64);
				memset(DCT_bTemp, 0.0f, sizeof(float) * 64);

				for(int blockX = 0; blockX < 8; blockX++)
				{
					for(int blockY = 0; blockY < 8; blockY++)
					{
						int gIndex = blockX + blockY * 8;

						for(int k = 0; k < 8; k++)
						{
							DCT_rTemp[gIndex] += DCT_MATRIX[blockY * 8 + k] * r[k * 8 + blockX];
							DCT_gTemp[gIndex] += DCT_MATRIX[blockY * 8 + k] * g[k * 8 + blockX];
							DCT_bTemp[gIndex] += DCT_MATRIX[blockY * 8 + k] * b[k * 8 + blockX];
						}
					}
				}

				for(int blockX = 0; blockX < 8; blockX++)
				{
					for(int blockY = 0; blockY < 8; blockY++)
					{
						int gIndex = blockX + blockY * 8;

						for(int k = 0; k < 8; k++)
						{
							DCT_r[gIndex] +=
								DCT_rTemp[blockY * 8 + k] * DCT_MATRIX_TRANSPOSE[k * 8 + blockX];
							DCT_g[gIndex] +=
								DCT_gTemp[blockY * 8 + k] * DCT_MATRIX_TRANSPOSE[k * 8 + blockX];
							DCT_b[gIndex] +=
								DCT_bTemp[blockY * 8 + k] * DCT_MATRIX_TRANSPOSE[k * 8 + blockX];
						}
					}
				}

				for(int blockY = 0; blockY < 8; blockY++)
				{
					for(int blockX = 0; blockX < 8; blockX++)
					{
						int gIndex = blockX + blockY * 8;

						DCT_r[gIndex] = round(DCT_r[gIndex] / tempLumaTable[gIndex]);

						DCT_g[gIndex] = round(DCT_g[gIndex] / tempChromaTable[gIndex]);
						DCT_b[gIndex] = round(DCT_b[gIndex] / tempChromaTable[gIndex]);
					}
				}

				// DCT are now calculated

				// Encode DC

				DCR = CreateBitstream(bitBuf, bitCnt, DCR, DCT_r, Thesis::YDC_HT, Thesis::YAC_HT);
				DCG = CreateBitstream(bitBuf, bitCnt, DCG, DCT_g, Thesis::UVDC_HT, Thesis::UVAC_HT);
				DCB = CreateBitstream(bitBuf, bitCnt, DCB, DCT_b, Thesis::UVDC_HT, Thesis::UVAC_HT);
			}
		}
	}

	return totalBitsRequired;
}

int CreateBitstream(int& bitBuf,
					int& bitCnt,
					int prevDC,
					float DCT_r[64],
					const unsigned short DC_HT[256][2],
					const unsigned short AC_HT[256][2])
{
	int DU[64];
	for(int i = 0; i < 64; i++)
	{
		DU[Thesis::ZigZag[i]] = DCT_r[i];
	}
	int diff = DU[0] - prevDC;
	if(diff == 0)
	{
		Thesis::writebits(&bitBuf, &bitCnt, DC_HT[0]);
	}
	else
	{
		unsigned short bits[2];
		Thesis::calcbits(diff, bits);
		Thesis::writebits(&bitBuf, &bitCnt, DC_HT[bits[1]]);
		Thesis::writebits(&bitBuf, &bitCnt, bits);
	}

	// Encode AC
	int end0Pos						  = 63;
	const unsigned short m16zeroes[2] = {AC_HT[0xF0][0], AC_HT[0xF0][1]};
	const unsigned short EOB[2]		  = {AC_HT[0x00][0], AC_HT[0x00][1]};

	for(; (end0Pos > 0) && (DCT_r[end0Pos] == 0); --end0Pos)
		;
	if(end0Pos == 0)
	{
		Thesis::writebits(&bitBuf, &bitCnt, EOB);
		return DU[0];
	}
	else
	{
		// end0pos = first element in reverse order != 0
		for(int i = 1; i <= end0Pos; i++)
		{
			int startpos = i;
			int nrzeroes;
			unsigned short bits[2];
			for(; DCT_r[i] == 0 && i <= end0Pos; i++)
				;
			nrzeroes = i - startpos;
			if(nrzeroes >= 16)
			{
				// Here we do zrl
				int lng = nrzeroes >> 4;
				for(int nrmarker = 1; nrmarker <= lng; nrmarker++)
				{
					Thesis::writebits(&bitBuf, &bitCnt, m16zeroes);
				}
				nrzeroes &= 15;
			}
			Thesis::calcbits(DCT_r[i], bits);
			Thesis::writebits(&bitBuf, &bitCnt, AC_HT[(nrzeroes << 4) + bits[1]]);
			Thesis::writebits(&bitBuf, &bitCnt, bits);
		}
	}
	if(end0Pos != 63)
	{
		Thesis::writebits(&bitBuf, &bitCnt, EOB);
	}
	return DU[0];
}

void SetupTables()
{
	for(int y = 0; y < 8; y++)
	{
		for(int x = 0; x < 8; x++)
		{

			if(0 == y)
				DCT_MATRIX[y * 8 + x] = 1.0f / sqrtf(8.0f);
			else
				DCT_MATRIX[y * 8 + x] =
					float(sqrtf(2.0f / 8.0f) *
						  cosf(((2.0f * x + 1.0f) * DirectX::XM_PI * y) / (2.0 * 8.0)));
		}
	}

	for(int y = 0; y < 8; y++)
	{
		for(int x = 0; x < 8; x++)
		{
			DCT_MATRIX_TRANSPOSE[y * 8 + x] = DCT_MATRIX[x * 8 + y];
		}
	}
}

unsigned int
CalculateQuality(int currentX, int currentY, int gazeX, int gazeY, GAZE_Setting* setting)
{

	float quality = 0.0f;
	if(setting->function == FUNCTION_GAZE)
	{
		int d = sqrt(pow(currentX - gazeX, 2) + pow(currentY - gazeY, 2));

		float d3 = 2000.0f * setting->radius;

		quality = ((1.0f - ((float(d / (d3))))) * 85.0f);
	}
	else
	{

		float screenDist = 2400.0f * setting->radius;
		float qualityFactor;

		int dist = sqrt(pow(currentX - gazeX, 2) + pow(currentY - gazeY, 2));

		qualityFactor = (1.0f - (dist / screenDist));

		if(qualityFactor >= 0.9f && qualityFactor <= 1.0f)
		{
			quality = 85;
		}
		else if(qualityFactor >= 0.8f && qualityFactor < 0.9f)
		{
			float num = (0.9f - qualityFactor) / (0.9f - 0.8f);

			quality = 85.0f + num * (20.0f - 85.0f);
		}
		else
		{

			quality = 20;
		}
	}
	return quality * setting->innerQuality;
}

void SetupQuantTables(unsigned char _pLumTable[64],
					  unsigned char _pChromaTable[64],
					  unsigned int quality)
{
	quality = quality < 1 ? 1 : quality > 100 ? 100 : quality;
	quality = quality < 50 ? 5000 / quality : 200 - quality * 2;

	for(int i = 0; i < 64; i++)
	{
		int luma	  = (_pLumTable[i] * quality + 50) / 100;
		luma		  = (luma < 1 ? 1 : luma > 255 ? 255 : luma);
		_pLumTable[i] = luma;

		int chroma		 = (_pChromaTable[i] * quality + 50) / 100;
		chroma			 = (chroma < 1 ? 1 : chroma > 255 ? 255 : chroma);
		_pChromaTable[i] = chroma;
	}
}

void Thesis::writebits(int* bitBufP, int* bitCntP, const unsigned short* bs)
{
	int bitBuf = *bitBufP, bitCnt = *bitCntP;
	bitCnt += bs[1];
	bitBuf |= bs[0] << (24 - bitCnt);
	while(bitCnt >= 8)
	{
		unsigned char c = (bitBuf >> 16) & 255;
		std::bitset<8> x(c);
		totalBitsRequired += 8LL;
		//std::cout << "(" << +c << ") " << x << " ";
		/**filebufferPtr = c;
		filebufferPtr++;*/
		if(c == 255)
		{
			/*std::cout << 0;
			*filebufferPtr = 0;
			filebufferPtr++;*/
		}
		bitBuf <<= 8;
		bitCnt -= 8;
	}

	*bitBufP = bitBuf;
	*bitCntP = bitCnt;
}

void Thesis::calcbits(int val, unsigned short bits[2])
{
	int tmp1 = val < 0 ? -val : val;
	val		 = val < 0 ? val - 1 : val;
	bits[1]  = 1;
	while(tmp1 >>= 1)
	{
		++bits[1];
	}
	bits[0] = val & ((1 << bits[1]) - 1);
}
