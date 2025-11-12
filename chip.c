#include <stdio.h>
#include <stdlib.h>

#include "alloc.h"
#include "chip.h"

chip_array_t Chips;

void InitPart(part_t* Part, string_t Name) {
  InitString(&Part->ChipName, Name);

  Part->Capacity = 4;
  Part->Data = tMalloc(sizeof(argument_t) * Part->Capacity);
  Part->Length = 0;
}

void SetPartArgumentKey(part_t* Part, string_t Key) {
  InitString(&Part->Data[Part->Length].Key, Key);
}

void SetPartArgumentValue(part_t* Part, string_t Value) {
  InitString(&Part->Data[Part->Length].Value, Value);
}

void SetPartArgumentRangeStart(part_t* Part, int RangeStart) {
  Part->Data[Part->Length].RangeStart = RangeStart;
}

void SetPartArgumentRangeEnd(part_t* Part, int RangeEnd) {
  Part->Data[Part->Length].RangeEnd = RangeEnd;
}

void IncrementPartArguments(part_t* Part) {
  Part->Length++;

  if (Part->Length >= Part->Capacity) {
    Part->Capacity <<= 1;
    Part->Data = realloc(Part->Data, sizeof(argument_t) * Part->Capacity);
  }
}

void ClosePart(part_t* Part) {
  int i;
  CloseString(&Part->ChipName);

  for (i = 0 ; i < Part->Length ; i++) {
    CloseString(&Part->Data[i].Key);
    CloseString(&Part->Data[i].Value);
  }
  tFree(Part->Data);
}

void InitPartsArray(part_array_t* Parts) {
  Parts->Capacity = 4;
  Parts->Data = tMalloc(sizeof(part_t) * Parts->Capacity);
  Parts->Length = 0;
}

void AppendPartsArray(part_array_t* Parts, part_t Part) { 
  if (Parts->Length >= Parts->Capacity) {
    Parts->Capacity <<= 1;
    Parts->Data = realloc(Parts->Data, sizeof(part_t) * Parts->Capacity);
  }

  Parts->Data[Parts->Length++] = Part;
}

void ClosePartsArray(part_array_t* Parts) {
  int i;

  if (Parts->Capacity == 0) return;

  for (i = 0 ; i < Parts->Length ; i++) {
    ClosePart(&Parts->Data[i]);
  }
  tFree(Parts->Data);
}

static void CloseVariable(variable_t* Variable) {
  CloseString(&Variable->Name);
}

static void CloseVariableArray(variable_array_t* Variables) {
  int i;

  if (Variables->Capacity == 0) return;

  for (i = 0 ; i < Variables->Length ; i++) {
    CloseVariable(&Variables->Data[i]);
  }

  tFree(Variables->Data);
}

void NandChip(void);

void InitChips(void) {
  Chips.Capacity = 8;
  Chips.Data = tMalloc(sizeof(chip_t) * Chips.Capacity);
  Chips.Length = 0;

  NandChip();
}

static void CloseChip(chip_t* Chip) {
  CloseString(&Chip->Name);
  ClosePartsArray(&Chip->Parts);
  CloseVariableArray(&Chip->Inputs);
  CloseVariableArray(&Chip->Outputs);
  CloseVariableArray(&Chip->Temps);
  tFree(Chip->Buffer);
}

void CloseChips(void) {
  int i;
  for (i = 0 ; i < Chips.Length ; i++) {
    CloseChip(Chips.Data + i);
  }
  tFree(Chips.Data);
}

static int MaxOf(int a, int b) {
  if (a > b) return a;
  return b;
}

void AppendVariableArray(variable_array_t* Array, variable_t Var) {
  if (Array->Capacity == 0) {
    Array->Capacity = 2;
    Array->Data = tMalloc(sizeof(variable_t) * Array->Capacity);
  }

  if (Array->Length >= Array->Capacity) {
    Array->Capacity += MaxOf((Array->Capacity >> 1), 1);
    Array->Data = realloc(Array->Data, sizeof(variable_t) * Array->Capacity);
  }

  Array->Data[Array->Length++] = Var;
}

variable_array_t EmptyVariableArray(void) {
  return (variable_array_t){.Length = 0, .Capacity = 0, .Data = NULL};
}

int ChipExists(string_t Name) {
  int i;
  for (i = 0 ; i < Chips.Length ; i++) {
    if (StringEquals(Chips.Data[i].Name, Name)) return 1;
  }

  return 0;
}

chip_t* ChipFromName(string_t Name) {
  int i;
  for (i = 0 ; i < Chips.Length ; i++) {
    if (StringEquals(Chips.Data[i].Name, Name)) return &Chips.Data[i];
  }

  printf("Chip ");
  PrintString(Name, STR(""));
  printf(" not found!\n");

  return NULL;
}

static void PrintVariable(chip_t* Chip, variable_t* Var) {
  int i;

  for (i = 0 ; i < Var->Length ; i++) {
    printf("%x", GetVariableValue(Chip, Var->Name, Var->Length - 1 - i));
  }
}

void PrintVariableNames(chip_t* Chip) {
  int i;
  for (i = 0 ; i < Chip->Inputs.Length ; i++) {
    printf("| ");
    PrintString(Chip->Inputs.Data[i].Name, STR(""));
    printf(" ");
  }

  for (i = 0 ; i < Chip->Outputs.Length ; i++) {
    printf("| ");
    PrintString(Chip->Outputs.Data[i].Name, STR(""));
    printf(" ");
  }

  printf("|\n");
}

void PrintVariables(chip_t* Chip) {
  int i;
  for (i = 0 ; i < Chip->Inputs.Length ; i++) {
    printf("| ");
    PrintVariable(Chip, &Chip->Inputs.Data[i]);
    printf(" ");
  }

  for (i = 0 ; i < Chip->Outputs.Length ; i++) {
    printf("| ");
    PrintVariable(Chip, &Chip->Outputs.Data[i]);
  }

  printf(" |\n");
}

variable_t* GetVariable(string_t Name, string_t Key) {
  int i;
  chip_t* Chip;

  Chip = ChipFromName(Name);

  for (i = 0 ; i < Chip->Inputs.Length ; i++) {
    if (StringEquals(Chip->Inputs.Data[i].Name, Key)) return Chip->Inputs.Data + i;
  }

  for (i = 0 ; i < Chip->Outputs.Length ; i++) {
    if (StringEquals(Chip->Outputs.Data[i].Name, Key)) return Chip->Outputs.Data + i;
  }

  for (i = 0 ; i < Chip->Temps.Length ; i++) {
    if (StringEquals(Chip->Temps.Data[i].Name, Key)) return Chip->Temps.Data + i;
  }

  printf("Variable ");
  PrintString(Name, STR("."));
  PrintString(Key, STR(""));
  printf(" not found!\n");
  return NULL;
}

int GetVariableLength(chip_t* Chip, string_t Key) {
  variable_t* var;

  var = GetVariable(Chip->Name, Key);
  if (NULL == var) return -1;
  return var->Length;
}

unsigned char GetVariableValue(chip_t* Chip, string_t Key, int Index) {
  variable_t* Var;

  Var = GetVariable(Chip->Name, Key);
  if (NULL == Var) return -1;

  return Chip->Buffer[Var->BufferIndex + Index];
}

void SetVariableValue(chip_t* Chip, string_t Key, int Index, unsigned char Value) {
  variable_t* Var;

  Var = GetVariable(Chip->Name, Key);
  if (NULL == Var) return;

  Chip->Buffer[Var->BufferIndex + Index] = Value;
}

static void CopyChip(chip_t* Dest, chip_t* Src) {
  unsigned char* NewBuffer;

  *Dest = *Src;
  Dest->Buffer = tMalloc(Src->BufferSize);
}

static void FreeCopiedChip(chip_t* Chip) {
  tFree(Chip->Buffer); /* buffer is the only thing the copy owns, so the rest doesn't need to go */
}

static string_t GetMapping(part_t* Part, string_t Key) {
  int i;

  for (i = 0 ; i < Part->Length ; i++) {
    if (StringEquals(Part->Data[i].Key, Key)) return Part->Data[i].Value;
  }

  return STR("");
}

static argument_t* FindConnection(part_t* Part, string_t Key) {
  int i;

  for (i = 0 ; i < Part->Length ; i++) {
    if (StringEquals(Part->Data[i].Key, Key)) return &Part->Data[i];
  }

  return NULL;
}

static void FillBuffer(chip_t* ToChip, variable_t* To, chip_t* FromChip, variable_t* From, int ToStart, int FromStart, int Length) {
  int i;

  for (i = 0 ; i < Length ; i++) {
    ToChip->Buffer[To->BufferIndex + ToStart + i] = FromChip->Buffer[From->BufferIndex + FromStart + i];
  }
}

static void ProcessPart(chip_t* Chip, part_t* Part) {
  int i;

  chip_t TempChip;

  variable_t* ProcessVariable;
  argument_t* Connection;
  int ConnectionSize;

  CopyChip(&TempChip, ChipFromName(Part->ChipName));

  for (i = 0 ; i < TempChip.Inputs.Length ; i++) {
    Connection = FindConnection(Part, TempChip.Inputs.Data[i].Name);
    if (NULL == Connection) {
      printf("Unaccounted for input to chip!\n");
      return;
    }

    ProcessVariable = GetVariable(Chip->Name, Connection->Value);

    ConnectionSize = Connection->RangeEnd - Connection->RangeStart;
    if (0 == ConnectionSize) {
      ConnectionSize = ProcessVariable->Length;
    }

    if (ConnectionSize != TempChip.Inputs.Data[i].Length) {
      printf("Argument lengths do not match!\n");
      return;
    }

    FillBuffer(&TempChip, &TempChip.Inputs.Data[i],
               Chip, ProcessVariable,
               0, Connection->RangeStart, ConnectionSize);
  }

  TempChip.ProcessChip(&TempChip);

  for (i = 0 ; i < TempChip.Outputs.Length ; i++) {
    Connection = FindConnection(Part, TempChip.Outputs.Data[i].Name);
    if (NULL == Connection) {
      printf("Unaccounted for output to chip!\n");
      return;
    }

    ProcessVariable = GetVariable(Chip->Name, Connection->Value);

    ConnectionSize = Connection->RangeEnd - Connection->RangeStart;
    if (0 == ConnectionSize) {
      ConnectionSize = ProcessVariable->Length;
    }

    if (ConnectionSize != TempChip.Outputs.Data[i].Length) {
      printf("Argument lengths do not match!\n");
      return;
    }

    FillBuffer(Chip, ProcessVariable, 
              &TempChip, &TempChip.Outputs.Data[i],
              Connection->RangeStart, 0, ConnectionSize);
  }

  FreeCopiedChip(&TempChip);
}

void ProcessGenericChip(chip_t* Chip) {
  int i;

  for (i = 0 ; i < Chip->Parts.Length ; i++) 
    ProcessPart(Chip, &Chip->Parts.Data[i]);
}

void ProcessNandChip(chip_t* Chip) {
  Chip->Buffer[Chip->Outputs.Data[0].BufferIndex] =
    !(Chip->Buffer[Chip->Inputs.Data[0].BufferIndex] & Chip->Buffer[Chip->Inputs.Data[1].BufferIndex]);
}

void NewChip(string_t Name, variable_array_t Inputs, variable_array_t Outputs, variable_array_t Temps, part_array_t Parts) {
  chip_t* Chip;

  int BufferSize;
  int i;

  if (Chips.Length >= Chips.Capacity) {
    Chips.Capacity <<= 1;
    Chips.Data = realloc(Chips.Data, sizeof(chip_t) * Chips.Capacity);
  }

  BufferSize = 0;
  for (i = 0 ; i < Inputs.Length ; i++) {
    Inputs.Data[i].BufferIndex = BufferSize;
    BufferSize += Inputs.Data[i].Length;
  }
  for (i = 0 ; i < Outputs.Length ; i++) {
    Outputs.Data[i].BufferIndex = BufferSize;
    BufferSize += Outputs.Data[i].Length;
  }
  for (i = 0 ; i < Temps.Length ; i++) {
    Temps.Data[i].BufferIndex = BufferSize;
    BufferSize += Temps.Data[i].Length;
  }

  Chip = Chips.Data + Chips.Length;

  InitString(&Chip->Name, Name);

  Chip->Buffer = tMalloc(BufferSize);
  Chip->BufferSize = BufferSize;
  Chip->Inputs = Inputs;
  Chip->Outputs = Outputs;
  Chip->Temps = Temps;

  Chip->Parts = Parts;

  Chip->ProcessChip = ProcessGenericChip;

  Chips.Length++;
}

void NandChip(void) {
  variable_array_t Inputs, Outputs, Temps;
  part_array_t Parts;

  Inputs.Capacity = 2;
  Inputs.Length = 2;
  Inputs.Data = tMalloc(sizeof(variable_t) * Inputs.Length);

  Outputs.Capacity = 1;
  Outputs.Length = 1;
  Outputs.Data = tMalloc(sizeof(variable_t) * Outputs.Length);

  Temps = EmptyVariableArray();

  Parts.Length = 0;
  Parts.Capacity = 0;

  InitString(&Inputs.Data[0].Name, STR("a"));
  Inputs.Data[0].Length = 1;

  InitString(&Inputs.Data[1].Name, STR("b"));
  Inputs.Data[1].Length = 1;

  InitString(&Outputs.Data[0].Name, STR("out"));
  Outputs.Data[0].Length = 1;

  NewChip(STR("Nand"), Inputs, Outputs, Temps, Parts);
  Chips.Data[Chips.Length-1].ProcessChip = ProcessNandChip;
}
