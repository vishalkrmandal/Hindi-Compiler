#include <stdio.h>
#include <string.h>

int main() {
    const char* hindi = "शून्य";
    printf("String length: %zu\n", strlen(hindi));
    
    for (int i = 0; i < strlen(hindi); i++) {
        unsigned char c = (unsigned char)hindi[i];
        printf("Byte %d: 0x%02X\n", i, c);
    }
    
    return 0;
}
