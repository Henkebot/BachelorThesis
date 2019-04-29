// BachelorThesis.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"


float DCT_MATRIX[64];
float DCT_MATRIX_TRANSPOSE[64];

unsigned char Quat[64] = {

	16, 11, 10, 16, 24,  40,  51,  61,  12, 12, 14, 19, 26,  58,  60,  55,
	14, 13, 16, 24, 40,  57,  69,  56,  14, 17, 22, 29, 51,  87,  80,  62,
	18, 22, 37, 56, 68,  109, 103, 77,  24, 35, 55, 64, 81,  104, 113, 92,
	49, 64, 78, 87, 103, 121, 120, 101, 72, 92, 95, 98, 112, 100, 103, 99};

unsigned char ZigZag[] = {0,  1,  5,  6,  14, 15, 27, 28, 2,  4,  7,  13, 16, 26, 29, 42,
						  3,  8,  12, 17, 25, 30, 41, 43, 9,  11, 18, 24, 31, 40, 44, 53,
						  10, 19, 23, 32, 39, 45, 52, 54, 20, 22, 33, 38, 46, 51, 55, 60,
						  21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48, 49, 57, 58, 62, 63};

// The structure is the [HuffmanCode:Bits required to store the value]
// They are stored as a lookup table for the size of the bit
static const unsigned short YDC_HT[256][2]  = {{0, 2},
											   {2, 3},
											   {3, 3},
											   {4, 3},
											   {5, 3},
											   {6, 3},
											   {14, 4},
											   {30, 5},
											   {62, 6},
											   {126, 7},
											   {254, 8},
											   {510, 9}};
static const unsigned short UVDC_HT[256][2] = {{0, 2},
											   {1, 2},
											   {2, 2},
											   {6, 3},
											   {14, 4},
											   {30, 5},
											   {62, 6},
											   {126, 7},
											   {254, 8},
											   {510, 9},
											   {1022, 10},
											   {2046, 11}};

static const unsigned short YAC_HT[256][2] = {
	{10, 4},	 {0, 2},	  {1, 2},	  {4, 3},		{11, 4},	 {26, 5},	 {120, 7},
	{248, 8},	{1014, 10},  {65410, 16}, {65411, 16}, {0, 0},		 {0, 0},	  {0, 0},
	{0, 0},		 {0, 0},	  {0, 0},	  {12, 4},		{27, 5},	 {121, 7},	{502, 9},
	{2038, 11},  {65412, 16}, {65413, 16}, {65414, 16}, {65415, 16}, {65416, 16}, {0, 0},
	{0, 0},		 {0, 0},	  {0, 0},	  {0, 0},		{0, 0},		 {28, 5},	 {249, 8},
	{1015, 10},  {4084, 12},  {65417, 16}, {65418, 16}, {65419, 16}, {65420, 16}, {65421, 16},
	{65422, 16}, {0, 0},	  {0, 0},	  {0, 0},		{0, 0},		 {0, 0},	  {0, 0},
	{58, 6},	 {503, 9},	{4085, 12},  {65423, 16}, {65424, 16}, {65425, 16}, {65426, 16},
	{65427, 16}, {65428, 16}, {65429, 16}, {0, 0},		{0, 0},		 {0, 0},	  {0, 0},
	{0, 0},		 {0, 0},	  {59, 6},	 {1016, 10},  {65430, 16}, {65431, 16}, {65432, 16},
	{65433, 16}, {65434, 16}, {65435, 16}, {65436, 16}, {65437, 16}, {0, 0},	  {0, 0},
	{0, 0},		 {0, 0},	  {0, 0},	  {0, 0},		{122, 7},	{2039, 11},  {65438, 16},
	{65439, 16}, {65440, 16}, {65441, 16}, {65442, 16}, {65443, 16}, {65444, 16}, {65445, 16},
	{0, 0},		 {0, 0},	  {0, 0},	  {0, 0},		{0, 0},		 {0, 0},	  {123, 7},
	{4086, 12},  {65446, 16}, {65447, 16}, {65448, 16}, {65449, 16}, {65450, 16}, {65451, 16},
	{65452, 16}, {65453, 16}, {0, 0},	  {0, 0},		{0, 0},		 {0, 0},	  {0, 0},
	{0, 0},		 {250, 8},	{4087, 12},  {65454, 16}, {65455, 16}, {65456, 16}, {65457, 16},
	{65458, 16}, {65459, 16}, {65460, 16}, {65461, 16}, {0, 0},		 {0, 0},	  {0, 0},
	{0, 0},		 {0, 0},	  {0, 0},	  {504, 9},	{32704, 15}, {65462, 16}, {65463, 16},
	{65464, 16}, {65465, 16}, {65466, 16}, {65467, 16}, {65468, 16}, {65469, 16}, {0, 0},
	{0, 0},		 {0, 0},	  {0, 0},	  {0, 0},		{0, 0},		 {505, 9},	{65470, 16},
	{65471, 16}, {65472, 16}, {65473, 16}, {65474, 16}, {65475, 16}, {65476, 16}, {65477, 16},
	{65478, 16}, {0, 0},	  {0, 0},	  {0, 0},		{0, 0},		 {0, 0},	  {0, 0},
	{506, 9},	{65479, 16}, {65480, 16}, {65481, 16}, {65482, 16}, {65483, 16}, {65484, 16},
	{65485, 16}, {65486, 16}, {65487, 16}, {0, 0},		{0, 0},		 {0, 0},	  {0, 0},
	{0, 0},		 {0, 0},	  {1017, 10},  {65488, 16}, {65489, 16}, {65490, 16}, {65491, 16},
	{65492, 16}, {65493, 16}, {65494, 16}, {65495, 16}, {65496, 16}, {0, 0},	  {0, 0},
	{0, 0},		 {0, 0},	  {0, 0},	  {0, 0},		{1018, 10},  {65497, 16}, {65498, 16},
	{65499, 16}, {65500, 16}, {65501, 16}, {65502, 16}, {65503, 16}, {65504, 16}, {65505, 16},
	{0, 0},		 {0, 0},	  {0, 0},	  {0, 0},		{0, 0},		 {0, 0},	  {2040, 11},
	{65506, 16}, {65507, 16}, {65508, 16}, {65509, 16}, {65510, 16}, {65511, 16}, {65512, 16},
	{65513, 16}, {65514, 16}, {0, 0},	  {0, 0},		{0, 0},		 {0, 0},	  {0, 0},
	{0, 0},		 {65515, 16}, {65516, 16}, {65517, 16}, {65518, 16}, {65519, 16}, {65520, 16},
	{65521, 16}, {65522, 16}, {65523, 16}, {65524, 16}, {0, 0},		 {0, 0},	  {0, 0},
	{0, 0},		 {0, 0},	  {2041, 11},  {65525, 16}, {65526, 16}, {65527, 16}, {65528, 16},
	{65529, 16}, {65530, 16}, {65531, 16}, {65532, 16}, {65533, 16}, {65534, 16}, {0, 0},
	{0, 0},		 {0, 0},	  {0, 0},	  {0, 0}};
static const unsigned short UVAC_HT[256][2] = {
	{0, 2},		 {1, 2},	  {4, 3},	  {10, 4},		{24, 5},	 {25, 5},	 {56, 6},
	{120, 7},	{500, 9},	{1014, 10},  {4084, 12},  {0, 0},		 {0, 0},	  {0, 0},
	{0, 0},		 {0, 0},	  {0, 0},	  {11, 4},		{57, 6},	 {246, 8},	{501, 9},
	{2038, 11},  {4085, 12},  {65416, 16}, {65417, 16}, {65418, 16}, {65419, 16}, {0, 0},
	{0, 0},		 {0, 0},	  {0, 0},	  {0, 0},		{0, 0},		 {26, 5},	 {247, 8},
	{1015, 10},  {4086, 12},  {32706, 15}, {65420, 16}, {65421, 16}, {65422, 16}, {65423, 16},
	{65424, 16}, {0, 0},	  {0, 0},	  {0, 0},		{0, 0},		 {0, 0},	  {0, 0},
	{27, 5},	 {248, 8},	{1016, 10},  {4087, 12},  {65425, 16}, {65426, 16}, {65427, 16},
	{65428, 16}, {65429, 16}, {65430, 16}, {0, 0},		{0, 0},		 {0, 0},	  {0, 0},
	{0, 0},		 {0, 0},	  {58, 6},	 {502, 9},	{65431, 16}, {65432, 16}, {65433, 16},
	{65434, 16}, {65435, 16}, {65436, 16}, {65437, 16}, {65438, 16}, {0, 0},	  {0, 0},
	{0, 0},		 {0, 0},	  {0, 0},	  {0, 0},		{59, 6},	 {1017, 10},  {65439, 16},
	{65440, 16}, {65441, 16}, {65442, 16}, {65443, 16}, {65444, 16}, {65445, 16}, {65446, 16},
	{0, 0},		 {0, 0},	  {0, 0},	  {0, 0},		{0, 0},		 {0, 0},	  {121, 7},
	{2039, 11},  {65447, 16}, {65448, 16}, {65449, 16}, {65450, 16}, {65451, 16}, {65452, 16},
	{65453, 16}, {65454, 16}, {0, 0},	  {0, 0},		{0, 0},		 {0, 0},	  {0, 0},
	{0, 0},		 {122, 7},	{2040, 11},  {65455, 16}, {65456, 16}, {65457, 16}, {65458, 16},
	{65459, 16}, {65460, 16}, {65461, 16}, {65462, 16}, {0, 0},		 {0, 0},	  {0, 0},
	{0, 0},		 {0, 0},	  {0, 0},	  {249, 8},	{65463, 16}, {65464, 16}, {65465, 16},
	{65466, 16}, {65467, 16}, {65468, 16}, {65469, 16}, {65470, 16}, {65471, 16}, {0, 0},
	{0, 0},		 {0, 0},	  {0, 0},	  {0, 0},		{0, 0},		 {503, 9},	{65472, 16},
	{65473, 16}, {65474, 16}, {65475, 16}, {65476, 16}, {65477, 16}, {65478, 16}, {65479, 16},
	{65480, 16}, {0, 0},	  {0, 0},	  {0, 0},		{0, 0},		 {0, 0},	  {0, 0},
	{504, 9},	{65481, 16}, {65482, 16}, {65483, 16}, {65484, 16}, {65485, 16}, {65486, 16},
	{65487, 16}, {65488, 16}, {65489, 16}, {0, 0},		{0, 0},		 {0, 0},	  {0, 0},
	{0, 0},		 {0, 0},	  {505, 9},	{65490, 16}, {65491, 16}, {65492, 16}, {65493, 16},
	{65494, 16}, {65495, 16}, {65496, 16}, {65497, 16}, {65498, 16}, {0, 0},	  {0, 0},
	{0, 0},		 {0, 0},	  {0, 0},	  {0, 0},		{506, 9},	{65499, 16}, {65500, 16},
	{65501, 16}, {65502, 16}, {65503, 16}, {65504, 16}, {65505, 16}, {65506, 16}, {65507, 16},
	{0, 0},		 {0, 0},	  {0, 0},	  {0, 0},		{0, 0},		 {0, 0},	  {2041, 11},
	{65508, 16}, {65509, 16}, {65510, 16}, {65511, 16}, {65512, 16}, {65513, 16}, {65514, 16},
	{65515, 16}, {65516, 16}, {0, 0},	  {0, 0},		{0, 0},		 {0, 0},	  {0, 0},
	{0, 0},		 {16352, 14}, {65517, 16}, {65518, 16}, {65519, 16}, {65520, 16}, {65521, 16},
	{65522, 16}, {65523, 16}, {65524, 16}, {65525, 16}, {0, 0},		 {0, 0},	  {0, 0},
	{0, 0},		 {0, 0},	  {1018, 10},  {32707, 15}, {65526, 16}, {65527, 16}, {65528, 16},
	{65529, 16}, {65530, 16}, {65531, 16}, {65532, 16}, {65533, 16}, {65534, 16}, {0, 0},
	{0, 0},		 {0, 0},	  {0, 0},	  {0, 0}};

unsigned char fileBuffer[512] = {};
unsigned char* filebufferPtr  = fileBuffer;
void writebits(int* bitBufP, int* bitCntP, const unsigned short* bs)
{
	int bitBuf = *bitBufP, bitCnt = *bitCntP;
	bitCnt += bs[1];
	bitBuf |= bs[0] << (24 - bitCnt);
	while(bitCnt >= 8)
	{
		unsigned char c = (bitBuf >> 16) & 255;
		std::bitset<8> x(c);
		std::cout << "(" << +c << ") " << x << " ";
		*filebufferPtr = c;
		filebufferPtr++;
		if(c == 255)
		{
			std::cout << 0;
			*filebufferPtr = 0;
			filebufferPtr++;
		}
		bitBuf <<= 8;
		bitCnt -= 8;
	}

	*bitBufP = bitBuf;
	*bitCntP = bitCnt;
}

void calcbits(int val, unsigned short bits[2])
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

int main()
{
	
	//for(int y = 0; y < 8; y++)
	//{
	//	for(int x = 0; x < 8; x++)
	//	{

	//		if(0 == y)
	//			DCT_MATRIX[y * 8 + x] = 1.0f / sqrtf(8.0f);
	//		else
	//			DCT_MATRIX[y * 8 + x] = float(
	//				sqrtf(2.0f / 8.0f) * cosf(((2 * x + 1) * DirectX::XM_PI * y) / (2.0 * 8.0)));
	//	}
	//}

	//for(int y = 0; y < 8; y++)
	//{
	//	for(int x = 0; x < 8; x++)
	//	{
	//		DCT_MATRIX_TRANSPOSE[y * 8 + x] = DCT_MATRIX[x * 8 + y];
	//	}
	//}

	//constexpr int IMAGE_WIDTH  = 8;
	//constexpr int IMAGE_HEIGHT = 8;
	//constexpr int IMAGE_SIZE   = IMAGE_WIDTH * IMAGE_HEIGHT;

	//BYTE colorBlock[IMAGE_SIZE] = {
	//	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	//	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	//	255, 255, 255, 255, 255, 255, 255, 1,   255, 255, 255, 255, 255, 255, 255, 255,
	//	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	//};

	////NOTE(Henrik): Values taken from wikipedia
	///*BYTE colorBlock[IMAGE_SIZE] = {52, 55,  61,  66,  70, 61, 64,  73,  63,  59,  55,  90, 109,
	//							   85, 69,  72,  62,  59, 68, 113, 144, 104, 66,  73,  63, 58,
	//							   71, 122, 154, 106, 70, 69, 67,  61,  68,  104, 126, 88, 68,
	//							   70, 79,  65,  60,  70, 77, 68,  58,  75,  85,  71,  64, 59,
	//							   55, 61,  65,  83,  87, 79, 69,  68,  65,  76,  78,  94};*/

	//for(unsigned int y = 0; y < IMAGE_HEIGHT; y++)
	//{
	//	for(unsigned int x = 0; x < IMAGE_WIDTH; x++)
	//	{
	//		printf("%i ", colorBlock[x + y * IMAGE_WIDTH]);
	//	}
	//	puts("\n");
	//}

	//puts("\n");

	//double DCT_MatrixTemp[64];

	//for(unsigned int y = 0; y < IMAGE_HEIGHT; y++)
	//{
	//	for(unsigned int x = 0; x < IMAGE_WIDTH; x++)
	//	{
	//		unsigned int imgIdx = x + y * IMAGE_WIDTH;

	//		// NOTE(Henrik): For now is the imgIdx the same dimensions as the DCT table.
	//		DCT_MatrixTemp[imgIdx] = 0.0f;

	//		for(int k = 0; k < 8; k++)
	//		{
	//			DCT_MatrixTemp[imgIdx] += DCT_MATRIX[y * 8 + k] * (colorBlock[k * 8 + x] - 128);
	//		}
	//	}
	//}

	//double DCT_Coefficients[64];

	//for(unsigned int y = 0; y < IMAGE_HEIGHT; y++)
	//{
	//	for(unsigned int x = 0; x < IMAGE_WIDTH; x++)
	//	{
	//		unsigned int imgIdx = x + y * IMAGE_WIDTH;

	//		// NOTE(Henrik): For now is the imgIdx the same dimensions as the DCT table.
	//		DCT_Coefficients[imgIdx] = 0.0f;

	//		for(int k = 0; k < 8; k++)
	//		{
	//			DCT_Coefficients[imgIdx] +=
	//				DCT_MatrixTemp[y * 8 + k] * DCT_MATRIX_TRANSPOSE[k * 8 + x];
	//		}

	//		// Rounding

	//		DCT_Coefficients[imgIdx] = roundf(DCT_Coefficients[imgIdx] / Quat[imgIdx]);
	//	}
	//}

	//puts("After DCT\n");

	//for(unsigned int y = 0; y < IMAGE_HEIGHT; y++)
	//{
	//	for(unsigned int x = 0; x < IMAGE_WIDTH; x++)
	//	{
	//		printf("%.0f ", DCT_Coefficients[x + y * IMAGE_WIDTH]);
	//	}
	//	puts("\n");
	//}

	//puts("ZigZag ordering\n");

	//int DU[64];
	//for(int i = 0; i < 64; i++)
	//{
	//	DU[ZigZag[i]] = DCT_Coefficients[i];
	//}

	//for(unsigned int i = 0; i < 64; i++)
	//{
	//	if(i % 8 == 0)
	//		puts("\n");
	//	printf("%i ", DU[i]);
	//}
	//puts("\n");

	///*int testCase[] = {-7, -2, 3, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	//				  0,  0,  0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
	//				  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0};*/
	//// Encode DC
	//int bitBuf = 0, bitCnt = 0;
	//{
	//	int diff = DCT_Coefficients[0];
	//	//DU[0] - 0;
	//	unsigned short bits[2];
	//	calcbits(diff, bits);

	//	//Encode DC

	//	// Bits[0] is the actual value
	//	// Bits[1] is the amount of bits required to represent bits[0]
	//	// First we store the huffman representation of how many bits requried to store as a huffman code then the value
	//	writebits(&bitBuf, &bitCnt, UVDC_HT[bits[1]]);
	//	writebits(&bitBuf, &bitCnt, bits);
	//}

	//// We have the value and we have the nr of bits required to represent the value
	//// What we want to

	//// Encode ACs
	//int end0pos = 63;

	//const unsigned short m16zeroes[2] = {UVAC_HT[0xF0][0], UVAC_HT[0xF0][1]};
	//const unsigned short EOB[2]		  = {UVAC_HT[0x00][0], UVAC_HT[0x00][1]};

	//for(; (end0pos > 0) && (DCT_Coefficients[end0pos] == 0); --end0pos)
	//	;

	//if(end0pos == 0)
	//{
	//	writebits(&bitBuf, &bitCnt, EOB);
	//}
	//else
	//{
	//	// end0pos = first element in reverse order != 0
	//	for(int i = 1; i <= end0pos; i++)
	//	{
	//		int startpos = i;
	//		int nrzeroes;
	//		unsigned short bits[2];
	//		for(; DCT_Coefficients[i] == 0 && i <= end0pos; i++)
	//			;
	//		nrzeroes = i - startpos;
	//		if(nrzeroes >= 16)
	//		{
	//			// Here we do zrl
	//			int lng = nrzeroes >> 4;
	//			for(int nrmarker = 1; nrmarker <= lng; nrmarker++)
	//			{
	//				writebits(&bitBuf, &bitCnt, m16zeroes);
	//			}
	//			nrzeroes &= 15;
	//		}
	//		calcbits(DCT_Coefficients[i], bits);
	//		writebits(&bitBuf, &bitCnt, UVAC_HT[(nrzeroes << 4) + bits[1]]);
	//		writebits(&bitBuf, &bitCnt, bits);
	//	}
	//}

	//static const unsigned short fillbits[2] = {0x7F, 7};
	//writebits(&bitBuf, &bitCnt, fillbits);
	//int size = 0;
	//for(; (size < 512) && (fileBuffer[size] != 0); size++)
	//	;
	//std::cout << "\nCompression rate of " << 64 / size << std::endl;

	//// To get the values back, we need to traverse the huffman table
	///*
	//	What to expect:
	//	We get a huffman code that tells us how many bits that are required to represent the value
	//
	//*/

	//unsigned char bit = fileBuffer[0];
	//std::bitset<8> bb(bit);
	//for(auto& ht : UVDC_HT)
	//{
	//	int bitCtr = 7;

	//	int bitSize = ht[1] - 1;
	//	std::bitset<8> current(ht[0]);
	//	while((bitSize != -1) && (current.test(bitSize) == bb.test(bitCtr)))
	//	{
	//		bitCtr--;
	//		bitSize--;
	//	}

	//	if(bitSize < 0)
	//	{
	//		/*
	//			Now do we have found the correct huffman we now know how many bits that the next value is containing

	//		*/
	//		// Increment the bit counter
	//		bitCtr++;
	//		// Reset the amount of bits required
	//		bitSize = ht[1];
	//		// If there are bits remaining in the current byte we need to extract them
	//		if(bitCtr < bitSize)
	//		{
	//			unsigned char restValue = bit & ((1 << bitCtr) - 1);

	//			int bitsLeft = bitSize - bitCtr;
	//			// Increment the bit
	//			bit = fileBuffer[1];
	//			bb  = bit;

	//			unsigned char restValue2 = bit >> (8 - bitsLeft);
	//			unsigned char finalValue = (restValue << (8 - bitsLeft)) + restValue2;

	//			std::cout << "DC is " << +finalValue << std::endl;
	//		}
	//	}
	//}
	//for(unsigned int y = 0; y < IMAGE_HEIGHT; y++)
	//{
	//	for(unsigned int x = 0; x < IMAGE_WIDTH; x++)
	//	{
	//		DCT_Coefficients[x + y * IMAGE_WIDTH] *= Quat[x + y * IMAGE_WIDTH];
	//	}
	//}

	//for(unsigned int y = 0; y < IMAGE_HEIGHT; y++)
	//{
	//	for(unsigned int x = 0; x < IMAGE_WIDTH; x++)
	//	{
	//		unsigned int imgIdx = x + y * IMAGE_WIDTH;

	//		// NOTE(Henrik): For now is the imgIdx the same dimensions as the DCT table.
	//		DCT_MatrixTemp[imgIdx] = 0.0f;

	//		for(int k = 0; k < 8; k++)
	//		{
	//			DCT_MatrixTemp[imgIdx] +=
	//				DCT_MATRIX_TRANSPOSE[y * 8 + k] * DCT_Coefficients[k * 8 + x];
	//		}
	//	}
	//}

	//for(unsigned int y = 0; y < IMAGE_HEIGHT; y++)
	//{
	//	for(unsigned int x = 0; x < IMAGE_WIDTH; x++)
	//	{
	//		unsigned int imgIdx = x + y * IMAGE_WIDTH;

	//		// NOTE(Henrik): For now is the imgIdx the same dimensions as the DCT table.
	//		colorBlock[imgIdx] = 255;

	//		for(int k = 0; k < 8; k++)
	//		{
	//			colorBlock[imgIdx] += DCT_MatrixTemp[y * 8 + k] * DCT_MATRIX[k * 8 + x];
	//		}
	//		colorBlock[imgIdx] += 128;
	//	}
	//}

	//puts("Inverse DCT\n");
	//for(unsigned int y = 0; y < IMAGE_HEIGHT; y++)
	//{
	//	for(unsigned int x = 0; x < IMAGE_WIDTH; x++)
	//	{
	//		printf("%i ", colorBlock[x + y * IMAGE_WIDTH]);
	//	}
	//	puts("\n");
	//}


	return 0;
}
