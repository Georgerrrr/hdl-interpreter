#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "alloc.h"
#include "str.h"

void InitString(string_t* NewString, string_t In) {
  NewString->Capacity = In.Length;
  NewString->Length = NewString->Capacity;
  NewString->Data = tMalloc(sizeof(char) * (NewString->Capacity + 1));
  memset(NewString->Data, 0, sizeof(char) * (NewString->Capacity + 1));
  strcpy(NewString->Data, In.Data);
}

void InitStringLen(string_t* NewString, int Size) {
  NewString->Capacity = Size;
  NewString->Length = 0;
  NewString->Data = tMalloc(sizeof(char) * Size);
}

void AppendString(string_t* String, char a) {
  if (String->Length >= String->Capacity) {
    String->Capacity <<= 1;
    String->Data = realloc(String->Data, String->Capacity);
  }

  String->Data[String->Length++] = a;
}

int StringEquals(string_t String, string_t Other) {
  int i;
  char* si;
  const char* di;
  if (String.Length != Other.Length) return 0;

  si = String.Data;
  di = Other.Data;
  for (i = 0 ; i < String.Length ; i++, si++, di++) {
    if (*si != *di) return 0;
  }

  return 1;
}

void CloseString(string_t* String) {
  tFree(String->Data);
  memset(String, 0, sizeof(string_t));
}

void PrintString(string_t String, string_t End) {
  int i;
  for (i = 0 ; i < String.Length ; i++) {
    printf("%c", String.Data[i]);
  }
  for (i = 0 ; i < End.Length ; i++) {
    printf("%c", End.Data[i]);
  }
}

int StringIsInt(string_t String) {
  int i;

  for (i = 0 ; i < String.Length ; i++) {
    if (String.Data[i] < '0') return 0;
    if (String.Data[i] > '9') return 0;
  }

  return 1;
}

int StringToInt(string_t String) {
  int output, i;

  output = 0;

  for (i = 0 ; i < String.Length ; i++) {
    output = output * 10;
    output += (int)(String.Data[i] - 48);
  }

  return output;
}
