#ifndef TASSERT_H
#define TASSERT_H 

void tAssert(int expression, const char* print);

void tSyntaxError(char* Buffer, char* ErrorPtr, int LineNumber);

#endif // TASSERT_H
