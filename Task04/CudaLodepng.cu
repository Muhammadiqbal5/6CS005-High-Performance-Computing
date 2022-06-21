//Muhammad Iqbal
//1718170

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lodepng.h"

//each pixel has 4 values:
//Red (R)
//Green (G)
//Blue (B)
//Transparency (T)

//negative filter (reversing the RGB values)
//0-255   max-val    255-40

struct RGBT {
	int r;
	int g;
	int b;
	int t;
};

const int MAX_VALUE = 255;
const int ERROR_EXIT_VALUE = -1;


void printRGBTArray(unsigned char* rgbtArray, int height, int width)
{
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width*4; col += 4) {
			printf("Row: '%d' Col: '%d' R:%d, G:%d, B:%d, A:%d\n", row, col / 4, rgbtArray[row*width*4+col], rgbtArray[row*width*4+col+1], rgbtArray[row*width*4+col+2], rgbtArray[row*width*4+col+3]);
		}
	}
	printf("\n");
}

__device__
struct RGBT GetAverage(struct RGBT* vals, int valsLength, int totalValidValues)
{
	double totalR = 0.0, totalG = 0.0, totalB = 0.0;
	for (int i = 0; i < valsLength; i++) {
		totalR += vals[i].r;
		totalG += vals[i].g;
		totalB += vals[i].b;
	}

	struct RGBT getAverageValues;
	getAverageValues.r = totalR / totalValidValues;
	getAverageValues.g = totalG / totalValidValues;
	getAverageValues.b = totalB / totalValidValues;
	getAverageValues.t = MAX_VALUE;
	return getAverageValues;
}

__device__
struct RGBT GetRGBTValuesAtPixel(unsigned char* inputImage, int pixel) {
	struct RGBT values;
	values.r = inputImage[pixel];
	values.g = inputImage[pixel+1];
	values.b = inputImage[pixel+2];
	values.t = inputImage[pixel+3];
	return values;
}


__global__ 
void NegativeFilter(unsigned char* actualVals, unsigned char* blurredChars, int width, int height) {
	int threadIndex = blockDim.x * blockIdx.x + threadIdx.x;

	int pixelY = threadIndex % width;
	int pixelX = threadIndex / width;

	struct RGBT* thisValues = (struct RGBT*) malloc( sizeof(struct RGBT) * 9);
	int arrayStartIndex = threadIndex * 4;

	int validValuesCount = 0;
	thisValues[4] = GetRGBTValuesAtPixel(actualVals, arrayStartIndex);
	validValuesCount++;

	bool atLeftWall = pixelY == 0;
	bool atTopWall = pixelX == 0;
	bool atRightWall = pixelY > 0 && pixelY % (width - 1) * 4 == 0;
	bool atBtmWall = pixelX >= height - 1;

	int colArrayIndex = pixelY * 4;

	if (!atLeftWall) {
		int mlIndex = (pixelX * width * 4) + (colArrayIndex - (4 * 1));
		thisValues[3] = GetRGBTValuesAtPixel(actualVals, mlIndex);
		validValuesCount++;
	
	
	if (!atTopWall) {
		int tlIndex = ((pixelX + 1) * width * 4) + (colArrayIndex - (4 * 1));
		thisValues[6] = GetRGBTValuesAtPixel(actualVals, tlIndex);
		validValuesCount++;
	}

	if (!atBtmWall) {
		int blIndex = ((pixelX + 1) * width * 4) + (colArrayIndex - (4 * 1));
		thisValues[6] = GetRGBTValuesAtPixel(actualVals, blIndex);
		validValuesCount++;
	}
}




	if (!atRightWall) {
		int mrIndex = (pixelX * width * 4) + (colArrayIndex + (4 * 1));
		thisValues[5] = GetRGBTValuesAtPixel(actualVals, mrIndex);
		validValuesCount++;

	if (!atTopWall) {
		int trIndex = ((pixelX - 1 ) * width * 4) + (colArrayIndex + (4 * 1));
		thisValues[2] = GetRGBTValuesAtPixel(actualVals, trIndex);
		validValuesCount++;
	}

	if (!atBtmWall) {
		int brIndex = ((pixelX + 1 ) * width * 4) + (colArrayIndex + (4 * 1));
		thisValues[8] = GetRGBTValuesAtPixel(actualVals, brIndex);
		validValuesCount++;
	}
}



	if (!atTopWall) {
		int tmIndex = ((pixelX - 1 ) * width * 4) + colArrayIndex;
		thisValues[1] = GetRGBTValuesAtPixel(actualVals, tmIndex);
		validValuesCount++;
	}

	if (!atBtmWall) {
		int bmIndex = ((pixelX + 1 ) * width * 4) + colArrayIndex;
		thisValues[7] = GetRGBTValuesAtPixel(actualVals, bmIndex);
		validValuesCount++;
	}

	struct RGBT blurredVals = GetAverage(thisValues, 9, validValuesCount);
	blurredChars[arrayStartIndex] = blurredVals.r;
	blurredChars[arrayStartIndex + 1] = blurredVals.g;
	blurredChars[arrayStartIndex + 2] = blurredVals.b;
	blurredChars[arrayStartIndex + 3] = blurredVals.t;
	free(thisValues);
}

/**
printf("uid = %d\n", pixel);

r = inputImage[pixel];
g = inputImage[pixel+1];
b = inputImage[pixel+2];
t = inputImage[pixel+3];

outputImage[pixel] = 255-r;
outputImage[pixel+1] = 255-g;
outputImage[pixel+2] = 255-b;
outputImage[pixel+3] = t;

}**/

int main(int argc, char ** argv){

  unsigned int errorDecode; //variable will hold whether there was an issue with loading in the png file
  unsigned char* cpuImage; /** = (unsigned char*) malloc( sizeof(unsigned char) * width * height * 4 errorDecode = lodepng_decode32_file(&cpuImage, &width, &height, filename); **/ //this variable will hold all of our image data
  unsigned int width, height; //holds the width and height of image
  
  char* filename = "1.png";
  if (argc>1)
  filename = argv[1];

  char* newFilename = "blurredimg.png";
  if (argc>2)
  newFilename = argv[2];

  errorDecode = lodepng_decode32_file(&cpuImage, &width, &height, filename); // (where to store the image data, width, height, which file?)
  if(errorDecode){
    printf("error %u: %s\n", errorDecode, lodepng_error_text(errorDecode));
    exit(ERROR_EXIT_VALUE);
  }
  
  int arraySize = width*height*4;
  if (width <=0 || height <= 0) {
  printf("This is unable to decode the image. Please double check the png file and try again\n");
  exit(ERROR_EXIT_VALUE);
}
  //int memorySize = arraySize * sizeof(unsigned char);
  
  //unsigned char cpuOutImage[arraySize];
  
  unsigned char* gpuInput;
  cudaMalloc( (void**) &gpuInput, sizeof(unsigned char) * arraySize);
  cudaMemcpy(gpuInput, cpuImage, sizeof(unsigned char) * arraySize, cudaMemcpyHostToDevice);

  unsigned char* gpuOutput;
  cudaMalloc( (void**) &gpuOutput, sizeof(unsigned char) * arraySize);
  
  //cudaMemcpy(gpuInput, cpuImage, memorySize, cudaMemcpyHostToDevice);
  
  NegativeFilter<<< dim3(width,1,1), dim3(height,1,1) >>>(gpuInput, gpuOutput, width, height);
  cudaDeviceSynchronize();
  unsigned char* cpuOutImage = (unsigned char*) malloc(sizeof(unsigned char) * arraySize);
  cudaMemcpy(cpuOutImage, gpuOutput, sizeof(unsigned char) * arraySize, cudaMemcpyDeviceToHost);
  
  unsigned int errorEncode = lodepng_encode32_file(newFilename, cpuOutImage, width, height);
  if(errorEncode) {
  printf("error %u: %s\n", errorEncode, lodepng_error_text(errorEncode));
  exit(ERROR_EXIT_VALUE);
  }

  //free(image);
  free(cpuImage);
  free(cpuOutImage);
  cudaFree(gpuInput);
  cudaFree(gpuOutput);

}


