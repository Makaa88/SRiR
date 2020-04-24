#include "parser.h"

int* vertices_matrix = NULL;
int vertices= 0;

void PrintMatrix()
{
	for(int i = 0; i < vertices; i++)
	{
		for(int j = 0; j < vertices; j++)
		{les
			printf("%d ", vertices_matrix[vertices * i + j]);
		}
		printf("\n");
	}

	printf("\n\n\n");
}

void CheckMatrixSize(FILE* file)
{
	char c;
	while( (c = (char)fgetc(file)) != EOF)
	{
		if(c == '\n')
			vertices++;
	}
	vertices++;

	printf("MATRIX SIZE: %d x %d \n", vertices, vertices);
	rewind(file);
}

void InitializeMatrix(FILE* file)
{
	vertices_matrix = (int*)malloc(vertices * vertices* sizeof(int));

	if(vertices_matrix == NULL)
	{
		printf("Cannot initialize vertices_matrix\n");
		return;
	}

	rewind(file);

	int i = 0;
	int j = 0;

	while(!feof(file))
	{
		fscanf(file, "%d", &vertices_matrix[vertices * i + j]);
		j++;
		if(j == vertices) {
			j = 0;
			i++;
		}
	}

}

void LoadDataFromFile(const char* file_name)
{
	FILE *file;
	file = fopen(file_name, "r");
	if(file == NULL) 
	{
		printf("Cannot open file\n");
		return;
	}

	CheckMatrixSize(file);
	InitializeMatrix(file);
	PrintMatrix();

	fclose(file);
}



void FreeMatrix()
{

	free(vertices_matrix);
	vertices_matrix = NULL;
}