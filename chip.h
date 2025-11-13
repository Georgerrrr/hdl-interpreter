#ifndef CHIP_H
#define CHIP_H 

#include "str.h"

typedef struct {
  string_t Name;
  int Length;
  int BufferIndex;
} variable_t;

typedef struct {
  variable_t* Data;
  int Length;
  int Capacity;
} variable_array_t;

typedef struct {
  string_t Key,
           Value;
  int RangeStart,
      RangeEnd;
} argument_t;

typedef struct {
  string_t ChipName;
  argument_t* Data;
  int Length;
  int Capacity;
} part_t;

typedef struct {
  part_t* Data;
  int Length;
  int Capacity;
} part_array_t;

struct _chip;

typedef struct {
  struct _chip* Data;
  int Length;
  int Capacity;
} chip_array_t;

typedef struct _chip {
  string_t Name;

  unsigned char* Buffer; /* Large buffer storing all inputs/outpts/local variables */
  int BufferSize;

  /* These won't allocate anything, just store pointers to locations in the buffer */
  variable_array_t Inputs;
  variable_array_t Outputs;
  variable_array_t Temps;

  part_array_t Parts;  

  void (*ProcessChip)(struct _chip*);
} chip_t;

typedef enum {
  printBinary,
  printDecimal,
  printHex
} _print_fmt;

extern chip_array_t Chips;

void InitChips(void);
void CloseChips(void);

void AppendVariableArray(variable_array_t* Array, variable_t Var);
variable_array_t EmptyVariableArray(void);

void InitPart(part_t* Part, string_t Name);
void SetPartArgumentKey(part_t* Part, string_t Key);
void SetPartArgumentValue(part_t* Part, string_t Value);
void SetPartArgumentRangeStart(part_t* Part, int RangeStart);
void SetPartArgumentRangeEnd(part_t* Part, int RangeEnd);
void IncrementPartArguments(part_t* Part);
void ClosePart(part_t* Part);

void InitPartsArray(part_array_t* Parts);
void AppendPartsArray(part_array_t* Parts, part_t Part);
void ClosePartsArray(part_array_t* Parts);

int ChipExists(string_t Name);
chip_t* ChipFromName(string_t Name);
variable_t* GetVariable(string_t Name, string_t Key);

int GetVariableLength(chip_t* Chip, string_t Key);
unsigned char GetVariableValue(chip_t* Chip, string_t Key, int Index);
void SetVariableValue(chip_t* Chip, string_t Key, int Index, unsigned char Value);

void PrintVariableNames(chip_t* Chip);
void PrintVariables(chip_t* Chip, _print_fmt Format);

void NewChip(string_t Name, variable_array_t Inputs, variable_array_t Outputs, variable_array_t Temps, part_array_t Parts);

#endif // CHIP_H
