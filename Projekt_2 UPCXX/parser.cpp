#include "parser.h"

upcxx::global_ptr<int> vertices_matrix = nullptr;
int vertices= 0;

void PrintMatrix()
{
	int* local_vertices_matrix = vertices_matrix.local();
	for(int i = 0; i < vertices; i++)
	{
		for(int j = 0; j < vertices; j++)
		{
			std::cout <<local_vertices_matrix[vertices * i + j]<< " ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl << std::endl;
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
	std::cout << "MATIRX SIZE: " << vertices << " x " << vertices << std::endl;
	rewind(file);
}

void InitializeMatrix(FILE* file)
{
	//vertices_matrix = new int [vertices*vertices];
	vertices_matrix = upcxx::new_array<int>(vertices*vertices);

	if(vertices_matrix == nullptr)
	{
		printf("Cannot initialize vertices_matrix\n");
		std::cout << "Cannot initialize vertices matrix" << std::endl;
		return;
	}

	rewind(file);

	int i = 0;
	int j = 0;
	int* local_vertices_matrix = vertices_matrix.local();
	while(!feof(file))
	{
		fscanf(file, "%d", &local_vertices_matrix[vertices * i + j]);
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
		std::cout << "Cannot open file "<< std::endl;
		return;
	}

	CheckMatrixSize(file);
	InitializeMatrix(file);
	PrintMatrix();

	fclose(file);
}



void FreeMatrix()
{

	upcxx::delete_array(vertices_matrix);
	vertices_matrix = nullptr;
}