#ifndef PARSE_H
#define PARSE_H 

#include "chip.h"
#include "str.h"
#include "lexer.h"

typedef struct {
  int* Data;
  int Length;
} bool_array_t;

typedef struct {
  string_t ChipName;
  variable_array_t Inputs,
                   Outputs,
                   Temps;
  part_array_t Parts;
  bool_array_t ResolvedParts;
} chip_builder_t;

typedef struct {
  chip_builder_t* Data;
  int Length;
  int Capacity;
} chip_builder_array_t;

chip_builder_array_t InitChipBuilderArray(void);
void CloseChipBuilderArray(chip_builder_array_t* Array);
void AppendChipBuilderArray(chip_builder_array_t* Array, chip_builder_t ChipBuilder);

chip_builder_t ProcessTokens(token_t* TokenList);
void ResolveParts(chip_builder_array_t ChipBuilderArray);

#endif // PARSE_H
