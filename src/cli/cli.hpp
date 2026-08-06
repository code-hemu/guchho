#pragma once

#include <string>
#include <vector>

namespace guchho::cli {

// Exit codes used across the CLI.
constexpr int kExitOk    = 0;
constexpr int kExitError = 1;
constexpr int kExitUsage = 2;

// Runs the CLI with `args` (program name excluded) and returns the process
// exit code. This is the testable entry point used by main().
int Run(const std::vector<std::string>& args);

// Returns the version banner, e.g. "guchho 0.1.0".
std::string VersionString();

// Returns the top-level help text.
std::string HelpText();

// Returns the help text for a single command (empty if unknown).
std::string CommandHelpText(const std::string& command);

}
