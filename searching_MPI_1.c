
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <mpi.h>



////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////

void master();
void slave();



//char *textData;
//int textLength;

//char *patternData;
//int patternLength;

clock_t c0, c1;
time_t t0, t1;

void outOfMemory()
{
	fprintf (stderr, "Out of memory\n");
	exit (0);
}

void readFromFile (FILE *f, char **data, int *length)
{
	int ch;
	int allocatedLength;
	char *result;
	int resultLength = 0;

	allocatedLength = 0;
	result = NULL;

	

	ch = fgetc (f);
	while (ch >= 0)
	{
		resultLength++;
		if (resultLength > allocatedLength)
		{
			allocatedLength += 10000;
			result = (char *) realloc (result, sizeof(char)*allocatedLength);
			if (result == NULL)
				outOfMemory();
		}
		result[resultLength-1] = ch;
		ch = fgetc(f);
	}
	*data = result;
	*length = resultLength;
}


char* readTextData(){
	FILE *f;
        char fileName[1000];
	char* textData;
	int textLength;
#ifdef DOS
        sprintf (fileName, "inputs\\text.txt");
#else
        sprintf (fileName, "inputs/text.txt");
#endif
        f = fopen (fileName, "r");
        if (f == NULL){
		printf("no text data found!");
		//exit(-1);

	}
                
        readFromFile (f, &textData, &textLength);
        fclose (f);

	return textData;	
}



char* readPatternData(int testNumber){
	FILE *f;
        char fileName[1000];
	char* patternData;
	int patternLength;
	#ifdef DOS
        sprintf (fileName, "inputs\\pattern%d.txt", testNumber);
#else
        sprintf (fileName, "inputs/pattern%d.txt", testNumber);
#endif
        f = fopen (fileName, "r");
        if (f == NULL)
                printf ("pattern %d no found\n", testNumber);
        readFromFile (f, &patternData, &patternLength);
        fclose (f);

        printf ("Read pattern %d\n", testNumber);

        return patternData;	
}


int getArrayLength(char* array){
	
}



int readPatternLength(int testNumber){
        FILE *f;
        char fileName[1000];
	char* patternData;
	int patternLength;
        #ifdef DOS
        sprintf (fileName, "inputs\\pattern%d.txt", testNumber);
#else
        sprintf (fileName, "inputs/pattern%d.txt", testNumber);
#endif
        f = fopen (fileName, "r");
        if (f == NULL)
                return 0;
        readFromFile (f, &patternData, &patternLength);
        fclose (f);

        printf ("Read pattern %d\n", testNumber);

        return patternLength;
}




/*
int readData (int testNumber)
{
	FILE *f;
	char fileName[1000];
#ifdef DOS
        sprintf (fileName, "inputs\\text.txt");
#else
	sprintf (fileName, "inputs/text.txt");
#endif
	f = fopen (fileName, "r");
	if (f == NULL)
		return 0;
	readFromFile (f, &textData, &textLength);
	fclose (f);
#ifdef DOS
        sprintf (fileName, "inputs\\pattern%d.txt", testNumber);
#else
	sprintf (fileName, "inputs/pattern%d.txt", testNumber);
#endif
	f = fopen (fileName, "r");
	if (f == NULL)
		return 0;
	readFromFile (f, &patternData, &patternLength);
	fclose (f);

	printf ("Read pattern %d\n", testNumber);

	return 1;

}
*/


int hostMatch(long *comparisons,char textData[],int textLength,char patternData[],int patternLength)
{
	int i,j,k, lastI;
	
	i=0;
	j=0;
	k=0;
	lastI = textLength-patternLength;
        *comparisons=0;

	while (i<=lastI && j<patternLength)
	{
                (*comparisons)++;
		if (textData[k] == patternData[j])
		{
			k++;
			j++;
		}
		else
		{
			i++;
			k=i;
			j=0;
		}
	}
	if (j == patternLength)
		return i;
	else
		return -1;
}
void processData(int testNumber,char textData[],int textLength,char patternData[],int patternLength)
{
	unsigned int result;
        long comparisons;

	printf ("Text length = %d\n", textLength);
	printf ("Pattern %d length = %d\n",testNumber, patternLength);
	
	result = hostMatch(&comparisons,textData,textLength,patternData,patternLength);
	if (result == -1)
		printf ("Pattern not found\n");
	else
		printf ("Pattern %d found at position %d\n",testNumber, result);
        printf ("# comparisons for finding pattern %d = %ld\n",testNumber, comparisons);

}


int main(int argc, char **argv)
{
	int testNumber=1;
	int npes ,myrank;
	
	
	
	c0 = clock(); t0 = time(NULL);
	
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&npes);
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	
	if(myrank == 0)
		master();
	else
		slave();

	MPI_Finalize();	 
	c1 = clock(); t1 = time(NULL);
        printf("process %d elapsed wall clock time = %ld\n",myrank,(long)(t1-t0));
        printf("process %d elapsed CPU time = %lf\n\n",myrank,(double) (c1 - c0)/CLOCKS_PER_SEC);

}

void master(){
	int ntasks;;
	MPI_Status status;
	int testNumber;
	MPI_Comm_size(MPI_COMM_WORLD,&ntasks);

	char* textData;

	textData = readTextData();//read text data
	int textLength =  strlen(textData);//calculate the length of text
	MPI_Bcast(&textLength,1,MPI_INT,0,MPI_COMM_WORLD);//send the text length to every slave process
	MPI_Bcast(textData,10000001,MPI_CHAR,0,MPI_COMM_WORLD);//send the text to every slave process

	char* pattern;
	int patternLength; 
	for(testNumber=1;testNumber<ntasks;testNumber++){		
		pattern = readPatternData(testNumber);
		//patternLength = strlen(pattern);		
		//printf("pattern length : %s\n",pattern);	
		MPI_Send(pattern,1001,MPI_CHAR,testNumber,0,MPI_COMM_WORLD);
	}
	
}

void slave(){
	char pattern[1000];
	char textData[10000000];
	int result;
	MPI_Status status;
	int myrank;
	int textLength;
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	
	MPI_Bcast(&textLength,1,MPI_INT,0,MPI_COMM_WORLD);//receive the text length from master process

	MPI_Bcast(&textData,10000001,MPI_CHAR,0,MPI_COMM_WORLD);//receive text from master process	
	//printf("slave %d receive textData [%s] from master\n",myrank,textData);

	MPI_Recv(&pattern,1000,MPI_CHAR,0,0,MPI_COMM_WORLD,&status);//receive pattern from master process
	int patternLength = strlen(pattern);
	//printf("slave %d receive work [%s] from master\n",myrank,pattern);
	processData(myrank,textData,textLength,pattern,1000);	
}

