#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct BuildConfig {
    std::vector<std::string> dependencies;
    std::string target;
    std::string platform;

    std::vector<std::string> buildCommands;
    std::vector<std::string> runCommands;
    std::vector<std::string> userProcesses;
};

static std::string trim(const std::string& input) {
    const auto begin = std::find_if_not(
        input.begin(),
        input.end(),
        [](unsigned char ch) {
            return std::isspace(ch);
        }
    );

    const auto end = std::find_if_not(
        input.rbegin(),
        input.rend(),
        [](unsigned char ch) {
            return std::isspace(ch);
        }
    ).base();

    if (begin >= end) {
        return "";
    }

    return std::string(begin, end);
}

static std::string removeQuotes(std::string value) {
    value = trim(value);

    if (value.size() >= 2) {
        if (
            (value.front() == '"' && value.back() == '"') ||
            (value.front() == '\'' && value.back() == '\'')
        ) {
            return value.substr(1, value.size() - 2);
        }
    }

    return value;
}

static bool startsWith(const std::string& text, const std::string& prefix) {
    return text.rfind(prefix, 0) == 0;
}

class Parser {
public:
    BuildConfig parse(const std::string& path) {
        std::ifstream file(path);

        if (!file.is_open()) {
            throw std::runtime_error(
                "Unable to open build file: " + path
            );
        }

        BuildConfig config;

        std::string line;
        std::string activeBlock;

        while (std::getline(file, line)) {
            line = trim(line);

            if (line.empty()) {
                continue;
            }

            if (startsWith(line, "#")) {
                continue;
            }

            if (!activeBlock.empty()) {
                if (line == ")") {
                    activeBlock.clear();
                    continue;
                }

                parseBlockEntry(config, activeBlock, line);
                continue;
            }

            if (startsWith(line, "DEP=")) {
                if (line.find('(') != std::string::npos) {
                    activeBlock = "DEP";
                    continue;
                }

                config.dependencies.push_back(
                    removeQuotes(
                        line.substr(
                            line.find('=') + 1
                        )
                    )
                );

                continue;
            }

            if (startsWith(line, "TAR=")) {
                config.target = removeQuotes(
                    line.substr(
                        line.find('=') + 1
                    )
                );

                continue;
            }

            if (startsWith(line, "TPF=")) {
                config.platform = removeQuotes(
                    line.substr(
                        line.find('=') + 1
                    )
                );

                continue;
            }

            if (startsWith(line, "build=")) {
                activeBlock = "build";

                if (line.find("()") != std::string::npos) {
                    activeBlock.clear();
                }

                continue;
            }

            if (startsWith(line, "run=")) {
                activeBlock = "run";

                if (line.find("()") != std::string::npos) {
                    activeBlock.clear();
                }

                continue;
            }

            if (startsWith(line, "userprocess=")) {
                activeBlock = "userprocess";

                if (line.find("()") != std::string::npos) {
                    activeBlock.clear();
                }

                continue;
            }

            std::cerr
                << "[LibreBuilder] Warning: unknown statement: "
                << line
                << '\n';
        }

        return config;
    }

private:
    static void parseBlockEntry(
        BuildConfig& config,
        const std::string& block,
        const std::string& rawLine
    ) {
        std::string value = removeQuotes(rawLine);

        if (value.empty()) {
            return;
        }

        if (block == "DEP") {
            config.dependencies.push_back(value);
            return;
        }

        if (block == "build") {
            config.buildCommands.push_back(value);
            return;
        }

        if (block == "run") {
            config.runCommands.push_back(value);
            return;
        }

        if (block == "userprocess") {
            config.userProcesses.push_back(value);
            return;
        }
    }
};

static int executeCommand(
    const std::string& command
) {
    std::cout
        << "\n[LibreBuilder] $ "
        << command
        << '\n';

    const int result = std::system(command.c_str());

    if (result != 0) {
        std::cerr
            << "[LibreBuilder] Command failed with code "
            << result
            << '\n';
    }

    return result;
}

static void printConfig(
    const BuildConfig& config
) {
    std::cout << "\n";
    std::cout << "LibreBuilder\n";
    std::cout << "============\n";

    std::cout
        << "Target       : "
        << (
            config.target.empty()
                ? "<default>"
                : config.target
        )
        << '\n';

    std::cout
        << "Platform     : "
        << (
            config.platform.empty()
                ? "<host>"
                : config.platform
        )
        << '\n';

    std::cout << "Dependencies : ";

    if (config.dependencies.empty()) {
        std::cout << "<none>\n";
    } else {
        std::cout << '\n';

        for (const auto& dep : config.dependencies) {
            std::cout
                << "  - "
                << dep
                << '\n';
        }
    }

    std::cout
        << "Build mode   : "
        << (
            config.buildCommands.empty()
                ? "LibreGE default"
                : "custom"
        )
        << '\n';

    std::cout
        << "Run mode     : "
        << (
            config.runCommands.empty()
                ? "LibreGE default"
                : "custom"
        )
        << '\n';

    std::cout << '\n';
}

static bool checkDependencies(
    const BuildConfig& config
) {
    bool ok = true;

    for (const auto& dep : config.dependencies) {
        if (!fs::exists(dep)) {
            std::cerr
                << "[LibreBuilder] Missing dependency: "
                << dep
                << '\n';

            ok = false;
        }
    }

    return ok;
}

static int runUserProcesses(
    const BuildConfig& config
) {
    for (const auto& command : config.userProcesses) {
        const int result = executeCommand(command);

        if (result != 0) {
            return result;
        }
    }

    return 0;
}

static int defaultBuild() {
    std::cout
        << "[LibreBuilder] Using default LibreGE build process.\n";

    if (!fs::exists("build")) {
        fs::create_directories("build");
    }

    int result = executeCommand(
        "cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug"
    );

    if (result != 0) {
        return result;
    }

    result = executeCommand(
        "cmake --build build"
    );

    return result;
}

static int customBuild(
    const BuildConfig& config
) {
    for (const auto& command : config.buildCommands) {
        const int result = executeCommand(command);

        if (result != 0) {
            return result;
        }
    }

    return 0;
}

static int defaultRun() {
    const std::vector<std::string> possibleExecutables = {
        "build/LibreGE",
        "build/bin/LibreGE",
        "bin/LibreGE"
    };

    for (const auto& exe : possibleExecutables) {
        if (fs::exists(exe)) {
            return executeCommand("./" + exe);
        }
    }

    std::cerr
        << "[LibreBuilder] Could not find LibreGE executable.\n";

    std::cerr
        << "[LibreBuilder] Expected one of:\n";

    for (const auto& exe : possibleExecutables) {
        std::cerr
            << "  "
            << exe
            << '\n';
    }

    return 1;
}

static int customRun(
    const BuildConfig& config
) {
    for (const auto& command : config.runCommands) {
        const int result = executeCommand(command);

        if (result != 0) {
            return result;
        }
    }

    return 0;
}

static void printHelp() {
    std::cout << R"(
LibreBuilder

Usage:
  lbbe [command] [file]

Commands:
  build      Build the project
  run        Build and run the project
  parse      Parse and print configuration
  clean      Delete build directory
  help       Show this help

Examples:
  lbbe build
  lbbe run
  lbbe parse
  lbbe build LibreBuild.lbbe
)";
}

int main(
    int argc,
    char** argv
) {
    const std::string command =
        argc >= 2
            ? argv[1]
            : "build";

    const std::string buildFile =
        argc >= 3
            ? argv[2]
            : "LibreBuild.lbbe";

    if (
        command == "help" ||
        command == "-h" ||
        command == "--help"
    ) {
        printHelp();
        return 0;
    }

    if (command == "clean") {
        if (fs::exists("build")) {
            std::cout
                << "[LibreBuilder] Removing build directory...\n";

            fs::remove_all("build");
        }

        std::cout
            << "[LibreBuilder] Clean complete.\n";

        return 0;
    }

    BuildConfig config;

    try {
        Parser parser;
        config = parser.parse(buildFile);
    }
    catch (const std::exception& e) {
        std::cerr
            << "[LibreBuilder] "
            << e.what()
            << '\n';

        return 1;
    }

    printConfig(config);

    if (!checkDependencies(config)) {
        return 1;
    }

    if (command == "parse") {
        return 0;
    }

    int result = runUserProcesses(config);

    if (result != 0) {
        return result;
    }

    if (
        command == "build" ||
        command == "run"
    ) {
        if (config.buildCommands.empty()) {
            result = defaultBuild();
        } else {
            result = customBuild(config);
        }

        if (result != 0) {
            return result;
        }
    }

    if (command == "run") {
        if (config.runCommands.empty()) {
            result = defaultRun();
        } else {
            result = customRun(config);
        }
    }

    return result;
}
