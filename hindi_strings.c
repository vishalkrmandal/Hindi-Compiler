#include <stdio.h>
#include <string.h>

int main() {
    // Hindi greeting
    printf("नमस्ते दुनिया!\n");
    
    // Store Hindi text in variables
    const char* greeting = "नमस्ते";
    const char* world = "दुनिया";
    
    // Display concatenated greeting
    printf("%s %s!\n", greeting, world);
    
    // Show the byte length of Hindi strings
    printf("Bytes in greeting: %zu\n", strlen(greeting));
    printf("Bytes in world: %zu\n", strlen(world));
    
    return 0;
}
