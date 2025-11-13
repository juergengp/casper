# Quick Start Guide - ollamaCode v2.0

This guide will get you up and running with ollamaCode in 5 minutes.

## Prerequisites

1. **Ollama installed and running**
   ```bash
   # Check if Ollama is installed
   ollama --version

   # If not installed, install it:
   curl -fsSL https://ollama.ai/install.sh | sh

   # Start Ollama service
   ollama serve &
   ```

2. **Pull a model**
   ```bash
   # Pull a recommended model
   ollama pull llama3
   # or
   ollama pull mistral
   ```

3. **Install ollamaCode**
   ```bash
   cd /root/ollamaCode
   chmod +x bin/ollamacode-new
   sudo cp bin/ollamacode-new /usr/local/bin/ollamacode
   sudo mkdir -p /usr/local/lib/ollamacode
   sudo cp lib/*.sh /usr/local/lib/ollamacode/
   ```

## First Run

### Test Installation

```bash
# Check version
ollamacode --version

# View help
ollamacode --help

# Start interactive mode
ollamacode
```

### Your First Request

In interactive mode, try this simple request:

```
You> What files are in the current directory?
```

The AI should:
1. Use the **Bash** tool to run `ls -la`
2. Show you the output
3. Explain what it found

When prompted "Execute? (y/n/always):", type `y` and press Enter.

## Configuration

### Set Your Preferred Model

```bash
# Edit config
nano ~/.config/ollamacode/config

# Change this line:
MODEL=llama3
```

### Connect to Network Ollama

If your Ollama is on another machine:

```bash
echo 'OLLAMA_HOST=http://192.168.1.100:11434' >> ~/.config/ollamacode/config
```

## Try These Examples

### Example 1: Simple File Operation

```bash
ollamacode "Create a file called hello.txt with the text 'Hello from ollamaCode'"
```

Expected behavior:
- AI uses **Write** tool
- Creates hello.txt
- Confirms creation

### Example 2: Code Search

```bash
ollamacode "Find all shell scripts in the current directory"
```

Expected behavior:
- AI uses **Glob** tool with pattern `*.sh`
- Lists all matching files

### Example 3: System Info

```bash
ollamacode "Tell me about this system - OS, memory, disk space"
```

Expected behavior:
- AI uses **Bash** tool multiple times
- Runs commands like `uname`, `free`, `df`
- Summarizes the information

## Understanding Tool Execution

When the AI wants to use a tool, you'll see:

```
🔧 Executing 1 tool(s)...

═══════════════════════════════════════
Tool 1/1: Bash
═══════════════════════════════════════
[Tool: Bash]
Description: List directory contents
Command: ls -la

Execute? (y/n/always):
```

Your options:
- **y** - Execute this once
- **n** - Skip this tool
- **always** - Auto-approve all tools this session

## Safety Features

### Safe Mode (Default)

By default, only these commands are allowed:
- ls, cat, head, tail, grep, find
- git, docker, kubectl
- pwd, whoami, date
- ps, df, du, wc

### Disable Safe Mode

For trusted operations:
```bash
ollamacode --unsafe "Your request"
```

### Auto-Approve Mode

For automation:
```bash
ollamacode -a "Run all tests"
```

## Common Patterns

### Pattern 1: Read → Analyze → Report

```
You> Read the config.json file and tell me if there are any security issues
```

AI will:
1. Read the file
2. Analyze contents
3. Report findings

### Pattern 2: Search → Filter → Summarize

```
You> Find all Python files and tell me which ones import requests
```

AI will:
1. Glob for *.py files
2. Grep for "import requests"
3. Summarize results

### Pattern 3: Create → Modify → Verify

```
You> Create a Python script that prints "Hello World", then make it print "Hello Universe" instead
```

AI will:
1. Write initial script
2. Edit the script
3. Show the result

## Interactive Mode Commands

While in ollamaCode interactive mode:

```
models              # List available models
use llama3         # Switch model
temp 0.5           # Lower temperature for code tasks
safe off           # Disable safe mode
auto on            # Enable auto-approve
config             # Show current settings
clear              # Clear screen
exit               # Quit
```

## Tips for Success

### 1. Start Simple
Begin with basic requests to understand how tool calling works.

### 2. Be Specific
❌ "Check the files"
✅ "List all Python files in the src directory"

### 3. Use Lower Temperature for Code
```bash
ollamacode -t 0.3 "Fix the syntax error in app.py"
```

### 4. Chain Commands
In interactive mode, build up complex tasks step by step.

### 5. Review Before Approving
Always check commands before approving, especially:
- File deletions (rm)
- System modifications
- Network operations

## Troubleshooting

### "ollama is not running"

```bash
# Check status
ps aux | grep ollama

# Start if not running
ollama serve &

# Wait a few seconds
sleep 3

# Test connection
curl http://localhost:11434/api/tags
```

### "Model not found"

```bash
# List available models
ollama list

# Pull the model from config
ollama pull coreEchoFlux
# or
ollama pull llama3
```

### "Command not in allowed list"

Either:
1. Use unsafe mode: `ollamacode --unsafe "..."`
2. Add command to allowlist in `/usr/local/lib/ollamacode/tool_executor.sh`

### AI not using tools

Some models are better at following instructions. Try:
- llama3 (recommended)
- mistral
- codellama

Lower the temperature:
```bash
ollamacode -t 0.2 "Your request"
```

## Next Steps

1. **Read the full README**: `cat README-v2.md`
2. **Explore examples**: `cat docs/EXAMPLES.md`
3. **Customize configuration**: `nano ~/.config/ollamacode/config`
4. **Try automation**: Create scripts that call ollamacode

## Get Help

- Documentation: README-v2.md
- Examples: docs/EXAMPLES.md
- Issues: https://github.com/core-at/ollamacode/issues
- Email: support@core.at

---

**You're ready to go!** Start with `ollamacode` and let your AI assistant help you code.

```bash
ollamacode
```

🚀 Happy coding!
