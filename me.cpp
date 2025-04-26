#include <iostream>
#include <string>

int main() {
    std::string name;
    
    // Ask for the user's name
    std::cout << "Enter your name: ";
    std::getline(std::cin, name);
    
    // Greet the user
    std::cout << "Hello, " << name << "!" << std::endl;
    
    return 0;
}