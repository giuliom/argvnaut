#include <iostream>
#include <argvnaut.h>



int main()
{
    ArgvNaut::Parser parser("testprog");
    parser.addFlag("help", "h");
	parser.addOption("width", "w", ArgvNaut::OptionType::INTEGER, "1280");
	parser.addOption("height", "e", ArgvNaut::OptionType::INTEGER, "720");
	parser.addOption("output", "o", ArgvNaut::OptionType::STRING, "");
	parser.addOption("rendering", "r", ArgvNaut::OptionType::STRING, "raytracer");
	parser.addOption("shading", "s", ArgvNaut::OptionType::STRING, "lit");
	parser.addOption("file", "f", ArgvNaut::OptionType::STRING, "");
	parser.addOption("samples", "p", ArgvNaut::OptionType::INTEGER, "4");
	parser.addOption("bounces", "b", ArgvNaut::OptionType::INTEGER, "3");

    const char* args[] = {"testprog", "-h"};
    parser.parse(2, args);

    parser.getFlag("help");

    std::cout<< "Help: " << (parser.getFlag("help") ? "true" : "false") << std::endl;

    return 0;
}