# ollamaCode - macOS Setup Guide

Complete guide for installing and using ollamaCode on macOS (Apple Silicon & Intel).

## Quick Install

```bash
cd /path/to/ollamaCode
chmod +x install.sh
./install.sh
```

The universal installer will detect macOS and handle everything automatically.

## Requirements

- **macOS**: 10.15 (Catalina) or later
- **Homebrew**: Package manager (auto-installed if missing)
- **Bash**: 4.0+ (pre-installed on macOS)
- **jq**: JSON processor (auto-installed)
- **Ollama**: Local LLM runtime (auto-installed with confirmation)

## Installation Methods

### Method 1: Universal Installer (Recommended)

The universal installer works on macOS, Linux, and other Unix systems:

```bash
git clone <repository-url>
cd ollamaCode
chmod +x install.sh
./install.sh
```

What it does:
1. Detects your operating system (macOS)
2. Checks for Homebrew, installs if missing
3. Installs dependencies (jq, curl)
4. Prompts to install Ollama if not present
5. Copies ollamacode to /usr/local/bin
6. Creates configuration directory at ~/.config/ollamacode
7. Tests the installation

### Method 2: macOS-Specific Installer

If you prefer the macOS-specific installer:

```bash
cd ollamaCode/macos
chmod +x install.sh
./install.sh
```

### Method 3: Manual Installation

For advanced users who want full control:

```bash
# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install jq curl

# Install Ollama
brew install ollama

# Install ollamaCode
cd ollamaCode
sudo cp bin/ollamacode /usr/local/bin/
sudo chmod +x /usr/local/bin/ollamacode

# Create config directory
mkdir -p ~/.config/ollamacode
```

## First-Time Setup

### 1. Start Ollama Service

```bash
# Start Ollama (if not already running)
brew services start ollama

# Or run in foreground
ollama serve
```

### 2. Pull a Model

```bash
# Pull a recommended model
ollama pull llama3

# Or pull other models
ollama pull mistral
ollama pull codellama
ollama pull qwen2.5-coder
```

### 3. Test ollamaCode

```bash
# Start interactive mode
ollamacode

# Or send a quick prompt
ollamacode "Hello, how are you?"
```

## Configuration

Config file location: `~/.config/ollamacode/config`

```bash
# Edit configuration
nano ~/.config/ollamacode/config
```

Default configuration:
```bash
MODEL=coreEchoFlux
OLLAMA_HOST=http://localhost:11434
TEMPERATURE=0.7
MAX_TOKENS=2048
SYSTEM_PROMPT="You are a helpful AI coding assistant."
```

## Usage Examples

### Interactive Mode

```bash
ollamacode
```

Commands available in interactive mode:
- `help` - Show available commands
- `models` - List installed models
- `use llama3` - Switch to llama3 model
- `temp 0.5` - Set temperature to 0.5
- `system "You are a DevOps expert"` - Change system prompt
- `history` - Show conversation history
- `config` - Show current configuration
- `clear` - Clear screen
- `exit` or `quit` - Exit ollamacode

### Single Prompt Mode

```bash
# Simple query
ollamacode "Explain Docker containers"

# With specific model
ollamacode -m llama3 "Write a Python script to parse JSON"

# With custom temperature
ollamacode -t 0.2 "Generate secure password hashing code"

# From file
ollamacode -f prompt.txt

# Custom system prompt
ollamacode --system "You are a security expert" "Review this code for vulnerabilities"
```

## Platform-Specific Notes

### Apple Silicon (M1/M2/M3) vs Intel

ollamaCode works on both Apple Silicon and Intel Macs. Ollama automatically optimizes for your hardware:

- **Apple Silicon**: Uses Metal for GPU acceleration
- **Intel**: Uses CPU-optimized inference

### Permissions

If you see permission errors:

```bash
# Make sure /usr/local/bin is writable
sudo chown -R $(whoami) /usr/local/bin

# Or reinstall with sudo
sudo ./install.sh
```

### PATH Configuration

If `ollamacode` command is not found, add to your shell profile:

```bash
# For bash (add to ~/.bash_profile or ~/.bashrc)
export PATH="/usr/local/bin:$PATH"

# For zsh (add to ~/.zshrc)
export PATH="/usr/local/bin:$PATH"

# Reload shell configuration
source ~/.zshrc  # or ~/.bash_profile
```

## Troubleshooting

### Ollama Not Running

```bash
# Check if Ollama is running
pgrep ollama

# Start Ollama
brew services start ollama

# Or run manually
ollama serve
```

### Connection Refused Error

```bash
# Check Ollama status
curl http://localhost:11434/api/tags

# Restart Ollama
brew services restart ollama
```

### Model Not Found

```bash
# List installed models
ollama list

# Pull missing model
ollama pull llama3
```

### sed Command Errors

The ollamacode script is now cross-platform compatible. The `sed -i` command works correctly on both macOS and Linux.

### Homebrew Not Found

```bash
# Install Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Follow the post-install instructions to add Homebrew to PATH
```

## Uninstallation

To remove ollamaCode from your system:

```bash
# Remove binary
sudo rm /usr/local/bin/ollamacode

# Remove configuration (optional)
rm -rf ~/.config/ollamacode

# Remove Ollama (optional)
brew uninstall ollama
brew services stop ollama
```

## Advanced Configuration

### Using Different Ollama Host

If you're running Ollama on a different machine or port:

```bash
# Temporary
OLLAMA_HOST=http://192.168.1.100:11434 ollamacode

# Permanent (edit config)
nano ~/.config/ollamacode/config
# Change: OLLAMA_HOST=http://192.168.1.100:11434
```

### Custom Models

```bash
# Create a custom model with Modelfile
cat > Modelfile <<EOF
FROM llama3
SYSTEM You are a helpful coding assistant specialized in Python.
PARAMETER temperature 0.5
EOF

ollama create my-python-assistant -f Modelfile

# Use in ollamacode
ollamacode -m my-python-assistant "Help me with Python"
```

### Performance Tuning

For faster responses on macOS:

```bash
# Use smaller models for quick tasks
ollamacode -m llama3:8b "Quick question"

# Use larger models for complex tasks
ollamacode -m llama3:70b "Complex architectural decision"

# Adjust context window in config
MAX_TOKENS=4096  # Larger context, slower
MAX_TOKENS=1024  # Smaller context, faster
```

## Integration with Other Tools

### VS Code Integration

Add to VS Code tasks.json:

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Ask ollamaCode",
      "type": "shell",
      "command": "ollamacode",
      "args": ["${input:prompt}"]
    }
  ],
  "inputs": [
    {
      "id": "prompt",
      "type": "promptString",
      "description": "Enter your prompt"
    }
  ]
}
```

### Terminal Alias

Add to ~/.zshrc or ~/.bash_profile:

```bash
# Quick alias
alias ai='ollamacode'
alias ask='ollamacode'

# Code review alias
alias review='ollamacode "Review this code for best practices and potential issues:"'
```

## Updates

To update ollamaCode:

```bash
# Pull latest changes
cd /path/to/ollamaCode
git pull

# Reinstall
./install.sh

# Update Ollama
brew upgrade ollama

# Update models
ollama pull llama3
```

## Getting Help

- **ollamaCode Issues**: Check GitHub repository
- **Ollama Issues**: https://github.com/ollama/ollama
- **Homebrew Issues**: https://brew.sh

## License

MIT License - See LICENSE file for details
