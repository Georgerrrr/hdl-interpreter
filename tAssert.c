#include <stdio.h>
#include <stdlib.h>

#include "tAssert.h"

void tAssert(int expression, const char* print) {
  if (expression) return;

  printf("%s\n", print);
  exit(1);
}

void tSyntaxError(char* Buffer, char* ErrorPtr, int LineNumber) {
  printf("Syntax error on line %d: \n  ", LineNumber);

  while (*ErrorPtr != '\n') {
    if (ErrorPtr == Buffer) break;
    ErrorPtr--;
  }

  do {
    if (*ErrorPtr == '\0') break;
    ErrorPtr++;
    printf("%c", *ErrorPtr);
  } while (*ErrorPtr != '\n');

  printf("\n");

  exit(1);
}

