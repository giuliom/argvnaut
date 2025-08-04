#pragma once

#include <string>
#include <vector>

class ArgvNaut {
public:
    ArgvNaut(int argc, char* argv[]);
    std::string getArgument(int index) const;
    std::vector<std::string> getAllArguments() const;

private:
    int argc;
    char** argv;
};
