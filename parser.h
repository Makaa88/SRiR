#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

extern int* vertices_matrix;
extern int vertices;


void PrintMatrix();
void CheckMatrixSize(FILE* file);
void InitializeMatrix(FILE* file);
void LoadDataFromFile(const char* file_name);
void FreeMatrix();

#endif