#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// #define DEBUG

#include "iniparse.h"


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

    // declare and initialize auxiliary variables
    short isValid = 1;
    int section_i = 0;
    int n;
    long startSection = 0;
    int attributes_count = 0;
    char *key;
    char *value;

    // iterate through the sections
    // check each line, look for a section
    // parse all the data into structures
    while (fgets(line, max_line_len + 1, ptrFile) != NULL) {
        // if there is a section name
        if (line[0] != '[') {
            // skip the commented line
            // gotoNextLine(ptrFile);

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
        n = strlen(line);
        sections[section_i].name = malloc(sizeof(char) * (n - 2));
        strncpy(sections[section_i].name, line + 1, n - 2);
        sections[section_i].name[n - 3] = '\0';

        startSection = ftell(ptrFile);
        attributes_count = 0;

        // fill all the attributes
        while (fgets(line, max_line_len + 1, ptrFile) != NULL) {
            // skip if new section
            if (line[0] == '[')
                break;

            // skip if blank line or comment
            if (line[0] == '\n')
                continue;

            if (line[0] == ';') {
                //gotoNextLine(ptrFile);
                continue;
            }

            // check keys
            key = malloc(sizeof(char) * strlen(line));
            value = malloc(sizeof(char) * strlen(line));
            sscanf(line, "%s = %s", key, value);
            if (!isCorrectKey(key)) {
                printf("InvalidKeyException: %s\n", key);
                isValid = 0;
                free(key);
                free(value);
                continue;
            }
            free(key);
            free(value);
            
            attributes_count++;
        }

        sections[section_i].size = attributes_count;
        // allocate memory for section's attributes
        sections[section_i].keys = malloc(sizeof(char*) * attributes_count);
        sections[section_i].values = malloc(sizeof(char*) * attributes_count);

        // back to the beginning of the section
        fseek(ptrFile, startSection, SEEK_SET);
        attributes_count = 0;

        // fill section's attributes
        while (fgets(line, max_line_len + 1, ptrFile) != NULL) {
            if (line[0] == '[')
                break;

            if (line[0] == '\n')
                continue;

            if (line[0] == ';') {
                //gotoNextLine(ptrFile);
                continue;
            }

            // check key
            key = malloc(sizeof(char) * strlen(line));
            value = malloc(sizeof(char) * strlen(line));
            sscanf(line, "%s = %s", key, value);
            if (!isCorrectKey(key)) {
                free(key);
                free(value);
                continue;
            }

            // if everything is alright,
            // memory is already allocated,
            // so just copy the pointer
            sections[section_i].keys[attributes_count] = key;
            sections[section_i].values[attributes_count] = value;

            attributes_count++;
        }

        section_i++;

        //go to the next section
        n = strlen(line);
        // if (line[n - 1] == '\n')
        //     fseek(ptrFile, -strlen(line) - 1, SEEK_CUR);
        // else
        //     fseek(ptrFile, -strlen(line), SEEK_CUR);
        fseek(ptrFile, -strlen(line) - 1, SEEK_CUR);
        printf("%ld", ftell(ptrFile));
    }

    // the implementation of finding "section.key"
    if (isValid && argc > 1) {
        n = strlen(argv[1]);
        char *query = malloc((n + 1) * sizeof(char));
        strncpy(query, argv[1], n);
        query[n] = '\0';

        char *token;
        token = strtok(query, ".");

        int choice = findSection(sections, token, section_i);

        if (choice >= 0) {
            token = strtok(NULL, ".");
            int keyChoice = findKey(sections, token, choice);

            if (keyChoice >= 0)
                printf("%s\n", sections[choice].values[keyChoice]);
        }
        free(query);
    }
    free(line);

    freeSections(sections, sections_count);
    fclose(ptrFile);

    return 0;
}