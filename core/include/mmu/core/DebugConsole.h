#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace mmu {

// Platform-agnostic debug console. Commands and log output live here; the
// ImGui front-end only renders them and feeds user input back in.
class DebugConsole {
public:
    using Arguments = std::vector<std::string>;
    using CommandFn = std::function<void(DebugConsole&, const Arguments&)>;

    explicit DebugConsole(std::size_t maxLogLines = 512);

    void registerCommand(std::string name, std::string help, CommandFn fn);
    void log(std::string line);
    bool execute(std::string_view line);

    const std::vector<std::string>& logLines() const { return logLines_; }
    const std::vector<std::string>& history() const { return history_; }
    const std::vector<std::string>& commandNames() const { return commandNames_; }
    std::size_t historyCursor() const { return historyCursor_; }
    void setHistoryCursor(std::size_t cursor) { historyCursor_ = cursor; }

    std::string helpText() const;
    void clear();

private:
    struct Command {
        std::string name;
        std::string help;
        CommandFn fn;
    };

    std::vector<Command> commands_;
    std::vector<std::string> commandNames_;
    std::vector<std::string> logLines_;
    std::vector<std::string> history_;
    std::size_t historyCursor_ = 0;
    std::size_t maxLogLines_ = 512;
};

} // namespace mmu
