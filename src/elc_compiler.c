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
	EMPTY		=	0,
	WAIT		=	1,
	LOG			=	2,
	
	// MATH
	ADD			=	10,
	SUBTRACT	=	11,
	MULTIPLY	=	12,
	RANDf		=	13,
	RANDi		=	14,


	// VARIABLES

	X			=	232,
	Y			=	233,

	PLX			=	234,
	PLY			=	235,
	AIM			=	236, /*Angle of an emmiter subjective to the position of PLX and PLY*/

	F1			=	237,
	F2			=	238,
	F3			=	239,
	F4			=	240,
	F5			=	241,
	F6			=	242,
	F7			=	243,
	F8			=	244,
	F9			=	245,
	F10			=	245,

	I1			=	246,
	I2			=	247,
	I3			=	248,
	I4			=	249,
	I5			=	250,
	I6			=	251,
	I7			=	252,
	I8			=	253,
	I9			=	254,
	I10			=	255
};

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
		if (text[i] == ';')
		{
			count++;
		}
	}
	*instruction_count = count;

	if (count == 0)
	{
		return NULL;
	}

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
			if (!isspace(text[temp_i]) && !in_word)
			{
				tokens_in_line++;
				in_word = 1;
			}
			else if (isspace(text[temp_i]))
			{
				in_word = 0;
			}
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
    float f[256];
    memset(f, 0, sizeof(f));

    FILE *file = fopen("test.ecl", "rb");
    if (!file)
    {
        printf("Could not open test.ecl\n");
        return 1;
    }

    long fsize;
    char *text = read_file(file, &fsize);
    fclose(file);

    int instruction_count;
    Instruction *instrs = lexerize(text, &instruction_count);

    FILE *compiled_bin = fopen("compiled_bin.ecl", "wb");
    if (!compiled_bin)
    {
        printf("Could not create output file\n");
        return 1;
    }

    for (int i = 0; i < instruction_count; i++)
    {
        if (instrs[i].argc == 0) continue;

        char *op = instrs[i].parameters[0];

		// ADD
        if (strcmp(op, "add") == 0 && instrs[i].argc >= 3)
        {
            int dest = strip(instrs[i].parameters[1]);
            int src = strip(instrs[i].parameters[2]);

            write8(ADD, compiled_bin);

            if (instrs[i].argc == 4)
            {
                float val = stripf(instrs[i].parameters[3]);
                f[dest] = f[src] + val;

                write8(dest | 0x80, compiled_bin)
                write8(src, compiled_bin);
                write32f(val, compiled_bin);
            }
            else
            {
                f[dest] = f[dest] + f[src];

                write8(dest, compiled_bin);
                write8(src, compiled_bin);
            }
        }

        // MOV
        else if (strcmp(op, "mov") == 0 && instrs[i].argc == 3)
        {
            int dest = strip(instrs[i].parameters[1]);
            f[dest] = stripf(instrs[i].parameters[2]);
        }

        // LOG
        else if (strcmp(op, "log") == 0 && instrs[i].argc == 2)
        {
            int index = strip(instrs[i].parameters[1]);
            if (index >= 0 && index < 256)
            {
                printf("Log F%d: %f\n", index, f[index]);
            }
        }

        else
        {
            printf("incorrect instruction: %s\n", op);
        }
    }

    fclose(compiled_bin);
    return 0;
}
