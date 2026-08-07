#include "guchho/logger.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <system_error>
#include <string_view>

namespace guchho::cli {
namespace logger = guchho::logger;

constexpr int kExitOk    = 0;
constexpr int kExitError = 1;
constexpr int kExitUsage = 2;

std::string VersionString()
{
#ifdef GUCHHO_VERSION_STRING
    return "guchho " GUCHHO_VERSION_STRING;
#else
    return "guchho 0.0.0";
#endif
}

// ---------------------------------------------------------------------------
// Handlers
// ---------------------------------------------------------------------------

using Handler = int (*)(const std::vector<std::string>& args, const std::vector<std::string>& os_args);

static int HandleInit(const std::vector<std::string>& args, const std::vector<std::string>& os_args)
{
    bool        force = false;
    std::string dir = ".";

    for (const auto& arg : args) {
        if (arg == "--force") {
            force = true;
        } else if (!arg.empty() && arg[0] == '-') {
            if (arg == "--color" || arg == "--color=true" || arg == "--color=false" ||
                arg == "--log-level=info" || arg == "--log-level=warning" ||
                arg == "--log-level=error" || arg == "--log-level=silent") {
                // Passthrough options are handled by the logger already.
            } else {
                logger::PrintErrorToStderr(os_args, "Unknown option: \"" + arg + "\"");
                return kExitUsage;
            }
        } else {
            dir = arg;
        }
    }

    std::error_code      ec;
    std::filesystem::path path = std::filesystem::path(dir) / "guchho.config.json";

    if (std::filesystem::exists(path) && !force) {
        logger::PrintErrorToStderr(os_args, "guchho.config.json already exists (use --force to overwrite)");
        return kExitError;
    }

    std::filesystem::create_directories(dir, ec);

    std::ofstream file(path);
    if (!file) {
        logger::PrintErrorToStderr(os_args, "Failed to create " + path.string());
        return kExitError;
    }

    file << R"({
  "root": ".",

  "server": {
    "host": "localhost",
    "port": 3000,
    "open": true
  },

  "build": {
    "outDir": "dist",
    "target": "es2022",
    "format": "esm",
    "minify": true,
    "sourcemap": true,
    "treeShake": true,
    "codeSplit": true
  },

  "resolve": {
    "alias": {
      "@": "./src"
    }
  },

  "define": {
    "__DEV__": true
  }
}
)";
    file.close();

    std::cout << "Created " << path.string() << '\n';
    return kExitOk;
}

static int HandleDoctor(const std::vector<std::string>& /*args*/, const std::vector<std::string>& /*os_args*/)
{
    logger::TerminalInfo info = logger::GetTerminalInfo(1);

    std::string os;
    std::string arch;

#ifdef _WIN32
    os = "Windows";
#ifdef _WIN64
    arch = "x64";
#elif defined(_M_ARM64)
    arch = "arm64";
#else
    arch = "x86";
#endif
#elif defined(__APPLE__)
    os = "macOS";
#ifdef __aarch64__
    arch = "arm64";
#else
    arch = "x64";
#endif
#elif defined(__linux__)
    os = "Linux";
#ifdef __x86_64__
    arch = "x64";
#elif defined(__aarch64__)
    arch = "arm64";
#else
    arch = "unknown";
#endif
#else
    os = "Unknown";
    arch = "unknown";
#endif

    std::cout << "guchho doctor\n";
    std::cout << "  version        : " << VersionString() << '\n';
    std::cout << "  platform       : " << os << " " << arch << '\n';
    std::cout << "  terminal       : " << (info.is_tty ? "yes" : "no") << '\n';
    std::cout << "  width          : " << info.width << '\n';
    std::cout << "  height         : " << info.height << '\n';
    std::cout << "  color escapes  : " << (info.use_color_escapes ? "yes" : "no") << '\n';
    std::cout << "  NO_COLOR       : " << (logger::HasEnvironmentVariableValue("NO_COLOR") ? "set" : "not set") << '\n';
    std::cout << "  win cmd prompt : " << (logger::IsProbablyWindowsCommandPrompt() ? "yes" : "no") << '\n';
    return kExitOk;
}

// ---------------------------------------------------------------------------
// Command table
// ---------------------------------------------------------------------------

struct Command {
    std::string_view name;
    std::string_view summary;
    std::string_view usage;
    std::string_view arg_help;
    Handler          handler; // nullptr => not implemented yet
};

static const Command kCommands[] = {
    {"build",
     "Bundle your project for production",
     "guchho build [options]",
     "",
     nullptr},
    {"create",
     "Scaffold a new project",
     "guchho create <name>",
     "",
     nullptr},
    {"dev",
     "Run the development server with live reload",
     "guchho dev [options]",
     "",
     nullptr},
    {"doctor",
     "Diagnose your environment",
     "guchho doctor",
     "",
     HandleDoctor},
    {"fmt",
     "Format source files",
     "guchho fmt [options]",
     "",
     nullptr},
    {"init",
     "Create a guchho.config.json",
     "guchho init [options] [dir]",
     "  --force   Overwrite an existing config file.",
     HandleInit},
    {"lint",
     "Lint source files",
     "guchho lint [options]",
     "",
     nullptr},
    {"publish",
     "Publish the package",
     "guchho publish [options]",
     "",
     nullptr},
    {"test",
     "Run tests",
     "guchho test [options]",
     "",
     nullptr},
    {"upgrade",
     "Upgrade guchho to the latest version",
     "guchho upgrade",
     "",
     nullptr},
};

// ---------------------------------------------------------------------------
// Help text
// ---------------------------------------------------------------------------

std::string HelpText()
{
    std::string text;
    text += "guchho — Fast build system for web applications\n\n";
    text += "Usage:\n";
    text += "  guchho <command> [options]\n\n";
    text += "Commands:\n";

    for (const auto& cmd : kCommands) {
        text += "  ";
        text += cmd.name;
        text.append(8 > cmd.name.size() ? 8 - cmd.name.size() : 1, ' ');
        text += cmd.summary;
        text += '\n';
    }

    text += "\nOptions:\n";
    text += "  -h, --help      Show help\n";
    text += "  -v, --version   Show version\n\n";
    text += "Run 'guchho <command> --help' for details on a command.\n";
    return text;
}

std::string CommandHelpText(const std::string& command)
{
    for (const auto& cmd : kCommands) {
        if (cmd.name == command) {
            std::string text;
            text += "Usage: ";
            text += cmd.usage;
            text += "\n\n";
            text += cmd.summary;
            text += '\n';
            if (!cmd.arg_help.empty()) {
                text += "\nOptions:\n";
                text += cmd.arg_help;
                text += '\n';
            }
            if (!cmd.handler) {
                text += "\nThis command is not implemented yet.\n";
            }
            return text;
        }
    }
    return "";
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int Run(const std::vector<std::string>& args)
{
    if (args.empty()) {
        std::cout << HelpText();
        return kExitOk;
    }

    const std::string& first = args[0];

    if (first == "-h" || first == "--help") {
        std::cout << HelpText();
        return kExitOk;
    }

    if (first == "-v" || first == "--version") {
        std::cout << VersionString() << '\n';
        return kExitOk;
    }

    for (const auto& cmd : kCommands) {
        if (cmd.name == first) {
            for (const auto& arg : args) {
                if (arg == "-h" || arg == "--help") {
                    std::cout << CommandHelpText(std::string(cmd.name)) << '\n';
                    return kExitOk;
                }
            }

            std::vector<std::string> rest(args.begin() + 1, args.end());

            if (cmd.handler) {
                return cmd.handler(rest, args);
            }

            logger::PrintErrorToStderr(args, "The '" + std::string(cmd.name) + "' command is not implemented yet.");
            return kExitError;
        }
    }

    logger::PrintErrorToStderr(args, "Unknown command: \"" + first + "\"");
    logger::PrintErrorToStderr(args, "Run 'guchho --help' for usage.");
    return kExitUsage;
}

}
