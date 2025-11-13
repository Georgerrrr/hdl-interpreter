#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "str.h"
#include "chip.h"
#include "parse.h"

#define BUFFER_LENGTH 0x8000

typedef struct _expression {
  string_t Key;
  int MaxArgs;
  void (*EvalFunc)(string_t*, int);
} expression_t;

void LoadChips(char* Buffer, int argc, char** argv) {
  FILE* f;
  token_t* TokenList;

  chip_builder_array_t ChipArray;

  chip_t* Chip;

  int i;

  ChipArray = InitChipBuilderArray();

  Buffer = tMalloc(BUFFER_LENGTH);
  for (i = 1 ; i < argc ; i++) {
    memset(Buffer, 0, BUFFER_LENGTH);
    printf("Building chip %s\n", argv[i]);
    f = fopen(argv[i], "r");
    TokenList = Lexer(f, Buffer, BUFFER_LENGTH);
    AppendChipBuilderArray(&ChipArray, ProcessTokens(TokenList));
    CloseTokenList(TokenList);
    fclose(f);
  }

  tFree(Buffer);

  printf("Linking chips\n");
  ResolveParts(ChipArray);

  CloseChipBuilderArray(&ChipArray);
}

chip_t* CurrentChip = NULL;
_print_fmt PrintFormat = printDecimal;

expression_t NewExpression(string_t Key, int MaxArgs, void (*Func)(string_t*, int)) {
  expression_t out;

  out.Key = Key;
  out.MaxArgs = MaxArgs;
  out.EvalFunc = Func;

  return out;
}

static int CharToHex(char c) {
  if ('0' <= c && c <= '9') return c - 48;
  if ('a' <= c && c <= 'f') return c - 87;
  if ('A' <= c && c <= 'F') return c - 55;
  return 0;
}

static int StringToValue(string_t Arg) {
  int i, value;

  value = 0;

  PrintFormat = printDecimal;

  i = 0;
  if (StringStartsWith(Arg, STR("0x"))) {
    PrintFormat = printHex;
    i = 2;
  }
  else if (StringStartsWith(Arg, STR("0b"))) {
    PrintFormat = printBinary;
    i = 2;
  }

  for ( ; i < Arg.Length ; i++) {
    if (PrintFormat == printBinary) {
      value = value << 1;
      value = value + (Arg.Data[i] - 48);
    }
    else if (PrintFormat == printHex) {
      value = value << 4;
      value = value + CharToHex(Arg.Data[i]);
    }
    else {
      value = value * 10;
      value = value + (Arg.Data[i] - 48);
    }
  }

  return value;
}

void LoadCommand(string_t* Args, int ArgCount) {
  if (ArgCount != 1) return;

  CurrentChip = ChipFromName(Args[0]);
  if (CurrentChip == NULL) {
    printf("Invalid chip name!\n");
    return;
  }

  PrintVariableNames(CurrentChip);
}

void SetCommand(string_t* Args, int ArgCount) {
  int i, count, value;

  if (ArgCount != 2) return;

  count = GetVariableLength(CurrentChip, Args[0]);
  value = StringToValue(Args[1]);

  for (i = 0 ; i < count ; i++) {
    SetVariableValue(CurrentChip, Args[0], i, (value >> i) & 0x1);
  }
}

void EvalCommand(string_t* Args, int ArgCount) {
  if (ArgCount != 0) return;

  CurrentChip->ProcessChip(CurrentChip);
}

void OutputCommand(string_t* Args, int ArgCount) {
  if (ArgCount != 0) return;

  PrintVariables(CurrentChip, printDecimal); 
}


int CommandIndex(string_t Key, const expression_t Commands[4]) {
  int i;

  for (i = 0 ; i < 4 ; i++) {
    if (StringEquals(Key, Commands[i].Key)) return i;
  }

  return -1;
}

void ReadStdin(char* Buffer) {
  int InputBufferLen;
  char* ReadPtr;

  string_t CurrentString;

  const expression_t Commands[] = {
    NewExpression(STR("load"),   1, LoadCommand),
    NewExpression(STR("set"),    2, SetCommand),
    NewExpression(STR("eval"),   0, EvalCommand),
    NewExpression(STR("output"), 0, OutputCommand)
  };

  int CurrentCommand;
  string_t Args[2];
  int ArgLen;

  CurrentCommand = -1;
  ArgLen = 0;

  CurrentString.Length = 0;

  do {
    InputBufferLen = fread(Buffer, sizeof(char), BUFFER_LENGTH, stdin);
    if (0 == InputBufferLen) break;

    for (ReadPtr = Buffer ; ReadPtr < Buffer + InputBufferLen ; ReadPtr++) {
      if (0 == CurrentString.Length) {
        if (IsWhitespace(*ReadPtr)) continue;

        CurrentString.Data = ReadPtr;
        CurrentString.Length++;
        continue;
      }

      if (!IsWhitespace(*ReadPtr)) {
        CurrentString.Length++;
        continue;
      }

      if (-1 == CurrentCommand) {
        CurrentCommand = CommandIndex(CurrentString, Commands);
        if (-1 == CurrentCommand) {
          printf("?\n");
          return;
        }

        if (0 == Commands[CurrentCommand].MaxArgs) {
          Commands[CurrentCommand].EvalFunc(Args, ArgLen);
          CurrentCommand = -1;
          ArgLen = 0;
        }

        CurrentString.Length = 0;
        continue;
      }

      if (ArgLen < Commands[CurrentCommand].MaxArgs) {
        Args[ArgLen] = CurrentString;
        ArgLen++;
      }

      if (ArgLen >= Commands[CurrentCommand].MaxArgs) {
        Commands[CurrentCommand].EvalFunc(Args, ArgLen);
        CurrentCommand = -1;
        ArgLen = 0;
      }

      CurrentString.Length = 0;
    }
  } while (0 != InputBufferLen);
}

int main(int argc, char** argv) {
  char* Buffer;

  if (argc < 2) return 0;

  InitChips();

  Buffer = malloc(BUFFER_LENGTH);

  LoadChips(Buffer, argc, argv);

  ReadStdin(Buffer);

  free(Buffer);

  CloseChips();

  return 0;
}
