
#include "Assembler.h"
#include "bits.h"

int main(int agrc, char** argv) {

   /* if (agrc < 3) {
        printf("Not enough arguments!\n");
        return -1;
    }
    openFiles(argv[1], argv[2]);
    //readAndConvertInput();
    firstPass();
    secondPass();
    closeFiles();*/

   printf("%x\n", bitCount(0x34518766));
    return 0;
}