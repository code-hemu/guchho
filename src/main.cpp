#include "cli/cli.hpp"

#include <string>
#include <vector>

int main(int argc, char** argv)
{
    std::vector<std::string> args(argv + 1, argv + argc);
    return guchho::cli::Run(args);
}
