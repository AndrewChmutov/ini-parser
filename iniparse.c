#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "iniparse.h"


// count amount of sections[0].nd maximum length of a single line
void countValues(FILE *file, int *sections, int *max_line_len) {
    char tempChar;
    int temp_line_len;
    
    // first cycle: each interation - one line
    do {
        tempChar = (char)fgetc(file);
        temp_line_len = 1;

        // count sections
        if (tempChar == '[') {
            (*sections)++;
        }

        // skip comments
        if (tempChar == ';')
            while (tempChar != '\n' && tempChar != EOF) {
                tempChar = (char)fgetc(file);
            }
        // count line length
        else {
            while (tempChar != '\n' && tempChar != EOF) {
                temp_line_len++;
                tempChar = (char)fgetc(file);
            }
        }

        // count max line length
        if (*max_line_len < temp_line_len)
            *max_line_len = temp_line_len;
    
    }
    while (tempChar != EOF);

    rewind(file);
}


int isNumeric(const char *str) {
    int n;
    return sscanf(str, "%d", &n) == 1 && !isspace(*str);
}


short isCorrectName(const char *str) {
    int n = strlen(str);

    for (int i = 0; i < n; i++) {
        if (!isalnum(str[i]) && str[i] != '-') {
            return 0;
        }
    }

    return 1;
}

short isCorrectNameSection(const char *str) {
    int n = strlen(str);
    if (n < 3)
        return 0;

    if (str[0] != '[' || str[n - 2] != ']')
        return 0;

    for (int i = 1; i < n - 2; i++)
        if (!isalnum(str[i]) && str[i] != '-')
            return 0;
        
    
    return 1;
}

short isCorrectKey(const char *str) {
    int n = strlen(str);

    for (int i = 1; i < n - 1; i++)
        if (!isalnum(str[i]) && str[i] != '-')
            return 0;

    return 1;
}

void gotoNextLine(FILE *file) {
    char tempChar = (char)fgetc(file);
    if (tempChar == '\n') {
        fseek(file, -1, SEEK_CUR);
        return;
    }
    while (tempChar != '\n' && tempChar != EOF) {
        tempChar = (char)fgetc(file);
    }
}

void freeSections(Section *sections, int n) {
    for (int i = 0; i < n; i++) {
        if (sections[i].size != 0) {
            for (int j = 0; j < sections[i].size; j++) {
                free(sections[i].keys[j]);
                free(sections[i].values[j]);
            }
            free(sections[i].keys);
            free(sections[i].values);
            free(sections[i].name);
        }
    }

    free(sections);
}

void copySectionName(char *secName, const char *line, int n) {
    for (int i = 1; i < n - 2; i++)
        secName[i - 1] = line[i];
    strncpy(secName, line + 1, n - 3);
    printf("%s\n", line);
    printf("%s\n", secName);
    // secName[n - 3] = '\0';
}

int findSection(Section *sections, const char *CLIargument, int size) {
    char *query = malloc((strlen(CLIargument) + 1) * sizeof(char));
    strncpy(query, CLIargument, strlen(CLIargument));
    query[strlen(CLIargument)] = '\0';
    char *token;
    token = strtok(query, ".");

    int choice = -1;
    for (int i = 0; i < size; i++) {
        if (!strcmp(sections[i].name, token)) {
            choice = i;
            break;
        }
    }

    if (choice < 0)
        printf("Failed to find section [%s]\n", token);

    free(query);

    return choice;
}