#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define CALL_FAILURE -1

typedef struct
{
    char **parameters;
    int argc;
} Instruction;

enum
{
    // UTILITY
    EMPTY        =    0,
    WAIT         =    1,
    LOG          =    2,
    
    // MATH
    ADD            =    10,
    SUBTRACT    =    11,
    MULTIPLY    =    12,
    RANDf        =    13,
    RANDi        =    14,

    // VARIABLES
    X            =    232,
    Y            =    233,
    PLX            =    234,
    PLY            =    235,
    AIM            =    236,

    F1            =    237,
    F2            =    238,
    F3            =    239,
    F4            =    240,
    F5            =    241,
    F6            =    242,
    F7            =    243,
    F8            =    244,
    F9            =    245,
    F10            =    245,

    I1            =    246,
    I2            =    247,
    I3            =    248,
    I4            =    249,
    I5            =    250,
    I6            =    251,
    I7            =    252,
    I8            =    253,
    I9            =    254,
    I10            =    255
};

int get_reg_id(const char *name)
{
    if (strcmp(name, "X") == 0) return X;
    if (strcmp(name, "Y") == 0) return Y;
    if (strcmp(name, "PLX") == 0) return PLX;
    if (strcmp(name, "PLY") == 0) return PLY;
    if (strcmp(name, "AIM") == 0) return AIM;
    
    if (name[0] == 'F') {
        int num = atoi(&name[1]);
        if (num >= 1 && num <= 10) return F1 + (num - 1);
    }
    if (name[0] == 'I') {
        int num = atoi(&name[1]);
        if (num >= 1 && num <= 10) return I1 + (num - 1);
    }
    return -1;
}

int strip(const char *input)
{
    char cleaned[256];
    int j = 0;
    for (int i = 0; input[i] != '\0' && j < 255; i++)
    {
        if (isdigit(input[i]) || input[i] == '-')
        {
            cleaned[j++] = input[i];
        }
    }
    cleaned[j] = '\0';
    return atoi(cleaned);
}

float stripf(const char *input)
{
    char cleaned[256];
    int j = 0;
    for (int i = 0; input[i] != '\0' && j < 255; i++)
    {
        if (isdigit(input[i]) || input[i] == '-' || input[i] == '.')
        {
            cleaned[j++] = input[i];
        }
    }
    cleaned[j] = '\0';
    return (float)atof(cleaned);
}

char *read_file(FILE *file, long *fsize)
{
    fseek(file, 0L, SEEK_END);
    *fsize = ftell(file);
    char *text = malloc((*fsize) + 1 * sizeof(char));
    fseek(file, 0L, SEEK_SET);
    fread(text, sizeof(char), (*fsize), file);
    text[*fsize] = '\0';
    return text;
}

void write8(uint8_t value, FILE *file)
{
    fwrite(&value, sizeof(uint8_t), 1, file);
}

void write32i(int32_t value, FILE *file)
{
    fwrite(&value, sizeof(int32_t), 1, file);
}

void write32f(float value, FILE *file)
{
    fwrite(&value, sizeof(float), 1, file);
}

Instruction *lexerize(char *text, int *instruction_count)
{
    int count = 0;
    for (int i = 0; text[i]; i++)
    {
        if (text[i] == ';') count++;
    }
    *instruction_count = count;
    if (count == 0) return NULL;
    Instruction *results = malloc(sizeof(Instruction) * count);
    int instr_idx = 0;
    int i = 0;
    while (text[i] != '\0' && instr_idx < count)
    {
        while (text[i] != '\0' && isspace(text[i])) i++;
        if (text[i] == '\0') break;
        int tokens_in_line = 0;
        int in_word = 0;
        int temp_i = i;
        while (text[temp_i] != '\0' && text[temp_i] != ';')
        {
            if (!isspace(text[temp_i]) && !in_word) { tokens_in_line++; in_word = 1; }
            else if (isspace(text[temp_i])) { in_word = 0; }
            temp_i++;
        }
        int line_end = temp_i;
        results[instr_idx].argc = tokens_in_line;
        results[instr_idx].parameters = malloc(sizeof(char*) * (tokens_in_line + 1));
        int token_idx = 0;
        while (i < line_end)
        {
            while (i < line_end && isspace(text[i])) i++;
            if (i >= line_end) break;
            int word_start = i;
            while (i < line_end && !isspace(text[i])) i++;
            int len = i - word_start;
            results[instr_idx].parameters[token_idx] = malloc(len + 1);
            memcpy(results[instr_idx].parameters[token_idx], &text[word_start], len);
            results[instr_idx].parameters[token_idx][len] = '\0';
            token_idx++;
        }
        results[instr_idx].parameters[token_idx] = NULL;
        instr_idx++;
        i = line_end;
        if (text[i] == ';') i++;
    }
    return results;
}

int main(int argc, char **argv)
{
    FILE *file = fopen("test.ecl", "rb");
    if (!file) { printf("Could not open test.ecl\n"); return 1; }
    long fsize;
    char *text = read_file(file, &fsize);
    fclose(file);

    int instruction_count;
    Instruction *instrs = lexerize(text, &instruction_count);
    FILE *compiled_bin = fopen("compiled_bin.ecl", "wb");
    if (!compiled_bin) { printf("Could not create output file\n"); return 1; }

    for (int i = 0; i < instruction_count; i++)
    {
        if (instrs[i].argc == 0) continue;
        char *op = instrs[i].parameters[0];

        uint8_t opcode = 0;
        if      (strcmp(op, "add") == 0)      opcode = ADD;
        else if (strcmp(op, "sub") == 0)      opcode = SUBTRACT;
        else if (strcmp(op, "mul") == 0)      opcode = MULTIPLY;
        else if (strcmp(op, "randf") == 0)    opcode = RANDf;
        else if (strcmp(op, "randi") == 0)    opcode = RANDi;

        if (opcode != 0 && instrs[i].argc >= 3)
        {
            int dest = get_reg_id(instrs[i].parameters[1]);
            write8(opcode, compiled_bin);

            int src_reg = get_reg_id(instrs[i].parameters[2]);
            
            if (src_reg != -1)
            {
                // TRYB: Rejestr -> Rejestr (Bit 7 dest = 0)
                write8(dest, compiled_bin);
                write8(src_reg, compiled_bin);
            }
            else
            {
                // TRYB: Rejestr -> Liczba (Bit 7 dest = 1)
                write8(dest | 0x80, compiled_bin);
                if (opcode == RANDi) write32i(strip(instrs[i].parameters[2]), compiled_bin);
                else write32f(stripf(instrs[i].parameters[2]), compiled_bin);
            }
            continue;
        }

        // LOG
        if (strcmp(op, "log") == 0 && instrs[i].argc == 2)
        {
            write8(LOG, compiled_bin);
            write8(get_reg_id(instrs[i].parameters[1]), compiled_bin);
        }
        // WAIT
        else if (strcmp(op, "wait") == 0 && instrs[i].argc == 2)
        {
            write8(WAIT, compiled_bin);
            write32i(strip(instrs[i].parameters[1]), compiled_bin);
        }
        else
        {
            printf("incorrect instruction: %s\n", op);
        }
    }

    fclose(compiled_bin);
    return 0;
}
