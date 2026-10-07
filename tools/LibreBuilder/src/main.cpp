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

struct LinuxDistro {
    std::string id;
    std::string idLike;
    std::string name;
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

static bool startsWith(
    const std::string& text,
    const std::string& prefix
) {
    return text.rfind(prefix, 0) == 0;
}

static bool contains(
    const std::string& text,
    const std::string& value
) {
    return text.find(value) != std::string::npos;
}

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

class Parser {
public:
    BuildConfig parse(
        const std::string& path
    ) {
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

                parseBlockEntry(
                    config,
                    activeBlock,
                    line
                );

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

static LinuxDistro detectLinuxDistro() {
    LinuxDistro distro;

#ifdef __linux__

    std::ifstream file("/etc/os-release");

    if (!file.is_open()) {
        return distro;
    }

    std::string line;

    while (std::getline(file, line)) {
        const auto pos = line.find('=');

        if (pos == std::string::npos) {
            continue;
        }

        std::string key = trim(
            line.substr(0, pos)
        );

        std::string value = removeQuotes(
            line.substr(pos + 1)
        );

        if (key == "ID") {
            distro.id = value;
        }
        else if (key == "ID_LIKE") {
            distro.idLike = value;
        }
        else if (key == "PRETTY_NAME") {
            distro.name = value;
        }
    }

#endif

    return distro;
}

static int setupLinuxDependencies() {

#ifndef __linux__

    std::cerr
        << "[LibreBuilder] setup currently supports Linux only.\n";

    return 1;

#else

    LinuxDistro distro = detectLinuxDistro();

    std::cout << "\nLibreBuilder Linux Setup\n";
    std::cout << "========================\n";

    if (!distro.name.empty()) {
        std::cout
            << "Detected : "
            << distro.name
            << '\n';
    }

    std::cout
        << "ID       : "
        << (
            distro.id.empty()
                ? "<unknown>"
                : distro.id
        )
        << '\n';

    std::cout
        << "ID_LIKE  : "
        << (
            distro.idLike.empty()
                ? "<none>"
                : distro.idLike
        )
        << "\n\n";

    const std::string combined =
        distro.id + " " + distro.idLike;

    /*
     * Debian / Ubuntu / Linux Mint
     */
    if (
        contains(combined, "debian") ||
        distro.id == "ubuntu" ||
        distro.id == "linuxmint" ||
        distro.id == "pop"
    ) {
        std::cout
            << "[LibreBuilder] Package manager: APT\n";

        int result = executeCommand(
            "sudo apt update"
        );

        if (result != 0) {
            return result;
        }

        return executeCommand(
            "sudo apt install -y "
            "build-essential "
            "cmake "
            "ninja-build "
            "git "
            "pkg-config"
        );
    }

    /*
     * Fedora / RHEL / Rocky / Alma
     */
    if (
        contains(combined, "fedora") ||
        contains(combined, "rhel") ||
        distro.id == "rocky" ||
        distro.id == "almalinux"
    ) {
        std::cout
            << "[LibreBuilder] Package manager: DNF\n";

        return executeCommand(
            "sudo dnf install -y "
            "gcc "
            "gcc-c++ "
            "cmake "
            "ninja-build "
            "git "
            "pkgconf-pkg-config"
        );
    }

    /*
     * Arch / Manjaro / EndeavourOS
     */
    if (
        contains(combined, "arch") ||
        distro.id == "manjaro" ||
        distro.id == "endeavouros"
    ) {
        std::cout
            << "[LibreBuilder] Package manager: Pacman\n";

        return executeCommand(
            "sudo pacman -S --needed "
            "base-devel "
            "cmake "
            "ninja "
            "git "
            "pkgconf"
        );
    }

    /*
     * openSUSE
     */
    if (
        contains(combined, "suse") ||
        distro.id == "opensuse-tumbleweed" ||
        distro.id == "opensuse-leap"
    ) {
        std::cout
            << "[LibreBuilder] Package manager: Zypper\n";

        return executeCommand(
            "sudo zypper install "
            "-y "
            "gcc "
            "gcc-c++ "
            "cmake "
            "ninja "
            "git "
            "pkg-config"
        );
    }

    std::cerr
        << "[LibreBuilder] Unsupported Linux distribution.\n\n";

    std::cerr
        << "LibreBuilder could not determine the correct package manager.\n";

    std::cerr
        << "Install these tools manually:\n"
        << "  C/C++ compiler\n"
        << "  CMake\n"
        << "  Ninja\n"
        << "  Git\n"
        << "  pkg-config\n";

    return 1;

#endif
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
    }
    else {
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


static int copyRuntimeAssets() {
    const fs::path sourceAssets = "assets";
    const fs::path buildAssets = "build/assets";

    if (!fs::exists(sourceAssets)) {
        std::cout
            << "[LibreBuilder] No assets directory found. Skipping asset copy.\n";

        return 0;
    }

    std::cout
        << "[LibreBuilder] Copying assets...\n";

    std::error_code ec;

    if (fs::exists(buildAssets)) {
        fs::remove_all(
            buildAssets,
            ec
        );

        if (ec) {
            std::cerr
                << "[LibreBuilder] Failed to remove old assets: "
                << ec.message()
                << '\n';

            return 1;
        }
    }

    fs::create_directories(
        buildAssets,
        ec
    );

    if (ec) {
        std::cerr
            << "[LibreBuilder] Failed to create build/assets: "
            << ec.message()
            << '\n';

        return 1;
    }

    fs::copy(
        sourceAssets,
        buildAssets,
        fs::copy_options::recursive |
        fs::copy_options::overwrite_existing,
        ec
    );

    if (ec) {
        std::cerr
            << "[LibreBuilder] Failed to copy assets: "
            << ec.message()
            << '\n';

        return 1;
    }

    std::cout
        << "[LibreBuilder] Assets copied to build/assets\n";

    return 0;
}

static int defaultBuild() {
    std::cout
        << "[LibreBuilder] Using default LibreGE build process.\n";

    if (!fs::exists("build")) {
        fs::create_directories("build");
    }

    int result = executeCommand(
        "cmake -S . -B build "
        "-G Ninja "
        "-DCMAKE_BUILD_TYPE=Debug"
    );

    if (result != 0) {
        return result;
    }

    result = executeCommand(
        "cmake --build build"
    );

    if (result != 0) {
        return result;
    }

    result = copyRuntimeAssets();

    if (result != 0) {
        return result;
    }

    std::cout
        << "[LibreBuilder] Building Java runtime...\\n";

    result = executeCommand(
        "./scripts/build-java-runtime.sh"
    );

    if (result != 0) {
        return result;
    }

    std::cout
        << "[LibreBuilder] Build package ready in build/\\n";

    return 0;

    std::cout
        << "[LibreBuilder] Build package ready in build/\n";

    return 0;
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
    const std::vector<std::string> executables = {
        "LibreGE",
        "bin/LibreGE"
    };

    for (const auto& exe : executables) {
        const fs::path fullPath =
            fs::path("build") / exe;

        if (fs::exists(fullPath)) {
            std::cout
                << "[LibreBuilder] Running from build directory...\n";

            return executeCommand(
                "cd build && ./" + exe
            );
        }
    }

    std::cerr
        << "[LibreBuilder] Could not find LibreGE executable.\n";

    std::cerr
        << "[LibreBuilder] Expected:\n"
        << "  build/LibreGE\n"
        << "  build/bin/LibreGE\n";

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

static void cleanBuild() {
    if (fs::exists("build")) {
        std::cout
            << "[LibreBuilder] Removing build directory...\n";

        fs::remove_all("build");
    }

    std::cout
        << "[LibreBuilder] Clean complete.\n";
}

static void printHelp() {
    std::cout << R"(
LibreBuilder

Usage:
  lbbe [command] [file]

Commands:

  setup
      Detect Linux distribution and install
      LibreBuilder build dependencies.

  update
      Safely update LibreGE engine files without
      replacing project assets, scripts, scenes,
      build configuration, or game source.

  build
      Build the LibreGE project.

  run
      Build and run LibreGE.

  parse
      Parse LibreBuild.lbbe and display
      its configuration.

  clean
      Remove the build directory.

  help
      Show this message.


Supported Linux distributions:

  Debian
  Ubuntu
  Linux Mint
  Pop!_OS

  Fedora
  RHEL
  Rocky Linux
  AlmaLinux

  Arch Linux
  Manjaro
  EndeavourOS

  openSUSE Leap
  openSUSE Tumbleweed


Examples:

  lbbe setup

  lbbe update

  lbbe build

  lbbe run

  lbbe clean

  lbbe parse

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

    if (command == "setup") {
        return setupLinuxDependencies();
    }

    if (command == "update") {
#ifdef __linux__
    return std::system(
        "./tools/LibreBuilder/update.sh"
    );
#else
    std::cerr
        << "[LibreBuilder] update currently supports Linux only.\n";

    return 1;
#endif
}

    if (command == "clean") {
        cleanBuild();
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

    if (
        command != "build" &&
        command != "run"
    ) {
        std::cerr
            << "[LibreBuilder] Unknown command: "
            << command
            << '\n';

        printHelp();

        return 1;
    }

    int result = runUserProcesses(config);

    if (result != 0) {
        return result;
    }

    if (config.buildCommands.empty()) {
        result = defaultBuild();
    }
    else {
        result = customBuild(config);
    }

    if (result != 0) {
        return result;
    }

    if (command == "run") {
        if (config.runCommands.empty()) {
            result = defaultRun();
        }
        else {
            result = customRun(config);
        }
    }

    return result;
}
