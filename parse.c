#include <stdlib.h>

#include "alloc.h"
#include "chip.h"
#include "parse.h"
#include "tAssert.h"

chip_builder_array_t InitChipBuilderArray(void) {
  chip_builder_array_t Out;
  Out.Capacity = 4;
  Out.Length = 0;
  Out.Data = tMalloc(sizeof(chip_builder_t) * Out.Capacity);
  return Out;
}

void CloseChipBuilderArray(chip_builder_array_t* Array) {
  int i;

  for (i = 0 ; i < Array->Length ; i++) {
    /* all the rest of the memory has been given to the actual chip to deal with.
     * is this solution stupid and not well thought through...
     * ...                                   maybe
     */
    CloseString(&Array->Data[i].ChipName);
    tFree(Array->Data[i].ResolvedParts.Data);
  }

  tFree(Array->Data);
}

void AppendChipBuilderArray(chip_builder_array_t* Array, chip_builder_t ChipBuilder) {
  if (Array->Length >= Array->Capacity) {
    Array->Capacity <<= 1;
    Array->Data = realloc(Array->Data, sizeof(chip_builder_t) * Array->Capacity);
  }

  Array->Data[Array->Length++] = ChipBuilder;
}

static void BalanceTokens(token_t* TokenList) {
  int BracketCount;
  int CurlyCount;
  int SquareCount;

  token_t* Current;

  BracketCount = CurlyCount = SquareCount = 0;

  Current = TokenList;
  while (Current != NULL) {
    if (Current->Type == OpenBracketToken) BracketCount++;
    if (Current->Type == CloseBracketToken) BracketCount--;
    if (Current->Type == OpenCurlyBracketToken) CurlyCount++;
    if (Current->Type == CloseCurlyBracketToken) CurlyCount--;
    if (Current->Type == OpenSquareBracketToken) SquareCount++;
    if (Current->Type == CloseSquareBracketToken) SquareCount--;

    Current = Current->Next;
  }

  tAssert(BracketCount == 0, "( symbol not balanced!");
  tAssert(CurlyCount   == 0, "{ symbol not balanced!");
  tAssert(SquareCount  == 0, "[ symbol not balanced!");
}

static int VarInChip(chip_builder_t* Chip, string_t Key) {
  int i;
  for (i = 0 ; i < Chip->Inputs.Length ; i++) 
    if (StringEquals(Chip->Inputs.Data[i].Name, Key)) return 1;

  for (i = 0 ; i < Chip->Outputs.Length ; i++) 
    if (StringEquals(Chip->Outputs.Data[i].Name, Key)) return 1;

  for (i = 0 ; i < Chip->Temps.Length ; i++) 
    if (StringEquals(Chip->Temps.Data[i].Name, Key)) return 1;

  return 0;
}

chip_builder_t ProcessTokens(token_t* TokenList) {
  chip_builder_t Out;

  part_t NewPart;
  variable_t NewVariable;

  token_t* CurrentToken;

  int i, j;

  typedef enum {
    pAny,
    pChipName,
    pChipDef,
    pChipPostDef,
    pChipIn,
    pChipInVariable,
    pChipInVariableSize,
    pChipInVariableSizeEnd,
    pChipInVariableSizeComplete,
    pChipPostIn,
    pChipOut,
    pChipOutVariable,
    pChipOutVariableSize,
    pChipOutVariableSizeEnd,
    pChipOutVariableSizeComplete,
    pChipPostOut,
    pChipParts,
    pChipInternals,
    pChipBeginPart,
    pPartArguments,
    pArgumentEquals,
    pArgumentValue,
    pArgumentEnd,
    pArgumentSubscript,
    pArgumentSubscriptRange,
    pArgumentSubscriptRangeEnd,
    pArgumentEndWithSubscript,
    pPartEnd
  } parser_t;

  parser_t Current = pAny;
  parser_t Next = pAny;

  BalanceTokens(TokenList);

  Out.Inputs = EmptyVariableArray();
  Out.Outputs = EmptyVariableArray();
  Out.Temps = EmptyVariableArray();

  InitPartsArray(&Out.Parts);

  for (CurrentToken = TokenList ; CurrentToken != NULL ; CurrentToken = CurrentToken->Next) {
    tAssert(CurrentToken->Type != UnknownToken, "Unknown token!");

    if (CurrentToken->Type == CommentToken) continue;

    switch (Current) {
      case pAny: {
        if (CurrentToken->Type == ChipToken) {
          Next = pChipName;
        }
      }; break;
      case pChipName: {
        tAssert(CurrentToken->Type == SymbolToken, "Expecting chip name!");
        InitString(&Out.ChipName, CurrentToken->Symbol);
        Next = pChipDef;
      }; break;
      case pChipDef: {
        tAssert(CurrentToken->Type == OpenCurlyBracketToken, "Expecting chip {!");
        Next = pChipPostDef;
      }; break;
      case pChipPostDef: {
        tAssert(CurrentToken->Type == InToken, "IN must follow chip definition!");
        Next = pChipIn;
      }; break;
      case pChipIn: {
        if (CurrentToken->Type == SymbolToken) {
          InitString(&NewVariable.Name, CurrentToken->Symbol);
          NewVariable.Length = 1;
          Next = pChipInVariable;
        }
        else {
          tAssert(0, "No variable name given for input!");
        }
      }; break;
      case pChipInVariable: {
        if (CurrentToken->Type == CommaToken) {
          AppendVariableArray(&Out.Inputs, NewVariable);
          Next = pChipIn;
        }
        else if (CurrentToken->Type == SemicolonToken) {
          AppendVariableArray(&Out.Inputs, NewVariable);
          Next = pChipPostIn;
        }
        else if (CurrentToken->Type == OpenSquareBracketToken) {
          Next = pChipInVariableSize;
        }
        else {
          tAssert(0, "IN variable expression is invalid!");
        }
      }; break;
      case pChipInVariableSize: {
        tAssert(CurrentToken->Type == SymbolToken, "IN variable size is invalid!");
        tAssert(StringIsInt(CurrentToken->Symbol), "IN variable size is invalid!");
        NewVariable.Length = StringToInt(CurrentToken->Symbol);
        Next = pChipInVariableSizeEnd;
      }; break;
      case pChipInVariableSizeEnd: {
        tAssert(CurrentToken->Type == CloseSquareBracketToken, "IN variable size is invalid!");
        Next = pChipInVariableSizeComplete;
      }; break;
      case pChipInVariableSizeComplete: {
        if (CurrentToken->Type == CommaToken) {
          AppendVariableArray(&Out.Inputs, NewVariable);
          Next = pChipIn;
        }
        else if (CurrentToken->Type == SemicolonToken) {
          AppendVariableArray(&Out.Inputs, NewVariable);
          Next = pChipPostIn;
        }
        else {
          tAssert(0, "IN variable expression is invalid!");
        }
      }; break;
      case pChipPostIn: {
        tAssert(CurrentToken->Type == OutToken, "OUT must follow IN definitions!");
        Next = pChipOut;
      }; break;
      case pChipOut: {
        if (CurrentToken->Type == SymbolToken) {
          InitString(&NewVariable.Name, CurrentToken->Symbol);
          NewVariable.Length = 1;
          Next = pChipOutVariable;
        }
        else {
          tAssert(0, "No variable name given for input!");
        }
      }; break;
      case pChipOutVariable: {
        if (CurrentToken->Type == CommaToken) {
          AppendVariableArray(&Out.Outputs, NewVariable);
          Next = pChipOut;
        }
        else if (CurrentToken->Type == SemicolonToken) {
          AppendVariableArray(&Out.Outputs, NewVariable);
          Next = pChipPostOut;
        }
        else if (CurrentToken->Type == OpenSquareBracketToken) {
          Next = pChipOutVariableSize;
        }
        else {
          tAssert(0, "OUT variable expression is invalid!");
        }
      }; break;
      case pChipOutVariableSize: {
        tAssert(CurrentToken->Type == SymbolToken, "OUT variable size is invalid!");
        tAssert(StringIsInt(CurrentToken->Symbol), "OUT variable size is invalid!");
        NewVariable.Length = StringToInt(CurrentToken->Symbol);
        Next = pChipOutVariableSizeEnd;
      }; break;
      case pChipOutVariableSizeEnd: {
        tAssert(CurrentToken->Type == CloseSquareBracketToken, "OUT variable size is invalid!");
        Next = pChipOutVariableSizeComplete;
      }; break;
      case pChipOutVariableSizeComplete: {
        if (CurrentToken->Type == CommaToken) {
          AppendVariableArray(&Out.Outputs, NewVariable);
          Next = pChipOut;
        }
        else if (CurrentToken->Type == SemicolonToken) {
          AppendVariableArray(&Out.Outputs, NewVariable);
          Next = pChipPostOut;
        }
        else {
          tAssert(0, "OUT variable expression is invalid!");
        }
      }; break;
      case pChipPostOut: {
        tAssert(CurrentToken->Type == PartsToken, "PARTS must follow OUT definitions!");
        Next = pChipParts;
      }; break;
      case pChipParts: {
        tAssert(CurrentToken->Type == ColonToken, "PARTS must be followed by ':'");
        Next = pChipInternals;
      }; break;
      case pChipInternals: {
        if (CurrentToken->Type == SymbolToken) {
          InitPart(&NewPart, CurrentToken->Symbol);
          Next = pChipBeginPart;
        }
        else if (CurrentToken->Type == CloseCurlyBracketToken) {
          Next = pAny;
        }
        else {
          tAssert(0, "Invalid name given for part type!");
        }
      }; break;
      case pChipBeginPart: {
        tAssert(CurrentToken->Type == OpenBracketToken, "Part name must be followed by brackets!");
        Next = pPartArguments;
      }; break;
      case pPartArguments: {
        tAssert(CurrentToken->Type == SymbolToken, "Invalid part argument!");
        SetPartArgumentKey(&NewPart, CurrentToken->Symbol);
        Next = pArgumentEquals;
      }; break;
      case pArgumentEquals: {
        tAssert(CurrentToken->Type == EqualToken, "Argument must be followed by '='");
        Next = pArgumentValue;
      }; break;
      case pArgumentValue: {
        tAssert(CurrentToken->Type == SymbolToken, "Invalid part argument value!");
        SetPartArgumentValue(&NewPart, CurrentToken->Symbol);
        Next = pArgumentEnd;
      }; break;
      case pArgumentEnd: {
        if (CurrentToken->Type == CommaToken) {
          SetPartArgumentRangeStart(&NewPart, 0);
          SetPartArgumentRangeEnd(&NewPart, 0);
          IncrementPartArguments(&NewPart);
          Next = pPartArguments;
        }
        else if (CurrentToken->Type == CloseBracketToken) {
          SetPartArgumentRangeStart(&NewPart, 0);
          SetPartArgumentRangeEnd(&NewPart, 0);
          IncrementPartArguments(&NewPart);
          Next = pPartEnd;
        }
        else if (CurrentToken->Type == OpenSquareBracketToken) {
          Next = pArgumentSubscript;
        }
        else {
          tAssert(0, "Invalid part argument end!");
        }
      }; break;
      case pArgumentSubscript: {
        int x;
        tAssert(CurrentToken->Type == SymbolToken, "Argument subscript not valid!");
        tAssert(StringIsInt(CurrentToken->Symbol), "Argument subscript must be integer!");
        x = StringToInt(CurrentToken->Symbol);
        SetPartArgumentRangeStart(&NewPart, x);
        SetPartArgumentRangeEnd(&NewPart, x+1);
        Next = pArgumentSubscriptRange;
      }; break;
      case pArgumentSubscriptRange: {
        if (CurrentToken->Type == CloseSquareBracketToken) {
          Next = pArgumentEndWithSubscript;
        }
        else if (CurrentToken->Type == RangeToken) {
          Next = pArgumentSubscriptRangeEnd;
        }
        else {
          tAssert(0, "Invalid subscript range!");
        }
      }; break;
      case pArgumentSubscriptRangeEnd: {
        tAssert(CurrentToken->Type == SymbolToken, "Argument subscript not valid!");
        tAssert(StringIsInt(CurrentToken->Symbol), "Argument subscript must be integer!");
        SetPartArgumentRangeEnd(&NewPart, StringToInt(CurrentToken->Symbol));
        Next = pArgumentEndWithSubscript;
      }; break;
      case pArgumentEndWithSubscript: {
        if (CurrentToken->Type == CommaToken) {
          IncrementPartArguments(&NewPart);
          Next = pPartArguments;
        }
        else if (CurrentToken->Type == CloseBracketToken) {
          IncrementPartArguments(&NewPart);
          Next = pPartEnd;
        }
        else {
          tAssert(0, "Invalid part argument end!");
        }
      }; break;
      case pPartEnd: {
        tAssert(CurrentToken->Type == SemicolonToken, "Part must be followed by ';'!");
        AppendPartsArray(&Out.Parts, NewPart);
        Next = pChipInternals;
      }; break; 
    }

    Current = Next;
  }

  Out.ResolvedParts.Length = Out.Parts.Length;
  Out.ResolvedParts.Data = tMalloc(sizeof(int) * Out.Parts.Length);
  memset(Out.ResolvedParts.Data, 0, sizeof(int) * Out.Parts.Length);

  return Out;
}

static void AddTempsFromPart(chip_builder_t* Chip, part_t* Part) {
  int i;
  variable_t NewVariable;

  for (i = 0 ; i < Part->Length ; i++) {
    if (VarInChip(Chip, Part->Data[i].Value)) continue;

    InitString(&NewVariable.Name, Part->Data[i].Value);
    NewVariable.Length = GetVariable(Part->ChipName, Part->Data[i].Key)->Length;
    AppendVariableArray(&Chip->Temps, NewVariable);
  }
}

static int IsChipResolved(chip_builder_t* Chip) {
  int i;

  for (i = 0 ; i < Chip->Parts.Length ; i++) {
    if (!Chip->ResolvedParts.Data[i]) return 0;
  }

  return 1;
}

static void ResolveChip(chip_builder_t* Chip) {
  int i;

  for (i = 0 ; i < Chip->Parts.Length ; i++) {
    if (Chip->ResolvedParts.Data[i]) continue;
    if (!ChipExists(Chip->Parts.Data[i].ChipName)) continue;

    AddTempsFromPart(Chip, &Chip->Parts.Data[i]);

    Chip->ResolvedParts.Data[i] = 1;
  }
}

void ResolveParts(chip_builder_array_t ChipBuilderArray) {
  int PreviousCount;
  int UnresolvedCount;

  int i;

  UnresolvedCount = 0;

  do {
    PreviousCount = UnresolvedCount;
    UnresolvedCount = 0;
    for (i = 0 ; i < ChipBuilderArray.Length ; i++) {
      if (IsChipResolved(&ChipBuilderArray.Data[i])) continue;

      UnresolvedCount++;
      ResolveChip(&ChipBuilderArray.Data[i]);
      if (!IsChipResolved(&ChipBuilderArray.Data[i])) continue;

      NewChip(
          ChipBuilderArray.Data[i].ChipName,
          ChipBuilderArray.Data[i].Inputs,
          ChipBuilderArray.Data[i].Outputs,
          ChipBuilderArray.Data[i].Temps,
          ChipBuilderArray.Data[i].Parts
          );
    }

    tAssert(PreviousCount != UnresolvedCount || PreviousCount == 0 || UnresolvedCount == 0, "Unable to resolve chips!");
  } while (UnresolvedCount > 0);
}
