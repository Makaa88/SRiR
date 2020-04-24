#include <limits.h>
#include <stdbool.h>
#include "parser.h"
#include "mpi.h"

const int ROOT = 0;

void InitializeVisitedAndWageAndIndex(int node_data_size, bool* visited_vertices, int* wage_table, int* verticles_index, int starting_index)
{
	for(int i = 0; i < node_data_size; i++)
	{
		visited_vertices[i] = false;
		wage_table[i] = vertices_matrix[starting_index + i];
		verticles_index[i] = starting_index + i;
	}
}

int CountVisitedVertices(bool* visited_vertices, int node_data_size)
{
	int visited = 0;
	for(int i = 0; i < node_data_size; i++)
		if(visited_vertices[i]) visited++;

	return visited;
}

int FindNextMinimalVertice(bool* visited_vertices, int* wage_table, int node_data_size)
{
	int current_index = -1;
	int current_wage = INT_MAX;

	for(int i = 0; i < node_data_size; i++)
	{
		if(!visited_vertices[i])
		{
			if((wage_table[i] != -1) && (wage_table[i] < current_wage)) 
			{
				current_index = i;
				current_wage = wage_table[i];
			}
		}
	}
	//printf("Going to verticle nr %d with wage %d\n", current_index, current_wage);
	return current_index;

}

void UpdateWageTable(int* wage_table, bool* visited_vertices, int global_min, int node_data_size, int starting_index)
{
	for(int i = 0; i < node_data_size; i++)
	{
		if(!visited_vertices[i])
		{
			int vertice_value = vertices_matrix[vertices * global_min + i + starting_index];
			if((wage_table[i] == -1) || ((vertice_value != 0) && (vertice_value != -1) && (vertice_value < wage_table[i])))
			{
				wage_table[i] = vertice_value;
			}
		}
	}
}

void PrintWageTable(int* wage_table, int size)
{	printf("[");
	for(int i = 0; i < size; i++)
	{
		printf("%d ", wage_table[i] );
	}
	printf("]\n");
}

void UpdateVisited(bool* visited_vertices, int* verticles_index, int global_min, int node_data_size)
{
	bool found = false;
	int idx = -1;
	for(int i = 0; i <  node_data_size; i++)
	{
		if(verticles_index[i] == global_min)
		{
			found = true;
			idx = i;
			break;
		}
	}

	if(found)
		visited_vertices[idx] = true;
}


int main(int argc, char *argv[])
{
	/*if(argc != 2)
	{
		printf("Wrong number of arguments\nSyntax: ./program_name file_name\n");
		return -1;
	}*/
	int node_id;
	int node_size;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &node_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &node_id);

	const char* file_name = "input2.txt";


	if(node_id == ROOT)
	{
		LoadDataFromFile(file_name);
	}

	//wyslylamy informacje o ilosci wieszcholkow
	MPI_Bcast(&vertices, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

	if(node_id != ROOT)
	{
		//alokacja macierzy wartosci krawedzi pomiedzy wierzcholkami
		vertices_matrix = (int*)malloc(vertices*vertices * sizeof(int));
	}

	// Tablica odpowiednich rozmiarow dla kazdego procesu
	int* node_data_sizes =  (int*)malloc(node_size * sizeof(int)); 

	//Tablica przesuniec
	int* dist = (int*)malloc(node_size * sizeof(int));

	//Ilosc wierzcholkow dla procesu
	int node_data_size = vertices/node_size + (vertices % node_size <= node_id ? 0 : 1);

	//Zbieranie informacji o rozmiarach
	//wysylane dane, rozmiar, typ, gdzie odbierany, rozmiar, typ, root, komunikator
	MPI_Gather(&node_data_size, 1, MPI_INT, node_data_sizes, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

	//Rozsylanie informacji o rozmiarach
	MPI_Bcast(node_data_sizes, node_size, MPI_INT, ROOT, MPI_COMM_WORLD);

	//Uzupelnianie tebalicy przesuniec
	dist[0] = 0;
	for(int i = 1; i < vertices; i++)
	{
		dist[i] = dist[i-1] + node_data_sizes[i-1];
	}

	//Rozsylanie informacji o macierzy
	MPI_Bcast(vertices_matrix, vertices*vertices, MPI_INT, ROOT, MPI_COMM_WORLD);

	//TODO zmienic rozmiar na odpowiedni dla kazdego procesu
	//Nie potrzebyjemy przehcowywac nadmiarowej ilosci danych jak i tak 
	//tylko operujemy na czesci tablicy
	bool* visited_vertices = malloc(node_data_size * sizeof(bool));
	int* verticles_index = (int*)malloc(node_data_size * sizeof(int));
    int* wage_table = malloc(node_data_size * sizeof(int));

   

    int starting_vertex = 0;
    
 
    //Ilosc odwiedzoncyh ogolem
    int all_visited = 0;


    int starting_index = dist[node_id];
    int ending_index = dist[node_id] + node_data_sizes[node_id];

    //Inicjalizacja poczatkoweych wartosci tabeli wag i odwiedzonych wierzhcolkow
	InitializeVisitedAndWageAndIndex(node_data_size, visited_vertices, wage_table, verticles_index, starting_index);

	//Proces ROOT zawsze posiada wierzhcolek o indexie 0
	if(node_id == ROOT)
	{
		visited_vertices[0] = true;
	}

	//kazdy proces liczy odwiedzone wierzcholki
	 //Ilosc odwiedzonych w procesie
    int visited_vertices_counter =  CountVisitedVertices(visited_vertices, node_data_size);
	MPI_Barrier(MPI_COMM_WORLD);

	//Ilosco odwiedzonych wierzcholkow w procesie glownym
	MPI_Reduce(&visited_vertices_counter, &all_visited, 1, MPI_INT, MPI_SUM, ROOT, MPI_COMM_WORLD);

	//Rozsylanie informacji o ilosci odzwiedzonych
	MPI_Bcast(&all_visited, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

	if(node_id == ROOT)
		printf("Current visited %d of %d\n", all_visited, vertices);

	if(node_id == ROOT){
			printf("Current visited %d of %d\n", all_visited, vertices);
					PrintWageTable(wage_table, node_data_size);
		}
		MPI_Barrier(MPI_COMM_WORLD);
		if(node_id == 1)
		{
			PrintWageTable(wage_table, node_data_size);
		}

	while(all_visited != vertices)
	{
		//Tablica okreslajaca lokalne minim i numer wierzcholka 
		int local_min[2] = {INT_MAX, -1};
		//Tablica do zebrania informacji o globalnym minimun i do ktorego procesu ono nalezy
		int global_min[2];

		//Lokalnie szukamy minimalnego wierzcholka
		int u = FindNextMinimalVertice(visited_vertices, wage_table, node_data_size);
		
		if(u != -1)
		{
			local_min[0] = wage_table[u];
			local_min[1] = verticles_index[u];
		}

		//Zbieranie informacji o globalnym minimum
		MPI_Allreduce(local_min, global_min, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);

		if(node_id == ROOT)
			printf("Min and idx: %d %d\n", global_min[0], global_min[1]);

		//Uaktualnienie odiwedzonych dla konkertnego procesu
		UpdateVisited(visited_vertices, verticles_index, global_min[1], node_data_size);
		



		UpdateWageTable(wage_table, visited_vertices, global_min[1], node_data_size, starting_index);





		//Zliczanie odwiedzonych i wysylanie informacji
		visited_vertices_counter =  CountVisitedVertices(visited_vertices, node_data_size);
		MPI_Barrier(MPI_COMM_WORLD);

		//Ilosco odwiedzonych wierzcholkow w procesie glownym
		MPI_Reduce(&visited_vertices_counter, &all_visited, 1, MPI_INT, MPI_SUM, ROOT, MPI_COMM_WORLD);

		if(node_id == ROOT){
			printf("Current visited %d of %d\n", all_visited, vertices);
					PrintWageTable(wage_table, node_data_size);
		}
		MPI_Barrier(MPI_COMM_WORLD);
		if(node_id == 1)
		{
			PrintWageTable(wage_table, node_data_size);
		}

		//Rozsylanie informacji o ilosci odzwiedzonych
		MPI_Bcast(&all_visited, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

		MPI_Barrier(MPI_COMM_WORLD);
		printf("\n");
	}

	//Tablica wynikowa d
	int* d = malloc(vertices * sizeof(int));

	MPI_Gather(wage_table, node_data_size, MPI_INT, d, node_data_size, MPI_INT, ROOT, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	if(node_id == ROOT)
	{
		PrintWageTable(d, vertices);
	}


	free(d);

	free(visited_vertices);
	free(wage_table);
	free(verticles_index);

	FreeMatrix();
	MPI_Finalize();
	return 0;
}