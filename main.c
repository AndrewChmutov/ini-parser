#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// #define DEBUG

//Structure for parsing sections
typedef struct Section {
    char*   name;
    int  size;
    char**  keys;
    char**  values;
} Section;


void countValues(FILE *file, int *sections, int *max_line_len);
int isNumeric(const char *str);
short isCorrectName(const char *str);
void gotoNextLine(FILE *file);
short isCorrectNameSection(const char *str);
short isCorrectKey(const char *str);
void freeSections(Section *sections, int n);
void copySectionName(char *secName, const char *line, int n);


int main(int argc, char **argv) {
    // open file
    FILE* ptrFile;
    ptrFile = fopen("test.ini", "r");

    // exit if no file with such name
    if (ptrFile == NULL)
        exit(EXIT_FAILURE);

    // count the maximum sizes
    int sections_count  = 0;
    int max_line_len    = 0;
    countValues(ptrFile, &sections_count, &max_line_len);

    #ifdef DEBUG
    printf("%d\n", max_line_len);
    printf("%d\n", sections_count);
    #endif

    // allocate memory for sections
    Section* sections = malloc(sizeof(Section) * sections_count);
    for (int i = 0; i < sections_count; i++)    {   sections[i].size = 0;   }
    char* line = malloc(sizeof(char) * max_line_len);

    // iterate through the sections
    // check each line, look for a section
    // parse all the data into structures
    short isValid = 1;
    int section_i = 0;
    int n = 0;
    long startSection = 0;
    int attributes_count = 0;
    char *key;
    char *value;


    while (fgets(line, max_line_len + 1, ptrFile) != NULL) {
        // if there is a section name
        if (line[0] != '[') {
            // skip the commented line
            gotoNextLine(ptrFile);

            // go to the next line
            continue;
        }

        // skip section if the name is wrong or no closing bracket
        if (!isCorrectNameSection(line)) {
            isValid = 0;
            printf("InvalidNameException in section: %s\n", line);
            continue;
        }

        // copy section name
        int n = strlen(line);
        sections[section_i].name = malloc(sizeof(char) * (n - 2));
        strncpy(sections[section_i].name, line + 1, n - 2);
        sections[section_i].name[n - 3] = '\0';

        startSection = ftell(ptrFile);
        attributes_count = 0;

        // fill all the attributes
        while (fgets(line, max_line_len + 1, ptrFile) != NULL) {
            if (line[0] == '[')
                break;

            if (line[0] == '\n')
                continue;

            if (line[0] == ';') {
                gotoNextLine(ptrFile);
                continue;
            }
            key = malloc(sizeof(char) * strlen(line));
            value = malloc(sizeof(char) * strlen(line));
            sscanf(line, "%s = %s", key, value);
            if (!isCorrectKey(key)) {
                printf("InvalidKeyException: %s\n", key);
                free(key);
                free(value);
                continue;
            }
            free(key);
            free(value);
            
            attributes_count++;
        }

        sections[section_i].size = attributes_count;
        sections[section_i].keys = malloc(sizeof(char*) * attributes_count);
        sections[section_i].values = malloc(sizeof(char*) * attributes_count);

        fseek(ptrFile, startSection, SEEK_SET);
        attributes_count = 0;

        // fill section
        while (fgets(line, max_line_len + 1, ptrFile) != NULL) {
            if (line[0] == '[')
                break;

            if (line[0] == '\n')
                continue;

            if (line[0] == ';') {
                gotoNextLine(ptrFile);
                continue;
            }


            key = malloc(sizeof(char) * strlen(line));
            value = malloc(sizeof(char) * strlen(line));
            sscanf(line, "%s = %s", key, value);
            if (!isCorrectKey(key)) {
                free(key);
                free(value);
                continue;
            }

            sections[section_i].keys[attributes_count] = key;
            sections[section_i].values[attributes_count] = value;

            //sscanf(line, "%s = %s", sections[section_i].keys[attributes_count], sections[section_i].values[attributes_count]);
            //printf("%s", sections[section_i].values[attributes_count]);

            attributes_count++;
        }

        section_i++;

        n = strlen(line);
        if (line[n - 1] == '\n')
            fseek(ptrFile, -strlen(line) - 1, SEEK_CUR);
        else
            fseek(ptrFile, -strlen(line), SEEK_CUR);

        
        // char* token = strtok(line, " ");

        // while (NULL != token) {
        //     result += atoi(token);
        //     token = strtok(NULL, " ");
        // }

    }

    // for (int i = 0; i < section_i; i++) {
    //     printf("Section: %s\n", sections[i].name);
        
    //     for (int j = 0; j < sections[i].size; j++)
    //         printf("%s = %s\n", sections[i].keys[j], sections[i].values[j]);

    //     printf("\n");
    // }
    
    

    if (isValid && argc > 1) {
        char *query = malloc((strlen(argv[1]) + 1) * sizeof(char));
        strncpy(query, argv[1], strlen(argv[1]));
        query[strlen(argv[1])] = '\0';
        char *token;
        token = strtok(query, ".");

        int choice = -1;
        for (int i = 0; i < section_i; i++) {
            if (!strcmp(sections[i].name, token)) {
                choice = i;
                break;
            }
        }

        if (choice < 0) {
            printf("Failed to find section [%s]\n", token);
        }
        else {
            token = strtok(NULL, ".");
            int keyChoice = -1;
            for (int i = 0; i < sections[choice].size; i++) {
                if (!strcmp(sections[choice].keys[i], token)) {
                    keyChoice = i;
                    break;
                }
            }

            if (keyChoice < 0) {
                printf("Failed to find key \"%s\" in section [%s]\n", 
                        token, sections[choice].name);
            }
            else
                printf("%s\n", sections[choice].values[keyChoice]);
        }
    }

    freeSections(sections, sections_count);
    fclose(ptrFile);

    return 0;
}

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