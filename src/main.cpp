#include <iostream>
#include <argvnaut.h>



int main(int argc, char** argv)
{
    ArgvNaut::Parser parser("ArgvnautSample");
    parser.addFlag("help", "h", "Show this help message");
    parser.addOption("width", "w", ArgvNaut::OptionType::INTEGER, "1280", false, "Image width in pixels");
    parser.addOption("height", "e", ArgvNaut::OptionType::INTEGER, "720", false, "Image height in pixels");
    parser.addOption("output", "o", ArgvNaut::OptionType::STRING, "", false, "Output file path");
    parser.addOption("rendering", "r", ArgvNaut::OptionType::STRING, "raytracer", false, "Rendering backend");
    parser.addOption("shading", "s", ArgvNaut::OptionType::STRING, "lit", false, "Shading model");
    parser.addOption("file", "f", ArgvNaut::OptionType::STRING, "", false, "Input scene file");
    parser.addOption("samples", "p", ArgvNaut::OptionType::INTEGER, "4", false, "Samples per pixel");
    parser.addOption("bounces", "b", ArgvNaut::OptionType::INTEGER, "3", false, "Maximum ray bounces");

    if (!parser.parse(argc, argv)) {
        std::cerr << "Error: " << parser.error() << "\n" << parser.usage() << std::endl;
        return 1;
    }

    if (parser.getFlag("help")) {
        std::cout << parser.help();
        return 0;
    }

    std::cout << "width:     " << parser.getInt("width").value() << "\n"
              << "height:    " << parser.getInt("height").value() << "\n"
              << "rendering: " << parser.getString("rendering").value() << "\n"
              << "shading:   " << parser.getString("shading").value() << "\n"
              << "samples:   " << parser.getInt("samples").value() << "\n"
              << "bounces:   " << parser.getInt("bounces").value() << std::endl;

    return 0;
}