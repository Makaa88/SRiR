#include "parser.h"


void InitializeVisitedAndWageAndIndex(int node_data_size, bool* visited_vertices, int* wage_table, int* verticles_index, int starting_index)
{
	for(int i = 0; i < node_data_size; i++)
	{
		visited_vertices[i] = false;
		wage_table[i] = upcxx::rget(vertices_matrix + (starting_index + i)).wait();
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

void UpdateWageTable(int* wage_table, bool* visited_vertices, int global_min, int node_data_size, int starting_index)
{
	for(int i = 0; i < node_data_size; i++)
	{
		if(!visited_vertices[i])
		{
			int vertice_value = upcxx::rget(vertices_matrix + (vertices * global_min + i + starting_index)).wait();
			if((wage_table[i] == -1) || ((vertice_value != 0) && (vertice_value != -1) && (vertice_value < wage_table[i])))
			{
				wage_table[i] = vertice_value;
			}
		}
	}
}




int main(int argc, char const *argv[])
{
	upcxx::init();

	if(upcxx::rank_me() == 0)
	{
		const char* file_name = argv[1];
		LoadDataFromFile(file_name);
	}

	vertices_matrix = upcxx::broadcast(vertices_matrix, 0).wait();
	vertices = upcxx::broadcast(vertices, 0).wait();


	std::cout << "Process: " << upcxx::rank_me() << " have size: " << vertices <<  "  " << upcxx::rank_n() << std::endl;

	upcxx::global_ptr<int> node_data_sizes = nullptr;

	if(upcxx::rank_me() == 0)
	{
		node_data_sizes =  upcxx::new_array<int>(upcxx::rank_n());
	}
	
	node_data_sizes = upcxx::broadcast(node_data_sizes, 0).wait();
	upcxx::global_ptr<int> process_node_data_size_place = node_data_sizes + upcxx::rank_me();

	int node_data_size = vertices/upcxx::rank_n() + (vertices % upcxx::rank_n() <= upcxx::rank_me() ? 0 : 1);
	upcxx::rput(node_data_size, process_node_data_size_place).wait();
	upcxx::barrier();

	upcxx::global_ptr<int> node_data_starting_indexes = nullptr;

	if(upcxx::rank_me() == 0)
	{
		node_data_starting_indexes =  upcxx::new_array<int>(upcxx::rank_n());

		int* local_node_starting_indexes = node_data_starting_indexes.local();
		int* local_node_data_sizes = node_data_sizes.local();
		local_node_starting_indexes[0] = 0;
		for(int i = 1; i < upcxx::rank_n(); i++)
		{
			local_node_starting_indexes[i] = local_node_starting_indexes[i-1] + local_node_data_sizes[i-1];
		}
	}
	node_data_starting_indexes = upcxx::broadcast(node_data_starting_indexes, 0).wait();

	

	bool* visited_vertices = new bool[node_data_size];
	int* verticles_index = new int[node_data_size];
	int* wage_table = new int[node_data_size];


	int* result_table = nullptr;

	//int starting_index = node_data_starting_indexes.local()[upcxx::rank_me()];
	int starting_index = upcxx::rget(node_data_starting_indexes + upcxx::rank_me()).wait();
    //int ending_index = dist[upcxx::rank_me()] + local_node_data_sizes[upcxx::rank_me()];*/

	std::cout << "Process: " << upcxx::rank_me() << " have data size: " << node_data_size << std::endl;
	upcxx::barrier();


	InitializeVisitedAndWageAndIndex(node_data_size, visited_vertices, wage_table, verticles_index, starting_index);

	if(upcxx::rank_me() == 0)
	{
		result_table = new int[vertices];
		visited_vertices[0] = true;
		result_table[0] = 0;
	}

	int visited_vertices_counter =  CountVisitedVertices(visited_vertices, node_data_size);
	int all_visited = upcxx::reduce_all(visited_vertices_counter, upcxx::op_fast_add).wait();


	while(all_visited != vertices)
	{
		upcxx::global_ptr<int> current_minimal_wages = nullptr;
		upcxx::global_ptr<int> current_minimal_indexes = nullptr;
		if(upcxx::rank_me() == 0)
		{
			current_minimal_wages =  upcxx::new_array<int>(upcxx::rank_n());
			current_minimal_indexes =  upcxx::new_array<int>(upcxx::rank_n());
		}
		current_minimal_wages = upcxx::broadcast(current_minimal_wages, 0).wait();
		current_minimal_indexes = upcxx::broadcast(current_minimal_indexes, 0).wait();


		//Lokalnie szukamy minimalnego wierzcholka, zwracamy minimalna index
		int minimal_index = FindNextMinimalVertice(visited_vertices, wage_table, node_data_size);
		int minimal_wage = INT_MAX;
		if(minimal_index != -1)
		{ 
			minimal_wage = wage_table[minimal_index];
			minimal_index = verticles_index[minimal_index];
		}

		upcxx::rput(minimal_wage, current_minimal_wages + upcxx::rank_me()).wait();
		upcxx::rput(minimal_index, current_minimal_indexes + upcxx::rank_me()).wait();

		int global_minimal_wage = upcxx::reduce_all(minimal_wage, upcxx::op_fast_min).wait();

		int process_to_update = -1;

		if(upcxx::rank_me() == 0)
		{
			int* local_current_minamal_wages = current_minimal_wages.local();
			int* local_current_minamal_indexes = current_minimal_indexes.local();
			for(int i  = 0; i < upcxx::rank_n(); i++)
			{
				if(local_current_minamal_wages[i] == global_minimal_wage)
				{
					process_to_update = i;
					break;
				}
			}

			result_table[local_current_minamal_indexes[process_to_update]] = global_minimal_wage; 
		}

		process_to_update = upcxx::broadcast(process_to_update, 0).wait();
		int update_index = upcxx::rget(current_minimal_indexes + process_to_update).wait();
		UpdateVisited(visited_vertices, verticles_index, update_index, node_data_size);
		UpdateWageTable(wage_table, visited_vertices, update_index, node_data_size, starting_index);

		if(upcxx::rank_me() == 0)
		{
			upcxx::delete_array(current_minimal_wages);
			upcxx::delete_array(current_minimal_indexes);
		}


		visited_vertices_counter = CountVisitedVertices(visited_vertices, node_data_size);
		all_visited = upcxx::reduce_all(visited_vertices_counter, upcxx::op_fast_add).wait();
		if(upcxx::rank_me() == 0) std::cout << all_visited << std::endl;
	}

	upcxx::barrier();

	if(upcxx::rank_me() == 0)
	{
		for(int i = 0; i < vertices; i++)
			std::cout << result_table[i] << " ";

		std::cout << std::endl << std::endl;
	}



	//Zwalnianie pamieci
	delete[] visited_vertices;
	delete[] verticles_index;
	delete[] wage_table;
	if(upcxx::rank_me() == 0)
	{
		upcxx::delete_array(node_data_starting_indexes);
		upcxx::delete_array(node_data_sizes);
		FreeMatrix();
		delete[] result_table;
	}

	return 0;
}