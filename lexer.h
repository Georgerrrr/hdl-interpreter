#ifndef LEXER_H 
#define LEXER_H 

#include <stdio.h>

#include "str.h"

typedef enum {
  UnknownToken,
  CommentToken,
  ChipToken,
  InToken,
  OutToken,
  PartsToken,
  SymbolToken,
  RangeToken,
  OpenBracketToken,
  CloseBracketToken,
  OpenCurlyBracketToken,
  CloseCurlyBracketToken,
  OpenSquareBracketToken,
  CloseSquareBracketToken,
  CommaToken,
  EqualToken,
  ColonToken,
  SemicolonToken,
  EndToken
} _token_type;

typedef struct _token {
  _token_type Type;
  string_t Symbol;
  struct _token* Next;
} token_t;

int IsWhitespace(char c);

token_t* InitToken(token_t* Tail, _token_type Type);
void CloseTokenList(token_t* List);
void PrintToken(token_t* Token);
void PrintTokenList(token_t* List);
token_t* Lexer(FILE* InputFile, char* Buffer, int BufferSize);

#endif // LEXER_H
