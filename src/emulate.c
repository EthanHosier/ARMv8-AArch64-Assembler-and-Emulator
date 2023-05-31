#include <stdio.h>
#include <stdint.h>
#include "utils.h"

#define MAX_WORDS 100
int main(void) {
    uint32_t words[MAX_WORDS];
    
    readBinaryFile(words, MAX_WORDS);

    // Print the words
    for (size_t i = 0; i < 3; i++) {
        printf("Word %zu: %u\n", i + 1, words[i]);
    }
    

    return 0;
} 

