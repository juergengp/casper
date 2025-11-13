# ollamaCode

Interactive CLI for Ollama - Inspired by Claude Code

**Platforms**: macOS (Intel & Apple Silicon) | Linux (Fedora, RHEL, CentOS, Debian, Ubuntu)

**Available in two versions:**
- 🐚 **Bash** - No compilation needed, install in seconds
- ⚡ **C++** - High performance, requires compilation ([see cpp/](cpp/))

**Not sure which?** See [VERSIONS.md](VERSIONS.md) for comparison. Most users should start with the Bash version below.

## Features

- 🚀 **Interactive Chat Mode** - Natural conversation with your local LLMs
- 🔧 **Tool Execution** - AI can execute commands on your machine (with safety controls)
- 🎨 **Syntax Highlighting** - Beautiful terminal output
- 📊 **Statistics** - Token count, speed, and timing information
- 🔄 **Multi-Model Support** - Switch between models on the fly
- 💾 **Session Management** - Save and load conversations
- ⚙️ **Configuration** - Customizable settings and prompts
- 🔐 **Safe Mode** - Command execution with allowlist and confirmations

## Installation

### Universal Installer (All Platforms)

```bash
# Works on macOS, Fedora, RHEL, CentOS, Debian, Ubuntu
cd ollamaCode
chmod +x install.sh
./install.sh
```

### Fedora / RHEL / CentOS

```bash
# Install dependencies
sudo dnf install -y curl jq ollama

# Install ollamaCode RPM
sudo rpm -ivh ollamacode-1.0.0-1.fc41.noarch.rpm

# Or build from source
cd /root/ollamaCode
./build-rpm.sh
sudo rpm -ivh ~/rpmbuild/RPMS/noarch/ollamacode-1.0.0-1.fc41.noarch.rpm
```

### macOS

```bash
# Option 1: Universal installer (recommended)
cd /root/ollamaCode
chmod +x install.sh
./install.sh

# Option 2: macOS-specific installer
cd /root/ollamaCode/macos
chmod +x install.sh
./install.sh
```

The installer will automatically:
- Install Homebrew (if not present)
- Install jq and curl
- Install Ollama (with confirmation)
- Set up ollamacode in /usr/local/bin
- Create configuration directory

**For detailed macOS instructions, see:**
- [MACOS_QUICKSTART.md](MACOS_QUICKSTART.md) - Get started in 5 minutes
- [MACOS_SETUP.md](MACOS_SETUP.md) - Comprehensive guide

### Manual Installation

```bash
# Install dependencies
# Fedora/RHEL:
sudo dnf install -y curl jq

# macOS:
brew install curl jq

# Install ollama
curl -fsSL https://ollama.ai/install.sh | sh

# Install ollamaCode
cd /root/ollamaCode
sudo cp bin/ollamacode /usr/local/bin/
sudo chmod +x /usr/local/bin/ollamacode
```

## Quick Start

```bash
# Start interactive mode
ollamacode

# Single prompt
ollamacode "Explain Docker containers"

# Use specific model
ollamacode -m llama3 "Hello"

# Read prompt from file
ollamacode -f prompt.txt

# Custom system prompt
ollamacode --system "You are a DevOps expert" "Setup nginx"
```

## Interactive Commands

| Command | Description |
|---------|-------------|
| `help` | Show available commands |
| `models` | List available models |
| `use MODEL` | Switch to different model |
| `temp NUM` | Set temperature (0.0-2.0) |
| `system PROMPT` | Set system prompt |
| `history` | Show conversation history |
| `config` | Show current configuration |
| `clear` | Clear screen |
| `exit`, `quit` | Exit ollamaCode |

## Tool Functions

ollamaCode includes built-in tools that the AI can use:

### Available Tools

- **bash** - Execute shell commands
- **read** - Read file contents
- **write** - Write to files
- **glob** - Find files by pattern
- **grep** - Search in files
- **git** - Git operations (status, diff, log, add)
- **sysinfo** - System information
- **docker** - Docker operations
- **ps** - Process information
- **netstat** - Network status

### Safety Features

- **Safe Mode** (default): Only allows pre-approved commands
- **Confirmation Required**: User must approve each command
- **Allowlist**: Configurable list of allowed commands

### Disable Safe Mode

```bash
# Allow all commands (use with caution!)
SAFE_MODE=false ollamacode
```

## Configuration

Configuration file: `~/.config/ollamacode/config`

```bash
# Model to use
MODEL=coreEchoFlux

# Ollama API host
OLLAMA_HOST=http://localhost:11434

# Generation parameters
TEMPERATURE=0.7
MAX_TOKENS=2048

# System prompt
SYSTEM_PROMPT="You are a helpful AI coding assistant."
```

## Examples

### Interactive Session

```bash
$ ollamacode
   ____  _ _                       ____          _
  / __ \| | | __ _ _ __ ___   __ _/ ___|___   __| | ___
 | |  | | | |/ _` | '_ ` _ \ / _` | |   / _ \ / _` |/ _ \
 | |__| | | | (_| | | | | | | (_| | |__| (_) | (_| |  __/
  \____/|_|_|\__,_|_| |_| |_|\__,_|\____\___/ \__,_|\___|

Interactive CLI for Ollama - Version 1.0.0
Type 'help' for commands, 'exit' to quit

Current Configuration:
  Model:        coreEchoFlux
  Host:         http://localhost:11434
  Temperature:  0.7
  Max Tokens:   2048

You> List all Docker containers

Thinking...

<AI response here>

───────────────────────────────────────
Stats: 3s | 150 tokens | 50.0 tok/s
```

### Direct Prompt

```bash
$ ollamacode "Write a Python script to read CSV files"
Thinking...

Here's a Python script to read CSV files:

```python
import csv

def read_csv(filename):
    with open(filename, 'r') as file:
        csv_reader = csv.reader(file)
        for row in csv_reader:
            print(row)

read_csv('data.csv')
```
```

### With Custom Settings

```bash
$ ollamacode -m llama3 -t 0.2 "Explain recursion"
```

## Building RPM Package

```bash
cd /root/ollamaCode
./build-rpm.sh
```

The RPM will be created in `~/rpmbuild/RPMS/noarch/`

## Requirements

- Bash 4.0+
- curl
- jq
- ollama

## License

MIT License

## Author

Core.at - <support@core.at>

## Contributing

Contributions welcome! Please submit pull requests or open issues.

## Changelog

### v1.0.0 (2025-10-16)
- Initial release
- Interactive chat mode
- Tool execution with safety controls
- Multi-model support
- Configuration management
- RPM package for Fedora
- macOS installation script
- Session history
- Statistics display
