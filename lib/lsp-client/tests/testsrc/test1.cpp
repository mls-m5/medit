#include "test1.h" // Include flag test. check .clangd-file in parent dir
#include <iostream>

void apa() {
    std::cout << "apeliap!\n";
}

int main() {
    std::cout << "hello\n";

    apa();
}
