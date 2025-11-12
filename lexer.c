#include "alloc.h"
#include "lexer.h"
#include "tAssert.h"

token_t* InitToken(token_t* Tail, _token_type Type) {
  token_t* NewToken;

  NewToken = tMalloc(sizeof(token_t));
  NewToken->Type = Type;
  InitStringLen(&NewToken->Symbol, 8);
  NewToken->Next = NULL;
  
  if (NULL != Tail) {
    Tail->Next = NewToken;
  }

  return NewToken;
}

void CloseTokenList(token_t* List) {
  token_t* Next = List;

  while (NULL != List) {
    Next = List->Next;
    
    CloseString(&List->Symbol);
    tFree(List);

    List = Next;
  }
}

void PrintToken(token_t* Token) {
  switch (Token->Type) {
    case UnknownToken:             printf("Unknown Token:            "); break;
    case CommentToken:             printf("Comment Token:            "); break;
    case ChipToken:                printf("Chip Token:               "); break;
    case InToken:                  printf("In Token:                 "); break;
    case OutToken:                 printf("Out Token:                "); break;
    case PartsToken:               printf("Parts Token:              "); break;
    case SymbolToken:              printf("Symbol Token:             "); break;
    case RangeToken:               printf("Range Token:              "); break;
    case OpenBracketToken:         printf("OpenBracket Token:        "); break;
    case CloseBracketToken:        printf("CloseBracket Token:       "); break;
    case OpenCurlyBracketToken:    printf("OpenCurlyBracket Token:   "); break;
    case CloseCurlyBracketToken:   printf("CloseCurlyBracket Token:  "); break;
    case OpenSquareBracketToken:   printf("OpenSquareBracket Token:  "); break;
    case CloseSquareBracketToken:  printf("CloseSquareBracket Token: "); break;
    case CommaToken:               printf("Comma Token:              "); break;
    case EqualToken:               printf("Equal Token:              "); break;
    case ColonToken:               printf("Colon Token:              "); break;
    case SemicolonToken:           printf("Semicolon Token:          "); break;
    case EndToken:                 printf("End Token:                "); break;
  }

  PrintString(Token->Symbol, STR("\n"));
}

void PrintTokenList(token_t* List) {
  while (NULL != List) {
    PrintToken(List);
    List = List->Next;
  }
}

/* return 1 if endline, 0 if not */
static int IsEndline(char c) {
  return (c == '\n');
}

/* return 1 if whitespace, 0 if not */
int IsWhitespace(char c) {
  if (c == ' ')  return 1;
  if (c == '\t') return 1;
  if (c == '\n') return 1;
  if (c == '\v') return 1;
  if (c == '\f') return 1;
  if (c == '\r') return 1;
  return 0;
}

static int AlphaNumeric(char c) {
  if (c >= 'a' && c <= 'z') return 1;
  if (c >= 'A' && c <= 'Z') return 1;
  if (c >= '0' && c <= '9') return 1;

  return 0;
}

token_t* Lexer(FILE* InputFile, char* Buffer, int BufferSize) {
  int LineNumber;

  int InputBufferLen;
  char* ReadPtr;

  token_t* CurrentToken;
  string_t TokenBuffer;

  token_t* TokenList;

  typedef enum {
    tNewToken,
    tCompleteToken,
    tWhitespace,
    tForwardSlash,
    tLineComment,
    tMultiLineComment,
    tTerminateMultiLineComment,
    tChipC,
    tChipH,
    tChipI,
    tInI,
    tOutO,
    tOutU,
    tPartsP,
    tPartsA,
    tPartsR,
    tPartsT,
    tDot,
    tString
  } tokeniser_t;

  tokeniser_t CurrentState;
  tokeniser_t NextState;

  CurrentToken = InitToken(NULL, UnknownToken);
  TokenList = CurrentToken;

  CurrentState = tNewToken;

  LineNumber = 1;

  do {
    InputBufferLen = fread(Buffer, sizeof(char), BufferSize, InputFile);
    if (0 == InputBufferLen) break;

    ReadPtr = Buffer;
    while (ReadPtr < Buffer + BufferSize) {
      if (*ReadPtr == '\0') break;

      switch (CurrentState) {
        case (tNewToken): {
          if (IsWhitespace(*ReadPtr)) {
            NextState = tWhitespace;
            ReadPtr++;
          }
          else if (*ReadPtr == '.') {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            NextState = tDot;
            ReadPtr++;
          }
          else if (*ReadPtr == '/') {
            NextState = tForwardSlash;
            ReadPtr++;
          }
          else if (*ReadPtr == 'C') {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            NextState = tChipC;
            ReadPtr++;
          }
          else if (*ReadPtr == 'I') {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            NextState = tInI;
            ReadPtr++;
          }
          else if (*ReadPtr == 'O') {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            NextState = tOutO;
            ReadPtr++;
          }
          else if (*ReadPtr == 'P') {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            NextState = tPartsP;
            ReadPtr++;
          }
          else if (*ReadPtr == '(') {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            CurrentToken->Type = OpenBracketToken;
            NextState = tCompleteToken;
            ReadPtr++;
          }
          else if (*ReadPtr == ')') {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            CurrentToken->Type = CloseBracketToken;
            NextState = tCompleteToken;
            ReadPtr++;
          }
          else if (*ReadPtr == '{') {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            CurrentToken->Type = OpenCurlyBracketToken;
            NextState = tCompleteToken;
            ReadPtr++;
          }
          else if (*ReadPtr == '}') {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            CurrentToken->Type = CloseCurlyBracketToken;
            NextState = tCompleteToken;
            ReadPtr++;
          }
          else if (*ReadPtr == '[') {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            CurrentToken->Type = OpenSquareBracketToken;
            NextState = tCompleteToken;
            ReadPtr++;
          }
          else if (*ReadPtr == ']') {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            CurrentToken->Type = CloseSquareBracketToken;
            NextState = tCompleteToken;
            ReadPtr++;
          }
          else if (*ReadPtr == ',') {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            CurrentToken->Type = CommaToken;
            NextState = tCompleteToken;
            ReadPtr++;
          }
          else if (*ReadPtr == '=') {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            CurrentToken->Type = EqualToken;
            NextState = tCompleteToken;
            ReadPtr++;
          }
          else if (*ReadPtr == ':') {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            CurrentToken->Type = ColonToken;
            NextState = tCompleteToken;
            ReadPtr++;
          }
          else if (*ReadPtr == ';') {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            CurrentToken->Type = SemicolonToken;
            NextState = tCompleteToken;
            ReadPtr++;
          }
          else if (AlphaNumeric(*ReadPtr)) {
            CurrentToken->Type = SymbolToken;
            NextState = tString;
          }
          else {
            tSyntaxError(Buffer, ReadPtr, LineNumber);
          }
        }; break;
        case (tCompleteToken): {
          CurrentToken = InitToken(CurrentToken, UnknownToken);
          NextState = tNewToken;
        }; break;
        case (tWhitespace): {
          if (IsWhitespace(*ReadPtr)) {
            NextState = tWhitespace;
            ReadPtr++;
          }
          else {
            NextState = tNewToken;
          }
        }; break;
        case (tForwardSlash): {
          if (*ReadPtr == '/') {
            CurrentToken->Type = CommentToken;
            NextState = tLineComment;
            ReadPtr++;
          }
          else if (*ReadPtr == '*') {
            CurrentToken->Type = CommentToken;
            NextState = tMultiLineComment;
            ReadPtr++;
          }
          else {
            tSyntaxError(Buffer, ReadPtr, LineNumber);
          }
        }; break;
        case (tLineComment): {
          if (IsEndline(*ReadPtr)) {
            CurrentToken = InitToken(CurrentToken, UnknownToken);
            NextState = tNewToken;
          }
          else {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
          }
          ReadPtr++;
        }; break;
        case (tMultiLineComment): {
          if ('*' == *ReadPtr) {
            NextState = tTerminateMultiLineComment;
          }
          else {
            NextState = tMultiLineComment;
          }
          AppendString(&CurrentToken->Symbol, *ReadPtr);
          ReadPtr++;
        }; break;
        case (tTerminateMultiLineComment): {
          if ('/' == *ReadPtr) {
            NextState = tCompleteToken;
          }
          else {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            NextState = tMultiLineComment;
          }
          ReadPtr++;
        }; break;
        case (tChipC): {
          AppendString(&CurrentToken->Symbol, *ReadPtr);
          if ('H' == *ReadPtr) {
            NextState = tChipH;
          }
          else {
            CurrentToken->Type = SymbolToken;
            NextState = tString;
          }
          ReadPtr++;
        }; break;
        case (tChipH): {
          AppendString(&CurrentToken->Symbol, *ReadPtr);
          if ('I' == *ReadPtr) {
            NextState = tChipI;
          }
          else {
            CurrentToken->Type = SymbolToken;
            NextState = tString;
          }
          ReadPtr++;
        }; break;
        case (tChipI): {
          AppendString(&CurrentToken->Symbol, *ReadPtr);
          if ('P' == *ReadPtr) {
            CurrentToken->Type = ChipToken;
            NextState = tCompleteToken;
          }
          else {
            CurrentToken->Type = SymbolToken;
            NextState = tString;
          }
          ReadPtr++;
        }; break;
        case (tInI): {
          AppendString(&CurrentToken->Symbol, *ReadPtr);
          if ('N' == *ReadPtr) {
            CurrentToken->Type = InToken;
            NextState = tCompleteToken;
          }
          else {
            CurrentToken->Type = SymbolToken;
            NextState = tString;
          }
          ReadPtr++;
        }; break;
        case (tOutO): {
          AppendString(&CurrentToken->Symbol, *ReadPtr);
          if ('U' == *ReadPtr) {
            NextState = tOutU;
          }
          else {
            CurrentToken->Type = SymbolToken;
            NextState = tString;
          }
          ReadPtr++;
        }; break;
        case (tOutU): {
          AppendString(&CurrentToken->Symbol, *ReadPtr);
          if ('T' == *ReadPtr) {
            CurrentToken->Type = OutToken;
            NextState = tCompleteToken;
          }
          else {
            CurrentToken->Type = SymbolToken;
            NextState = tString;
          }
          ReadPtr++;
        }; break;
        case (tPartsP): {
          AppendString(&CurrentToken->Symbol, *ReadPtr);
          if ('A' == *ReadPtr) {
            NextState = tPartsA;
          }
          else {
            CurrentToken->Type = SymbolToken;
            NextState = tString;
          }
          ReadPtr++;
        }; break;
        case (tPartsA): {
          AppendString(&CurrentToken->Symbol, *ReadPtr);
          if ('R' == *ReadPtr) {
            NextState = tPartsR;
          }
          else {
            CurrentToken->Type = SymbolToken;
            NextState = tString;
          }
          ReadPtr++;
        }; break;
        case (tPartsR): {
          AppendString(&CurrentToken->Symbol, *ReadPtr);
          if ('T' == *ReadPtr) {
            NextState = tPartsT;
          }
          else {
            CurrentToken->Type = SymbolToken;
            NextState = tString;
          }
          ReadPtr++;
        }; break;
        case (tPartsT): {
          AppendString(&CurrentToken->Symbol, *ReadPtr);
          if ('S' == *ReadPtr) {
            CurrentToken->Type = PartsToken;
            NextState = tCompleteToken;
          }
          else {
            CurrentToken->Type = SymbolToken;
            NextState = tString;
          }
          ReadPtr++;
        }; break;
        case (tDot): {
          if (*ReadPtr != '.') tSyntaxError(Buffer, ReadPtr, LineNumber);

          AppendString(&CurrentToken->Symbol, *ReadPtr);
          CurrentToken->Type = RangeToken;
          NextState = tCompleteToken;
          ReadPtr++;
        }; break;
        case (tString): {
          if (!AlphaNumeric(*ReadPtr)) {
            NextState = tCompleteToken;
          }
          else {
            AppendString(&CurrentToken->Symbol, *ReadPtr);
            NextState = tString;
            ReadPtr++;
          }
        }; break;
      }
      CurrentState = NextState;
    }
  } while (0 != InputBufferLen);

  if (CurrentToken->Type == UnknownToken && CurrentToken->Symbol.Length == 0) {
    CurrentToken->Type = EndToken;
  }

  return TokenList;
}

