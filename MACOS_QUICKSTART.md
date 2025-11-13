# ollamaCode - macOS Quick Start

Get up and running with ollamaCode on macOS in under 5 minutes!

## One-Line Install

```bash
cd /path/to/ollamaCode && chmod +x install.sh && ./install.sh
```

## What Gets Installed

- ✅ Homebrew (if needed)
- ✅ jq (JSON processor)
- ✅ Ollama (local LLM runtime)
- ✅ ollamacode CLI tool

## First Commands

```bash
# Start Ollama (first time only)
brew services start ollama

# Pull a model
ollama pull llama3

# Start ollamacode
ollamacode
```

## Interactive Mode Commands

```
You> help           # Show all commands
You> models         # List available models
You> use llama3     # Switch model
You> temp 0.5       # Set temperature
You> config         # Show settings
You> exit           # Quit
```

## Common Use Cases

```bash
# Quick question
ollamacode "What is Docker?"

# Code generation
ollamacode "Write a Python function to sort a list"

# Code review
ollamacode -f mycode.py

# Use specific model
ollamacode -m codellama "Explain this bug"

# Lower temperature (more focused)
ollamacode -t 0.2 "Generate production code"

# Higher temperature (more creative)
ollamacode -t 1.2 "Brainstorm feature ideas"
```

## Recommended Models

```bash
# General purpose
ollama pull llama3           # Meta's Llama 3 (8B)
ollama pull llama3:70b       # Larger, more capable

# Coding
ollama pull codellama        # Code-specialized
ollama pull qwen2.5-coder    # Excellent for coding

# Fast & efficient
ollama pull mistral          # Good balance
ollama pull phi3             # Very fast, smaller
```

## Keyboard Shortcuts (in interactive mode)

- `Ctrl+C` - Cancel current input
- `Ctrl+D` - Exit ollamacode
- `Ctrl+L` - Clear screen (or type `clear`)
- `↑/↓` - Browse command history (in some terminals)

## File Locations

```bash
# Config
~/.config/ollamacode/config

# History
~/.config/ollamacode/history

# Binary
/usr/local/bin/ollamacode
```

## Troubleshooting

### "ollama not running"
```bash
brew services start ollama
# or
ollama serve
```

### "model not found"
```bash
ollama list          # See installed models
ollama pull llama3   # Install a model
```

### "command not found: ollamacode"
```bash
# Add to PATH (in ~/.zshrc or ~/.bash_profile)
export PATH="/usr/local/bin:$PATH"
source ~/.zshrc
```

## Pro Tips

### Alias for Quick Access
```bash
# Add to ~/.zshrc
alias ai='ollamacode'
alias ask='ollamacode'

# Now use:
ai "your question"
```

### Different Models for Different Tasks

```bash
# Code tasks
ollamacode -m codellama "Write a function..."

# General Q&A
ollamacode -m llama3 "Explain..."

# Quick answers
ollamacode -m phi3 "What is..."
```

### Save Your Favorite Settings

Edit `~/.config/ollamacode/config`:
```bash
MODEL=codellama                    # Your preferred model
TEMPERATURE=0.7                    # Default creativity
SYSTEM_PROMPT="You are a Python expert"  # Custom behavior
```

## Next Steps

- Read [MACOS_SETUP.md](MACOS_SETUP.md) for detailed documentation
- Try different models and temperatures
- Integrate with your IDE/editor
- Create custom aliases and shortcuts

## Getting Help

```bash
ollamacode --help    # Show all options
ollamacode -v        # Show version
```

## Update ollamaCode

```bash
cd /path/to/ollamaCode
git pull
./install.sh
```

## Uninstall

```bash
# Remove ollamacode
sudo rm /usr/local/bin/ollamacode

# Remove config (optional)
rm -rf ~/.config/ollamacode

# Remove Ollama (optional)
brew uninstall ollama
```

---

**Ready to code?** Run `ollamacode` and start chatting with AI! 🚀
