#include "helpers/utf8.hpp"

#include <iostream>

int main()
{
    bool result = guchho::helpers::ContainsNonBMPCodePoint("Hello 😀");

    std::cout << std::boolalpha << result << '\n';

    return 0;
}