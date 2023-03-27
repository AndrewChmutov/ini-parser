#pragma once
#include <stdio.h>

typedef struct Section {
    char*   name;
    int  size;
    char**  keys;
    char**  values;
} Section;


void countValues(FILE *file, int *sections, int *max_line_len);
int isNumeric(const char *str);
short isCorrectName(const char *str);
short isCorrectNameSection(const char *str);
short isCorrectKey(const char *str);
void freeSections(Section *sections, int n);
void copySectionName(char *secName, const char *line, int n);
char *find(Section *sections, char *argument, int size);