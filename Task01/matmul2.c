// Muhammad Iqbal
// 1718170

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#define BUFFER_SIZE 256

struct ThreadArgs {
    int threadNum;
    int iterateAmount;
    struct Matrix* finalResultInfo;
    double* valueA;
    double* valueB;
};

int readFile(char* fileName) {
    FILE * filePtr;
    filePtr = fopen(fileName, "r");
    if (filePtr == NULL) {
        printf("Unable to load file '%s'\n", fileName);
        return 0;
    }

    int matrixCount = 0;
    char readBuffer[BUFFER_SIZE];
    while ( fgets(readBuffer, BUFFER_SIZE, filePtr) != NULL ) {
        if ( emptyChar(readBuffer, BUFFER_SIZE) > 0 ) {
            matrixCount++;
        }
    }
    matrixCount++;

    fclose(filePtr);

    return matrixCount;
}

struct sizeOfMatrix {
    int x;
    int y;
};

struct Matrix {
    struct sizeOfMatrix size;
    double* values;
};

/*
* The memory management
*/
struct sizeOfMatrix* charArrayIssizeOfMatrix (char * chars, int size) {
    struct sizeOfMatrix* mtxSize = (struct sizeOfMatrix*) malloc( sizeof(struct sizeOfMatrix) );
    int splitCount = 0;
    char temp[BUFFER_SIZE];
    strcpy(temp, chars);

    char* split = strtok(temp, ",");
    while (split != NULL) {
        double parsed = atof(split);
        if ( parsed > 0 ) 
        {
            if (splitCount == 0) {
                mtxSize->x = parsed;
            } else if (splitCount == 1) {
                mtxSize->y = parsed;
            } else {
                return NULL;
            }
            splitCount++;
        }
        split = strtok(NULL, ",");
    }

    return mtxSize;
}


// Check to see if the first character in the array is empty
int emptyChar (char * chars, int size) {
    return chars[0] == '\n';
}


/*
* Obtain all Matrixs in file and seperate them with comma.
*/
struct Matrix** Matrices;
struct Matrix** loadMatrix (char * fileName) {
    if (!fileName) {
        printf("Unable to find file '%s'\n", fileName);
        return NULL;
    }
    int load = readFile(fileName);
    FILE * filePtr;
    filePtr = fopen(fileName, "r");
    if (filePtr == NULL) {
        printf("Unable to load file '%s'\n", fileName);
        return NULL;
    }

    struct Matrix** matrices = malloc( sizeof(struct Matrix) * load );
    struct sizeOfMatrix* currentsizeOfMatrix = (struct sizeOfMatrix*) malloc(sizeof(struct sizeOfMatrix));
    currentsizeOfMatrix->x = currentsizeOfMatrix-> y = 0;

    double* currentMatrixValues = NULL;
    double* parseLineOfDoubles(char* lineChars, struct sizeOfMatrix mtxSize) {
    int lineYCount = 0;
    double* lineNum = malloc (sizeof(double) * mtxSize.y);
    const char splitter[2] = ",";
    char* split = strtok(lineChars, splitter);
    while (split) {
        double parsed = atof(split);
        if ( parsed > 0 ) {
            lineNum[lineYCount] = parsed;
            lineYCount += 1;
        }
        split = strtok(NULL, splitter);
    }
    return lineNum;
}

    int lineCount = 0;
    int countCurMat = 0;
    char readBuffer[BUFFER_SIZE];
    while ( fgets(readBuffer, BUFFER_SIZE, filePtr) != NULL ) {
        struct sizeOfMatrix* mtxSize = charArrayIssizeOfMatrix(readBuffer, BUFFER_SIZE);
        if ( mtxSize != NULL && currentsizeOfMatrix->x == 0 && currentsizeOfMatrix->y == 0 ) {
            currentsizeOfMatrix = mtxSize;
        } 
	else if 
	( emptyChar(readBuffer, BUFFER_SIZE) > 0 ) 
	{
            struct Matrix* thisMatrix = malloc( sizeof(struct Matrix) );
            thisMatrix->values = currentMatrixValues;
            thisMatrix->size = *currentsizeOfMatrix;

            matrices[countCurMat] = thisMatrix;
            countCurMat++;

            lineCount = 0;
            currentsizeOfMatrix->x = currentsizeOfMatrix->y = 0;
            currentMatrixValues = NULL;
        } 
	else 
	{
            if (currentMatrixValues == NULL) {
                currentMatrixValues = malloc( sizeof(double) * currentsizeOfMatrix->y);
            }

            double* values = parseLineOfDoubles(readBuffer, *currentsizeOfMatrix);
            for (int i = 0; i < currentsizeOfMatrix->y; i++) {
                int index = (lineCount * currentsizeOfMatrix->y) + i;
                currentMatrixValues[index] = values[i];
            }
            lineCount++;
        }
    }


    struct Matrix* thisMatrix = malloc( sizeof(struct Matrix) );
    thisMatrix->values = currentMatrixValues;
    thisMatrix->size = *currentsizeOfMatrix;

    matrices[countCurMat] = thisMatrix;

    countCurMat++;
    fclose(filePtr);

    return matrices;
}

void printMatrix(double* matrix, int xCount, int yCount);
void* matrixCalculation(void* param);
int multiplyingMatrix(struct Matrix a, struct Matrix b);

/*
* Reads the matrix data file "1718170-matrices.txt" and loads matrix A and B data, storing 
* them as an array. Checking to see if A and B can be multiplying together. threads will 
* created and store array of threads used for pthreads.
*/
int main (int argc, char* argv[]) {


    char* filePath = "./1718170-matrices.txt";
    if (argc > 1) {
        filePath = argv[1];
    }
    int load = readFile(filePath);
    Matrices = loadMatrix(filePath);
    printf("File name: %s \n", filePath);
    printf("Matrices loaded: '%d'\n", load);
    
    struct Matrix** allFinalMatrices = malloc( sizeof(struct Matrix) * load );
    int successfulMatrixCount = 0;
    for (int i = 0; i < load; i += 2) {
        struct Matrix matrixA = *(Matrices[i]);
        struct Matrix matrixB = *(Matrices[i + 1]);
        struct Matrix* resultMatrix = malloc( sizeof(struct Matrix) );
        resultMatrix->size.x = matrixA.size.x;
        resultMatrix->size.y = matrixB.size.y;
        resultMatrix->values = malloc( sizeof(double) * resultMatrix->size.x * resultMatrix->size.y );

        int multiplication = multiplyingMatrix(matrixA, matrixB);
        if (multiplication == 1) 
        {
            int countNumOfThread = resultMatrix->size.x * resultMatrix->size.y;
            pthread_t* threadIds = malloc( sizeof(pthread_t) * countNumOfThread);
            
            for (int i = 0; i < countNumOfThread; i++) {
                struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs));
                if (args != NULL) 
                {
                    args->threadNum = i;
                    args->iterateAmount = matrixA.size.y;
                    args->finalResultInfo = resultMatrix;
                    args->valueA = matrixA.values;
                    args->valueB = matrixB.values;

                    pthread_create( &threadIds[i], NULL, matrixCalculation, args);
                }
            }

            for (int i = 0; i < countNumOfThread; i++) {
                pthread_join( threadIds[i], NULL );
            }
            allFinalMatrices[successfulMatrixCount] = resultMatrix;
            successfulMatrixCount++;
        }
        else 
        {
            printf("Unable to multiply Matrices[%d] '%dx%d' and  Matrices[%d] '%dx%d'\n", i, matrixA.size.x, matrixA.size.y, i + 1, matrixB.size.x, matrixB.size.y);

	
        }

    }

/*
* Matrix results will be outputed & saved onto a new a new file "matrixresults-1718170.txt" 
*/
    char* outputMatrix = "./matrixresults-1718170.txt";
    if (argc > 2) {
        outputMatrix = argv[2];
    }
printf("matrices into file '%s'\n", outputMatrix);
    int success = saveMatricesToFile(outputMatrix, allFinalMatrices, successfulMatrixCount);
    if (success == 1) {
        printf("Successfully saved '%d' matrices into file '%s'\n", successfulMatrixCount, outputMatrix);
    } else {
        printf("Unable to save to file '%s'\n", outputMatrix);
    }

    return 0;
}

/*
* Matrix total calculation by finding row of matrix A & column of matrix B
*/
void* matrixCalculation (void* param) {
    struct ThreadArgs* args = (struct ThreadArgs*)param;
    struct sizeOfMatrix resultsizeOfMatrix = args->finalResultInfo->size;
    int index = args->threadNum;
    int col = index %  resultsizeOfMatrix.x;
    int row = (index - col) / resultsizeOfMatrix.y;
    
    if ( row >= resultsizeOfMatrix.x || col >= resultsizeOfMatrix.y ) {
        return 0;
    }
    double total = 0;
    for (int i = 0; i < args->iterateAmount; i++) {
        int mtxAThisIndex = (row * (resultsizeOfMatrix.x - 1)) + i;
        int mtxBThisIndex = (i * resultsizeOfMatrix.y) + col;
        double aVal = args->valueA[ mtxAThisIndex ];
        double bVal = args->valueB[ mtxBThisIndex ];
        total += aVal * bVal;
        args->finalResultInfo->values[args->threadNum] = total;
        return NULL;
	}
   }

/* 
* Printing out the matrix array to the console with it's indexes and it's values 
*
void printMatrix (double* matrix, int xCount, int yCount) {
    for (int i = 0; i < xCount; i++) {
        for(int j = 0; j < yCount; j++) {
            printf("%f", *(matrix + (i * yCount) + j));

            /// Print white space if not reached final column
            if ( j < yCount - 1 ) {
                printf("    ");
            }
        }
        printf("\n");
    }
}*/

/*
* Returns true if matrix a can be multiplied against matrix b
*/
int multiplyingMatrix(struct Matrix a, struct Matrix b) {
    // Only able to multiply if Matrix A Y column count
    // is equal to MatrixB X row count
    if (a.size.y == b.size.x) {
        return 1;
    }
    return 0;
}

/*
* This saves the matrices input to the file path. The matrixs will be seperated with commas and print a new line when looping diffrent matrixs
*/
int saveMatricesToFile ( char* filePath, struct Matrix** matrices, int countMat) {
    FILE* outputFile = fopen(filePath, "w");
    if (outputFile == NULL)
    {
        printf("Unable to open filePath '%s'. Check it isn't open\n", filePath);
        return 0;
    }
    for( int i = 0; i < countMat; i++ ) {
        struct Matrix matrix = *(matrices[i]);
        fprintf(outputFile, "%d,%d\n", matrix.size.x, matrix.size.y);

        for (int j = 0; j < matrix.size.x; j++) {
            int rowIndex = (j * matrix.size.y);
            for( int k = 0; k < matrix.size.y; k++ ) {
                fprintf(outputFile, "%f", matrix.values[rowIndex + k]);
                if (k < matrix.size.y - 1) {
                    fprintf(outputFile, ",");
                }
            }
            fprintf(outputFile, "\n");
        }
        fprintf(outputFile, "\n");
    }
    fclose(outputFile);
    return 1;
}
