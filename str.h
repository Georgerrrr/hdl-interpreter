#ifndef STR_H
#define STR_H

#include <string.h>

#define STR(x) ((string_t){.Data = x, .Length = strlen(x), .Capacity = strlen(x)})

typedef struct {
  char* Data;
  unsigned int Length;
  unsigned int Capacity;
} string_t;

void InitString(string_t* NewString, string_t In); 
void InitStringLen(string_t* NewString, int Size); 
void AppendString(string_t* String, char a); 
int StringEquals(string_t String, string_t Other);
int StringStartsWith(string_t String, string_t Other);
void CloseString(string_t* String); 
void PrintString(string_t String, string_t End); 
int StringIsInt(string_t String);
int StringToInt(string_t String);

#endif // STR_H
