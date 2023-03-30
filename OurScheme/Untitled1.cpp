#include <iostream>
#include <string>

int main() {
  std::string input_string;
  std::cin >> input_string;
  
  for (int i = 0; i < input_string.length(); ++i) {
    if (input_string[i] >= 'a' && input_string[i] <= 'z') {
      input_string[i] = input_string[i] - 'a' + 'A';
    }
  }
  
  std::cout << input_string << std::endl;
  
  return 0;
}
