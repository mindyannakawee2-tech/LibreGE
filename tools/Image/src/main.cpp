#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace fs = std::filesystem;

static void banner() {
    std::cout << "LibreGE Image Tool v0.1\n";
    std::cout << "=======================\n";
}

static std::string lowerExtension(const std::string& path) {
    std::string ext = fs::path(path).extension().string();

    std::transform(
        ext.begin(),
        ext.end(),
        ext.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return ext;
}

static std::string formatName(const std::string& path) {
    std::string ext = lowerExtension(path);

    if (ext == ".png") return "PNG";
    if (ext == ".jpg" || ext == ".jpeg") return "JPEG";
    if (ext == ".bmp") return "BMP";
    if (ext == ".tga") return "TGA";
    if (ext == ".gif") return "GIF";
    if (ext == ".hdr") return "HDR";
    if (ext == ".psd") return "PSD";
    if (ext == ".pic") return "PIC";
    if (ext == ".pnm" || ext == ".ppm" || ext == ".pgm") return "PNM";

    return "Unknown";
}

static const char* colorType(int channels) {
    switch (channels) {
        case 1: return "Grayscale";
        case 2: return "Grayscale + Alpha";
        case 3: return "RGB";
        case 4: return "RGBA";
        default: return "Unknown";
    }
}

static void help() {
    banner();

    std::cout << R"(

Usage:

  libre-image info <file>
  libre-image validate <file>
  libre-image convert <input> <output.png>
  libre-image help

Commands:

  info
      Display information about an image.

  validate
      Verify LibreGE can load the image.

  convert
      Convert an image to PNG.

  help
      Display this message.

)";
}

static int info(const std::string& path) {
    banner();

    if (!fs::exists(path)) {
        std::cerr << "\n[Image] File not found: " << path << '\n';
        return 1;
    }

    int width = 0;
    int height = 0;
    int channels = 0;

    if (!stbi_info(path.c_str(), &width, &height, &channels)) {
        std::cerr << "\n[Image] Failed to read image.\n";

        if (stbi_failure_reason()) {
            std::cerr << "[Image] " << stbi_failure_reason() << '\n';
        }

        return 1;
    }

    std::cout << '\n';
    std::cout << "File      : " << path << '\n';
    std::cout << "Format    : " << formatName(path) << '\n';
    std::cout << "Width     : " << width << " px\n";
    std::cout << "Height    : " << height << " px\n";
    std::cout << "Channels  : " << channels << '\n';
    std::cout << "Color     : " << colorType(channels) << '\n';
    std::cout << "Pixels    : "
              << static_cast<long long>(width) * height
              << '\n';
    std::cout << "File Size : "
              << fs::file_size(path)
              << " bytes\n";
    std::cout << "HDR       : "
              << (stbi_is_hdr(path.c_str()) ? "Yes" : "No")
              << '\n';

    return 0;
}

static int validate(const std::string& path) {
    banner();

    if (!fs::exists(path)) {
        std::cerr << "\nResult : INVALID\n";
        std::cerr << "Reason : File does not exist\n";
        return 1;
    }

    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* pixels = stbi_load(
        path.c_str(),
        &width,
        &height,
        &channels,
        4
    );

    if (!pixels) {
        std::cerr << "\nResult : INVALID\n";

        if (stbi_failure_reason()) {
            std::cerr << "Reason : " << stbi_failure_reason() << '\n';
        }

        return 1;
    }

    stbi_image_free(pixels);

    std::cout << "\nResult : VALID\n";
    std::cout << "Format : " << formatName(path) << '\n';
    std::cout << "Size   : " << width << "x" << height << '\n';
    std::cout << "\nLibreGE can load this image.\n";

    return 0;
}

static int convert(
    const std::string& input,
    const std::string& output
) {
    banner();

    if (!fs::exists(input)) {
        std::cerr << "\n[Image] Input file not found: "
                  << input << '\n';

        return 1;
    }

    if (lowerExtension(output) != ".png") {
        std::cerr << "\n[Image] v0.1 only supports PNG output.\n";
        return 1;
    }

    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* pixels = stbi_load(
        input.c_str(),
        &width,
        &height,
        &channels,
        4
    );

    if (!pixels) {
        std::cerr << "\n[Image] Failed to load input.\n";

        if (stbi_failure_reason()) {
            std::cerr << "[Image] "
                      << stbi_failure_reason()
                      << '\n';
        }

        return 1;
    }

    fs::path outputPath(output);

    if (outputPath.has_parent_path()) {
        fs::create_directories(outputPath.parent_path());
    }

    int result = stbi_write_png(
        output.c_str(),
        width,
        height,
        4,
        pixels,
        width * 4
    );

    stbi_image_free(pixels);

    if (!result) {
        std::cerr << "\n[Image] Failed to write output.\n";
        return 1;
    }

    std::cout << "\nConversion complete.\n";
    std::cout << "Input  : " << input << '\n';
    std::cout << "Output : " << output << '\n';
    std::cout << "Size   : " << width << "x" << height << '\n';

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        help();
        return 0;
    }

    std::string command = argv[1];

    if (
        command == "help" ||
        command == "-h" ||
        command == "--help"
    ) {
        help();
        return 0;
    }

    if (command == "info") {
        if (argc < 3) {
            std::cerr << "Usage: libre-image info <file>\n";
            return 1;
        }

        return info(argv[2]);
    }

    if (command == "validate") {
        if (argc < 3) {
            std::cerr << "Usage: libre-image validate <file>\n";
            return 1;
        }

        return validate(argv[2]);
    }

    if (command == "convert") {
        if (argc < 4) {
            std::cerr
                << "Usage: libre-image convert <input> <output.png>\n";

            return 1;
        }

        return convert(argv[2], argv[3]);
    }

    std::cerr << "[Image] Unknown command: "
              << command << "\n";

    return 1;
}
