//Muhammad Iqbal
//1718170

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//__global__ --> GPU function which can be launched by many blocks and threads
//__device__ --> GPU function or variables
//__host__ --> CPU function or variables

__device__ char* CudaCrypt(char* rawPassword){

	char * newPassword = (char *) malloc(sizeof(char) * 11);

	newPassword[0] = rawPassword[0] + 2;
	newPassword[1] = rawPassword[0] - 2;
	newPassword[2] = rawPassword[0] + 1;
	newPassword[3] = rawPassword[1] + 3;
	newPassword[4] = rawPassword[1] - 3;
	newPassword[5] = rawPassword[1] - 1;
	newPassword[6] = rawPassword[2] + 2;
	newPassword[7] = rawPassword[2] - 2;
	newPassword[8] = rawPassword[3] + 4;
	newPassword[9] = rawPassword[3] - 4;
	newPassword[10] = '\0';

	for(int i =0; i<10; i++){
		if(i >= 0 && i < 6){ //checking all lower case letter limits
			if(newPassword[i] > 122){
				newPassword[i] = (newPassword[i] - 122) + 97;
			}else if(newPassword[i] < 97){
				newPassword[i] = (97 - newPassword[i]) + 97;
			}
		}else{ //checking number section
			if(newPassword[i] > 57){
				newPassword[i] = (newPassword[i] - 57) + 48;
			}else if(newPassword[i] < 48){
				newPassword[i] = (48 - newPassword[i]) + 48;
			}
		}
	}
	return newPassword;
}

/**
*	Checking to see if first string matches with other length char strings 
*/
__device__
int MatchingEcryption(char* first, char* second, int length) {
	int result = 1;
	for (int i = 0; i < length; i++) {
		if (first[i] != second[i]) {
			result = 0;
			break;
		}
	}
	return result;
}

/**
*	The pass to be Decrypted via a coda thread
*/
__global__
void crack(char* alphabet, char* numbers, char* encryptedPsw, char* passOutput){
	int uid = blockDim.x * blockIdx.x + threadIdx.x;

	if(*passOutput != NULL) {
		return;
	}

	char genRawPass[4];

	genRawPass[0] = alphabet[blockIdx.x];
	genRawPass[1] = alphabet[blockIdx.y];

	genRawPass[2] = numbers[threadIdx.x];
	genRawPass[3] = numbers[threadIdx.y];

	//firstLetter - 'a' - 'z' (26 characters)
	//secondLetter - 'a' - 'z' (26 characters)
	//firstNum - '0' - '9' (10 characters)
	//secondNum - '0' - '9' (10 characters)

//Idx --> gives current index of the block or thread
	
	char* potentialEncry;
	potentialEncry = CudaCrypt(genRawPass);

	if (MatchingEcryption(encryptedPsw, potentialEncry, 11) > 0 )
	{
		
		for (int i = 0; i < 4; i++) {
		passOutput[i] = genRawPass[i];
		}
	}

}

int main(int argc, char ** argv){

	char * encryptedPsw = "ccbddb7362";
	if (argc > 1) {
		encryptedPsw = argv[1];
	}
	printf("Password Given: '%s'\n", encryptedPsw);

	char cpuAlphabet[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
	char cpuNumbers[10] = {'0','1','2','3','4','5','6','7','8','9'};
	int encryptedPswSize = sizeof(char) * 11;

	char * gpuAlphabet;
	cudaMalloc( (void**) &gpuAlphabet, sizeof(char) * 26); 
	cudaMemcpy(gpuAlphabet, cpuAlphabet, sizeof(char) * 26, cudaMemcpyHostToDevice);



	char * gpuNumbers;
	cudaMalloc( (void**) &gpuNumbers, sizeof(char) * 10); 
	cudaMemcpy(gpuNumbers, cpuNumbers, sizeof(char) * 10, cudaMemcpyHostToDevice);


	char * gpuOutputPass;
	cudaMalloc( (void**) &gpuOutputPass, encryptedPswSize);


	char * gpuencryptedPsw;
	cudaMalloc( (void**) &gpuencryptedPsw, encryptedPswSize);
	cudaMemcpy( gpuencryptedPsw, encryptedPsw, encryptedPswSize, cudaMemcpyHostToDevice);


	crack<<< dim3(26, 26, 1), dim3(10, 10, 1) >>>( gpuAlphabet, gpuNumbers, gpuencryptedPsw, gpuOutputPass);
	cudaDeviceSynchronize();


	char* cpuPassOutput = (char*)malloc( sizeof(char) * 4);
	cudaMemcpy(cpuPassOutput, gpuOutputPass, encryptedPswSize, cudaMemcpyDeviceToHost);
	if (cpuPassOutput != NULL && cpuPassOutput[0] != 0) {
		printf("Given Encrypted Password: '%s'\n", encryptedPsw);
		printf("Found Decrypted Password: '%s'\n", cpuPassOutput);
	
	} else {
		printf("Password could not be determined.\n");
	}

	cudaFree(gpuAlphabet);
	cudaFree(gpuNumbers);
	cudaFree(gpuencryptedPsw);
	cudaFree(gpuOutputPass);
	free(cpuPassOutput);
}









