#include <stdio.h>

// Function to add two numbers (using transliteration of Hindi word "jod")
int jod(int a, int b) {
    return a + b;
}

int main() {
    // Variables with transliterated Hindi names
    int sankhya1 = 10;  // संख्या1 (number1)
    int sankhya2 = 5;   // संख्या2 (number2)
    
    // Calculate sum using transliterated Hindi function
    int parinam = jod(sankhya1, sankhya2);  // परिणाम (result)
    
    // Display result with transliterated Hindi words in strings
    printf("Namaste Duniya!\n");            // नमस्ते दुनिया!
    printf("Jod: %d + %d = %d\n", sankhya1, sankhya2, parinam);
    
    return 0;
}
