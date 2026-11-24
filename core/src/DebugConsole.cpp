#include "mmu/core/DebugConsole.h"

#include <algorithm>

namespace mmu {
namespace {

std::vector<std::string> splitWords(std::string_view line) {
    std::vector<std::string> words;
    std::size_t pos = 0;
    while (pos < line.size()) {
        while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) ++pos;
        const std::size_t start = pos;
        while (pos < line.size() && line[pos] != ' ' && line[pos] != '\t') ++pos;
        if (pos > start) words.emplace_back(line.substr(start, pos - start));
    }
    return words;
}

} // namespace

DebugConsole::DebugConsole(std::size_t maxLogLines) : maxLogLines_(maxLogLines) {}

void DebugConsole::registerCommand(std::string name, std::string help, CommandFn fn) {
    auto it = std::find_if(commands_.begin(), commands_.end(), [&](const Command& cmd) {
        return cmd.name == name;
    });
    if (it != commands_.end()) {
        it->help = std::move(help);
        it->fn = std::move(fn);
        return;
    }
    commands_.push_back({std::move(name), std::move(help), std::move(fn)});
    commandNames_.push_back(commands_.back().name);
    std::sort(commandNames_.begin(), commandNames_.end());
}

void DebugConsole::log(std::string line) {
    logLines_.push_back(std::move(line));
    while (logLines_.size() > maxLogLines_)
        logLines_.erase(logLines_.begin());
}

bool DebugConsole::execute(std::string_view line) {
    Arguments args = splitWords(line);
    if (args.empty()) return true;
    history_.push_back(std::string(line));
    historyCursor_ = history_.size();
    log("> " + std::string(line));

    for (Command& cmd : commands_) {
        if (cmd.name == args[0]) {
            args.erase(args.begin());
            cmd.fn(*this, args);
            return true;
        }
    }
    log("Unknown command: " + args[0] + " (try 'help')");
    return false;
}

std::string DebugConsole::helpText() const {
    std::string text = "Commands:\n";
    for (const std::string& name : commandNames_) {
        const Command* found = nullptr;
        for (const Command& cmd : commands_)
            if (cmd.name == name) { found = &cmd; break; }
        if (found) text += "  " + found->name + " - " + found->help + "\n";
    }
    return text;
}

void DebugConsole::clear() {
    logLines_.clear();
}

} // namespace mmu
