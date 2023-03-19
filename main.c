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

    // iterate through the sections
    // check each line, look for a section
    // parse all the data into structures
    short isValid = 1;
    int section_i = 0;
    int n;
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
        n = strlen(line);
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