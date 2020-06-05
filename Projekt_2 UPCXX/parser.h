#ifndef PARSER_H
#define PARSER_H

#include <upcxx/upcxx.hpp>
#include <iostream>
#include <stdlib.h>
#include <stdbool.h>

extern upcxx::global_ptr<int> vertices_matrix;
extern int vertices;


void PrintMatrix();
void CheckMatrixSize(FILE* file);
void InitializeMatrix(FILE* file);
void LoadDataFromFile(const char* file_name);
void FreeMatrix();

#endif