//
// Created by Bradley Pham on 2/28/2019.
//

#include "Assembler.h"
#include "map.h"

static FILE *fileToRead;
static FILE *fileToWrite;
static map table;
static int memory;
static char** linesOfFile;
static int counter;

static int filesAreOpen() {
    return fileToRead != NULL && fileToWrite != NULL;
}

int openFiles(char* nameFileToRead, char* nameFileToWrite) {

    if (filesAreOpen()) {
        printf("Previous files are still open");
        return 0;
    }
    fileToRead = fopen(nameFileToRead, "r");
    if (fileToRead == NULL) {
        printf("Having problem reading the file!");
        return 0;
    }
    fileToWrite = fopen(nameFileToWrite, "w");
    if (fileToWrite == NULL) {
        printf("Having problem open the file to write!");
        return 0;
    }
    //intialise counter
    counter = 0;
    //initialse array to store lines
    linesOfFile = malloc(MAX_LENGTH*sizeof(char*));
    //initialise the symbol table and add predefined values
    table = createMap(MAX_LENGTH);
    insertKey(table, "R0", "0");
    insertKey(table, "R1", "1");
    insertKey(table, "R2", "2");
    insertKey(table, "R3", "3");
    insertKey(table, "R4", "4");
    insertKey(table, "R5", "5");
    insertKey(table, "R6", "6");
    insertKey(table, "R7", "7");
    insertKey(table, "R8", "8");
    insertKey(table, "R9", "9");
    insertKey(table, "R10", "10");
    insertKey(table, "R11", "11");
    insertKey(table, "R12", "12");
    insertKey(table, "R13", "13");
    insertKey(table, "R14", "14");
    insertKey(table, "R15", "15");
    insertKey(table, "SCREEN", "16384");
    insertKey(table, "KBD", "24576");
    insertKey(table, "SP", "0");
    insertKey(table, "LCL", "1");
    insertKey(table, "ARG", "2");
    insertKey(table, "THIS", "3");
    insertKey(table, "THAT", "4");
    //start memory
    memory = 16;
    return 1;
}

//have to allocate memory for the trimmed line for arguments before using this function
static void trim(char* lineWithSpaces, char* lineTrimmed) {
    while (*lineWithSpaces != '\0' && *lineWithSpaces != '\n') {
        if (isspace(*lineWithSpaces) == 0) {
            *lineTrimmed = *lineWithSpaces;
            lineTrimmed++;
        }
        lineWithSpaces++;
    }
    *lineTrimmed = '\0';
}

//return 0 for blank and comment
//return 1 for L_COM
//return 2 for A_COM
//return 3 for C_COM
static int convert(char* line, char* result) {

    //trim the line first
    char* trimmedLine = calloc(strlen(line), sizeof(char));
    trim(line, trimmedLine);

    //ignore in case of a blank line
    if(isspace(trimmedLine[0]) || trimmedLine[0] == '\0') {
        strcpy(result, "BLANK");
        return 0;
    }
    if (trimmedLine[0] == '/') {
        strcpy(result, "COMMENT");
        return 0;
    } else {
        char *buffer = strtok(trimmedLine, "//");
        //Note: buffer won't contain '\n' but fprintf always prints on new line for each line
        //from the input
        if (buffer[0] == '(') {
            strcpy(result, trimmedLine);
            free(trimmedLine);
            return 1;
        } else if (buffer[0] == '@') {
            strcpy(result, trimmedLine);
            free(trimmedLine);
            return 2;
        } else if ((buffer[0] >= 'A' && buffer[0] <= 'Z') || (buffer[0] >= '0' && buffer[0] <= '9')) {
            strcpy(result, trimmedLine);
            free(trimmedLine);
            return 3;
        } else {
            //This is for some reason, blank line on the server is not recognised as "\n"
            strcpy(result, "BLANK");
            free(trimmedLine);
            return 0;
        }
    }
}

//to take the argument's name from (a)
static void extractFromPara(char* lineWithPara, char* output) {
    while (*lineWithPara != '\0' && *lineWithPara != '\n') {
        if (*lineWithPara != '(' || *lineWithPara != ')') {
            *output = *lineWithPara;
            output++;
        }
        lineWithPara++;
    }
    *output = '\0';
}

//return number of non-blank/comment lines
int firstPass() {
    if (!filesAreOpen())
        return 0;
    char* line =  malloc(sizeof(char)*MAX_LENGTH);
    char* convertedLine = malloc(sizeof(char)*MAX_LENGTH);
    while(fgets(line, MAX_LENGTH, fileToRead) != NULL) {
        int res = convert(line, convertedLine);
        //not comment or blank
        if (res != 0) {
            if (res == 1) {
                linesOfFile[counter] = malloc(strlen(convertedLine)*sizeof(char));
                strncpy(linesOfFile[counter], convertedLine, strlen(convertedLine));
                linesOfFile[counter][strlen(convertedLine)] = '\0';

                extractFromPara(convertedLine, convertedLine);

                char buffer[20];
                sprintf(buffer, "%d", counter + 1);
                //to make sure the key doesn't have any extra value for later comparison
                char* trimmedLine = calloc(strlen(buffer), sizeof(char));
                trim(buffer, trimmedLine);
                insertKey(table, convertedLine, trimmedLine);
                //free(temp);
            } else {
                linesOfFile[counter] = malloc(strlen(convertedLine)*sizeof(char));
                strcpy(linesOfFile[counter], convertedLine);
            }
            //printf("%s\n", linesOfFile[counter]);
            counter++;
        }

    }
    free(line);
    free(convertedLine);
    return counter;
}
//this is used to convert an int to 16 bits binary number
char* intToBinaryStr(int n) {
    char* result = malloc(17*sizeof(char));
    char* slider = result;
    *slider = '0';
    slider++;
    for (int i = 14; i >= 0; i--) {
        int k = n >> i;
        if (k & 1) {
            *slider = '1';
        } else {
            *slider = '0';
        }
        slider++;
    }
    *slider = '\0';
    return result;
}
//this is used to convert a COMP part of C_COMMAND and convert a corresponding 7-bits binary value
char* produceComp(char* stringToCmp, int n) {
    char* toWrite = malloc(8*sizeof(char));
    toWrite[0] = '\0';
    if (!strncmp(stringToCmp,"0", n)) {
        strcat(toWrite, "0101010");
    } else if (!strncmp(stringToCmp,"1", n)) {
        strcat(toWrite, "0111111");
    } else if (!strncmp(stringToCmp,"-1", n)) {
        strcat(toWrite, "0111010");
    } else if (!strncmp(stringToCmp,"D", n)) {
        strcat(toWrite, "0001100");
    } else if (!strncmp(stringToCmp,"A", n)) {
        strcat(toWrite, "0110000");
    } else if (!strncmp(stringToCmp,"M", n)) {
        strcat(toWrite, "1110000");
    } else if (!strncmp(stringToCmp,"!D", n)) {
        strcat(toWrite, "0001101");
    } else if (!strncmp(stringToCmp,"!A", n)) {
        strcat(toWrite, "0110001");
    } else if (!strncmp(stringToCmp,"!M", n)) {
        strcat(toWrite, "1110001");
    } else if (!strncmp(stringToCmp,"-D", n)) {
        strcat(toWrite, "0001111");
    } else if (!strncmp(stringToCmp,"-A", n)) {
        strcat(toWrite, "0110011");
    } else if (!strncmp(stringToCmp,"-M", n)) {
        strcat(toWrite, "1110011");
    } else if (!strncmp(stringToCmp,"D+1", n)) {
        strcat(toWrite, "0011111");
    } else if (!strncmp(stringToCmp,"A+1", n)) {
        strcat(toWrite, "0110111");
    } else if (!strncmp(stringToCmp,"M+1", n)) {
        strcat(toWrite, "1110111");
    } else if (!strncmp(stringToCmp,"D-1", n)) {
        strcat(toWrite, "0001110");
    } else if (!strncmp(stringToCmp,"A-1", n)) {
        strcat(toWrite, "0110010");
    } else if (!strncmp(stringToCmp,"M-1", n)) {
        strcat(toWrite, "1110010");
    } else if (!strncmp(stringToCmp,"D+A", n)) {
        strcat(toWrite, "0000010");
    } else if (!strncmp(stringToCmp,"D+M", n)) {
        strcat(toWrite, "1000010");
    } else if (!strncmp(stringToCmp,"D-A", n)) {
        strcat(toWrite, "0010011");
    } else if (!strncmp(stringToCmp,"D-M", n)) {
        strcat(toWrite, "1010011");
    } else if (!strncmp(stringToCmp,"A-D", n)) {
        strcat(toWrite, "0000111");
    } else if (!strncmp(stringToCmp,"M-D", n)) {
        strcat(toWrite, "1000111");
    } else if (!strncmp(stringToCmp,"D&A", n)) {
        strcat(toWrite, "0000000");
    } else if (!strncmp(stringToCmp,"D&M", n)) {
        strcat(toWrite, "1000000");
    } else if (!strncmp(stringToCmp,"D|A", n)) {
        strcat(toWrite, "0010101");
    } else if (!strncmp(stringToCmp,"D|M", n)) {
        strcat(toWrite, "1010101");
    } else {
        strcat(toWrite, "invalid");
    }
    strcat(toWrite, "\0");
    return toWrite;
}
//second pass to translate A and C_COMMAND
int secondPass() {
    for (int i = 0; i < counter; i++) {
        printf("%s\n",linesOfFile[i]);
        //A_COMMAND
        if (linesOfFile[i][0] == '@') {
            //new value
            if (linesOfFile[i][1] >= '0' && linesOfFile[i][1] <= '9') {
                int number = atoi(&linesOfFile[i][1]);
                char* toWrite = intToBinaryStr(number);
                fprintf(fileToWrite, "%s\n", toWrite);
            } else { //a symbol
                if (containsKey(table, &linesOfFile[i][1]) != -1) {
                    int number = atoi(lookupKey(table, &linesOfFile[i][1]));
                    char* toWrite = intToBinaryStr(number);
                    fprintf(fileToWrite, "%s\n", toWrite);
                } else {
                    char buffer[15];
                    sprintf(buffer, "%d", memory++);
                    char* trimmedLine = calloc(strlen(buffer), sizeof(char));
                    trim(buffer, trimmedLine);
                    insertKey(table, &linesOfFile[i][1], trimmedLine);
                }
            }
        }
        //C_COMMAND
        else if ((linesOfFile[i][0] >= 'A' && linesOfFile[i][0] <= 'Z') || (linesOfFile[i][0] >= '0' && linesOfFile[i][0] <= '9')) {
            char* toWrite = malloc(17*sizeof(char));
            //add 111
            toWrite[0] = '\0';
            strcat(toWrite, "111");
            char* pointerToEqual = strchr(linesOfFile[i], '=');
            char* pointerToSemi = strchr(linesOfFile[i], ';');
            if (pointerToSemi == NULL) { // No Jump => Must have dest
                char* result = produceComp(pointerToEqual+1, (int)(linesOfFile[i] + strlen(linesOfFile[i]) - pointerToEqual - 1));
                strcat(toWrite, result);
                //check dest
                //long sizeToCheck = pointerToEqual - linesOfFile[i];
                if (!strncmp(linesOfFile[i], "AMD", 3)) {
                    strcat(toWrite, "111");
                } else if (!strncmp(linesOfFile[i], "AD", 2)) {
                    strcat(toWrite, "110");
                } else if (!strncmp(linesOfFile[i], "AM", 2)) {
                    strcat(toWrite, "101");
                } else if (!strncmp(linesOfFile[i], "MD", 2)) {
                    strcat(toWrite, "011");
                } else if (!strncmp(linesOfFile[i], "M", 1)) {
                    strcat(toWrite, "001");
                } else if (!strncmp(linesOfFile[i], "D", 1)) {
                    strcat(toWrite, "010");
                } else if (!strncmp(linesOfFile[i], "A", 1)) {
                    strcat(toWrite, "100");
                } else {
                    strcat(toWrite, "000");
                }
                //add JMP
                strcat(toWrite, "000");
                strcat(toWrite, "\0");
            } else { //there is Jump
                if (pointerToEqual == NULL) { //no dest
                    char* result = produceComp(linesOfFile[i], 1);
                    strcat(toWrite, result);
                    //add dest
                    strcat(toWrite, "000");
                } else { //there is dest
                    char* result = produceComp(pointerToEqual + 1, (int)(pointerToSemi - pointerToEqual - 1));
                    strcat(toWrite, result);
                    if (!strncmp(linesOfFile[i], "AMD", 3)) {
                        strcat(toWrite, "111");
                    } else if (!strncmp(linesOfFile[i], "AD", 2)) {
                        strcat(toWrite, "110");
                    } else if (!strncmp(linesOfFile[i], "AM", 2)) {
                        strcat(toWrite, "101");
                    } else if (!strncmp(linesOfFile[i], "MD", 2)) {
                        strcat(toWrite, "011");
                    } else if (!strncmp(linesOfFile[i], "M", 1)) {
                        strcat(toWrite, "001");
                    } else if (!strncmp(linesOfFile[i], "D", 1)) {
                        strcat(toWrite, "010");
                    } else if (!strncmp(linesOfFile[i], "A", 1)) {
                        strcat(toWrite, "100");
                    } else {
                        strcat(toWrite, "000");
                    }
                }
                //add jump
                if(!strcmp(pointerToSemi+1, "JGT")) {
                    strcat(toWrite, "001");
                } else if (!strcmp(pointerToSemi + 1, "JEQ")) {
                    strcat(toWrite, "010");
                } else if (!strcmp(pointerToSemi + 1, "JGE")) {
                    strcat(toWrite, "011");
                } else if (!strcmp(pointerToSemi + 1, "JLT")) {
                    strcat(toWrite, "100");
                } else if (!strcmp(pointerToSemi + 1, "JNE")) {
                    strcat(toWrite, "101");
                } else if (!strcmp(pointerToSemi + 1, "JLE")) {
                    strcat(toWrite, "110");
                } else if (!strcmp(pointerToSemi + 1, "JMP")) {
                    strcat(toWrite, "111");
                } else {
                    strcat(toWrite, "000");
                }
                strcat(toWrite, "\0");
            }
            fprintf(fileToWrite, "%s\n", toWrite);
        } else {
            continue;
        }
    }
    return 1;
}


//deprecated
//Only in the first version
int readAndConvertInput() {
    if (!filesAreOpen())
        return 0;
    char* line =  malloc(sizeof(char)*MAX_LENGTH);
    char* convertedLine = malloc(sizeof(char)*MAX_LENGTH);

    //first pass

    while(fgets(line, MAX_LENGTH ,fileToRead) != NULL) {
        convert(line, convertedLine);
        //if the converted line is not blank or comment
        if (strcmp(convertedLine, "COMMENT") && strcmp(convertedLine, "BLANK")) {
            fprintf(fileToWrite, "%s\n", convertedLine);
        }
    }

    free(line);
    free(convertedLine);
    return 1;
}

int closeFiles() {
    if (fileToWrite != NULL && fileToRead != NULL) {
        fclose(fileToWrite);
        fclose(fileToRead);
        fileToRead = NULL;
        fileToWrite = NULL;
    } else {
        printf("No open stream to close\n");
        return 0;
    }
    freeMap(table);
    for (int i = 0; i < counter; i++) {
        free(linesOfFile[i]);
    }
    free(linesOfFile);
    return 1;
}


