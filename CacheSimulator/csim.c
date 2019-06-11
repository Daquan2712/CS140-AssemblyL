//
// Created by Bradley Pham on 4/20/2019.
//

#include <getopt.h>
#include <string.h>
#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
/*
 * Struct for cache_line
 *
 */
struct cache_line {
    int valid;
    //Should consider if we care about value for this Part A
    long value; //assume cache block has size of 8 bytes
    long tag;
    long LRU_counter;
};
typedef struct cache_line* cache_line_pointer;

/**
 * These are global variables needed for this file
 */
struct cache_line* cache; //simulate matrix by using cache[row][col] = cache[row*num_of_col + col]
int ADDRESS_SIZE = 64;
int tag_size = -1;
long num_of_set = 0;
long num_of_line = 0;
/*----------------------------------------------*/
//remember to free at the end (Not yet done)
int initialize_cache(int s, int E, int b) {

    num_of_set = (long)pow(2,s); //cast because malloc doesn't accept double

    num_of_line = E;
    tag_size = ADDRESS_SIZE - b - s;

    //allocate the size for cache matrix
    cache = malloc(num_of_set*num_of_line*sizeof(struct cache_line));
    if (cache == NULL) {
        printf("Run out of memory\n");
        return -1;
    }
    for (long row = 0; row < num_of_set; row++) {
        for (long col = 0; col < num_of_line; col++) {
            long indx = row*num_of_line + col;
            cache[indx].valid = 0;
        }
    }
    return 1;
}

/*
 * Return 1 if hit
 * Return 2 if miss but store successfully (no evict)
 * Return 3 if miss but store unsuccessfully (has to evict)
 */
int access_cache_store_or_load(long tag, long set, long LRU_count) {
    for (long line = 0; line < num_of_line; line++) {
        long index = set*num_of_line + line;
        /*struct cache_line temp = cache[index];
        if (temp.valid == 1) {
            if (tag == temp.tag) {
                temp.LRU_counter = LRU_count;
                return 1;
            }
        }*/
        if (cache[index].valid == 1) {
            if (tag == cache[index].tag) {
                cache[index].LRU_counter = LRU_count;
                return 1;
            }
        }
    }
    //get until here means miss
    long lowest_count = LONG_MAX; //these two are for in case of no more vacant line
    long lowest_count_indx = -1;
    for (long line = 0; line < num_of_line; line++) {
        long index = set*num_of_line + line;
        /*struct cache_line temp = cache[index];
        if (temp.valid == 0) {
            temp.valid = 1;
            temp.LRU_counter = LRU_count;
            temp.tag = tag;
            return 2;
        } else { //valid == 1
            if (temp.LRU_counter < lowest_count){
                lowest_count = temp.LRU_counter;
                lowest_count_indx = line;
            }
        }*/
        if (cache[index].valid == 0) {
            cache[index].valid = 1;
            cache[index].LRU_counter = LRU_count;
            cache[index].tag = tag;
            return 2;
        } else { //valid == 1
            if (cache[index].LRU_counter < lowest_count){
                lowest_count = cache[index].LRU_counter;
                lowest_count_indx = line;
            }
        }
    }
    //get until here means no more space
    //replace old value at lowest_count_indx
    long index = set*num_of_line + lowest_count_indx;
    /*struct cache_line candidate = cache[index];
    candidate.tag = tag;
    candidate.LRU_counter = LRU_count;*/
    cache[index].tag = tag;
    cache[index].LRU_counter = LRU_count;
    return 3;
}

int read_and_count(FILE* file_to_read, int verbose, int s, int b, int* result) {
    long LRU_count = 0;
    long hit = 0, miss = 0, evict = 0; //to return final values
    char identifier;
    unsigned long address; //because the address can be 64 bits, but int only has 32 bits
    int size;
    while (fscanf(file_to_read," %c %lx,%d", &identifier, &address, &size)>0) {
        //printf("%c %lx %d\n", identifier, address, size);
        //tag is the leftest sequence of bits
        long tag = address >> (s + b);
        //set is in the middle of tag and block offset
        //create mask, s bits => mask is (2^s - 1) and shift left b bit
        long mask = (long)(pow(2, s) - 1) << b; //move 1 bits to s's position
        long set = (address & mask) >> b; //get the actual value of set (move to the rightest

        //There are 3 operations: L (load), M (load -> store), S (store)
        //Load: if load hit, 1 access. If load miss, then add new data into cache. If no vacant line, evict. Otherwise, store
        //M: if load hit, 1 access. If load miss, then store: if no empty line, evict then hit, else, hit
        //S: if there is empty line, then hit. Else, evict then hit
        switch (identifier) {
            case 'L':
            case 'S': {
                int res = access_cache_store_or_load(tag, set, LRU_count);
                if (res == 1) {
                    hit++;
                    if (verbose) printf("%c %lx,%d hit\n", identifier, address, size);
                } else if (res == 2) {
                    miss++;
                    if (verbose) printf("%c %lx,%d miss\n", identifier, address, size);
                } else {
                    miss++; evict++;
                    if (verbose) printf("%c %lx,%d miss eviction\n", identifier, address, size);
                }
                LRU_count++;
                break;
            }
            case 'M': {
                int res_load = access_cache_store_or_load(tag, set, LRU_count);
                if (res_load == 1) {
                    hit++;
                    if (verbose) printf("%c %lx,%d hit ", identifier, address, size);
                } else if (res_load == 2) {
                    miss++;
                    if (verbose) printf("%c %lx,%d miss ", identifier, address, size);
                } else {
                    miss++; evict++;
                    if (verbose) printf("%c %lx,%d miss eviction ", identifier, address, size);
                }
                int res_store = access_cache_store_or_load(tag, set, ++LRU_count);
                if (res_store == 1) {
                    hit++;
                    if (verbose) printf("hit\n");
                } else {
                    printf("Cannot store\n");
                }
                LRU_count++;
                break;
            }
            default:
                printf("Wrong operation\n");
                break;
        }
    }
    result[0] = hit;
    result[1] = miss;
    result[2] = evict;
    return 1;
}

//Have to pass char[65] for result
//Not needed
int hex_to_binary_string(unsigned hex, char* result) {
    //convert hex number to string first
    char string_hex[9];
    sprintf(string_hex, "%016x", hex);
    for (int indx = 0; indx < 9; indx++) {
        switch (string_hex[indx]) {
            case '0':
                strcat(result, "0000");
                break;
            case '1':
                strcat(result, "0001");
                break;
            case '2':
                strcat(result, "0010");
                break;
            case '3':
                strcat(result, "0011");
                break;
            case '4':
                strcat(result, "0100");
                break;
            case '5':
                strcat(result, "0101");
                break;
            case '6':
                strcat(result, "0110");
                break;
            case '7':
                strcat(result, "0111");
                break;
            case '8':
                strcat(result, "1000");
                break;
            case '9':
                strcat(result, "1001");
                break;
            case 'a':
            case 'A':
                strcat(result, "1010");
                break;
            case 'b':
            case 'B':
                strcat(result, "1011");
                break;
            case 'c':
            case 'C':
                strcat(result, "1100");
                break;
            case 'd':
            case 'D':
                strcat(result, "1101");
                break;
            case 'e':
            case 'E':
                strcat(result, "1110");
                break;
            case 'f':
            case 'F':
                strcat(result, "1111");
                break;
            default:
                printf("Input is not hexadecimal number\n");
                return -1;
        }
    }
    return 1;
}

/*-------------------------------------------*/
int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Program need arguments\n");
        return -1;
    }
    int opt = 0;
    int s = -1, E = -1, b = -1;
    char t[25];
    int h = 0, v = 0; //these represent boolean value
    char* ARG_OPT = ":s:E:b:t:h::V::";
    while ((opt = getopt(argc, argv, ARG_OPT)) != -1) {
        switch (opt) {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                strcpy(t, optarg);
                break;
            case 'h':
                h = 1;
                break;
            case 'v':
                v = 1;
                break;
            case ':':
                printf("Missing value for option %c\n", optopt);
                break; //this checks if option has the followed argument, but only works if the option is the last
            default:
                printf("Wrong arguments included\n");
        }
    }
    //need to check for t argument
    if (s == -1 || b == -1 || E == -1 || s == 0 || b == 0 || E == 0)  {
        printf("Missing required option(s) or option expects argument but none found\n");
        return -1;
    }
    //initialize the cache matrix
    initialize_cache(s, E, b);
    //printf("set: %ld line: %ld\n", num_of_set, num_of_line);
    FILE* source_file = fopen(t, "r");
    //printf("Arguments are (s, E, b, t, h, v): (%d %d %d %s %d %d)\n", s, E, b, t, h, v);
    int result[3];
    read_and_count(source_file, 1, s, b, result);
    //printf("Hit: %d Miss: %d Evict: %d \n", result[0], result[1], result[2]);
    printSummary(result[0], result[1], result[2]);
    return 0;
}