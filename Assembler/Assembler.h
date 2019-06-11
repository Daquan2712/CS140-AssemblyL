//
// Created by Bradley Pham on 2/28/2019.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifndef ASSEMBLER_ASSEMBLER_H
#define ASSEMBLER_ASSEMBLER_H

#endif //ASSEMBLER_ASSEMBLER_H

#define MAX_LENGTH 200
/*
 * Before using readAndConvertInput(), user must openFiles first, otherwise an error will be printed out
 * The function accepts 2 strings: The name of the file to read and the name of the file to write
 * These name files must include the extension to output the right one
 * Return: 1 if successful, otherwise 0;
 * */
int openFiles(char* nameFileToRead, char* nameFileToWrite);
/*
 * After opening the file, user can utilise this function to convert HACK language to Binary Code
 * The output will be stored in the output file specified in openFiles()
 * Deprecated, used only in the first version
 * Return: 1 if sucessful, otherwise 0;
 * */
int readAndConvertInput();

/*
 * Use this function to go through HACK file to take care of L_COMMAND; get ready to translate all C and A_COMMAND
 * Return: the number of lines in the HACK file excluding blank and comment lines
 * */
int firstPass();
/*
 * Use this function to pass through the file second time and convert A and C_COMMAND into binary file
 * The output of binary lines will be output to the FILE fileToWrite defined in the openFiles()
 * Return: 1 if successful
 * */
int secondPass();
/*
 * This function is used to closed open stream to reading and writing files
 * and also free all the memory used in the previous translation
 * Return: 1 if successful, otherwise 0;
 * */
int closeFiles();