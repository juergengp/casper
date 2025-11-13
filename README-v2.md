# ollamaCode v2.0

**Claude Code-style Interactive CLI for Ollama with Tool Calling**

ollamaCode brings Claude Code's powerful AI assistant capabilities to your local Ollama installation. Unlike basic chat interfaces, ollamaCode empowers your AI with **real tools** - it can execute commands, read files, write code, search your codebase, and more - just like Claude Code does.

## 🚀 What's New in v2.0

**Tool Calling Support** - The AI can now actively use tools to help you:
- Execute bash commands
- Read, write, and edit files
- Search for files with glob patterns
- Search code with grep
- Iteratively solve complex multi-step problems

This is a **major upgrade** from v1.0's simple chat interface. Now your AI can actually *do things* on your system.

## ✨ Key Features

### 🔧 **AI Tool Execution**
Your AI assistant can use these tools autonomously:

| Tool | Purpose | Example Use Case |
|------|---------|------------------|
| **Bash** | Execute shell commands | List files, check processes, run builds |
| **Read** | Read file contents | Review code, check configs |
| **Write** | Create new files | Generate scripts, create configs |
| **Edit** | Modify existing files | Update code, fix bugs |
| **Glob** | Find files by pattern | Locate all Python files, find configs |
| **Grep** | Search in files | Find function definitions, search logs |

### 🛡️ **Safety First**
- **Safe Mode** (default): Only approved commands can run
- **Command Confirmation**: Review each command before execution
- **Auto-Approve Mode**: For trusted automation tasks
- **Configurable Allowlist**: Customize which commands are safe

### 💬 **Interactive Experience**
- Natural conversation with your AI
- AI explains reasoning before taking action
- Iterative problem solving (AI can call multiple tools)
- Real-time output and statistics
- Session history tracking

### 🎯 **Network Ollama Support**
- Connect to Ollama running anywhere on your network
- Perfect for shared AI infrastructure
- Configurable via `OLLAMA_HOST` environment variable

## 📦 Installation

### Quick Install (Fedora/RHEL/CentOS)

```bash
# Build the RPM
cd /root/ollamaCode
./build-rpm.sh

# Install
sudo rpm -ivh ~/rpmbuild/RPMS/noarch/ollamacode-2.0.0-1.*.noarch.rpm
```

### Manual Installation

```bash
# Install dependencies
sudo dnf install -y curl jq ollama

# Copy files
sudo cp bin/ollamacode-new /usr/local/bin/ollamacode
sudo mkdir -p /usr/local/lib/ollamacode
sudo cp lib/*.sh /usr/local/lib/ollamacode/
sudo chmod +x /usr/local/bin/ollamacode
```

### Network Ollama Setup

If your Ollama is running on a different machine:

```bash
# One-time setup
echo 'OLLAMA_HOST=http://your-server:11434' >> ~/.config/ollamacode/config

# Or per-session
OLLAMA_HOST=http://192.168.1.100:11434 ollamacode
```

## 🎮 Usage

### Interactive Mode

Start an interactive session where the AI can use tools:

```bash
ollamacode
```

**Example conversation:**

```
You> List all Python files in this directory and count the lines of code

🤔 Thinking...

I'll help you find all Python files and count their lines. Let me start by finding the files.

🔧 Executing 2 tool(s)...

═══════════════════════════════════════
Tool 1/2: Glob
═══════════════════════════════════════
[Tool: Glob]
Pattern: *.py
Path: .

=== Matching Files ===
./app.py
./utils.py
./config.py
====================

═══════════════════════════════════════
Tool 2/2: Bash
═══════════════════════════════════════
[Tool: Bash]
Description: Count lines in Python files
Command: wc -l *.py

Execute? (y/n/always): y

Executing...
✓ Success

=== Output ===
  150 app.py
  89 utils.py
  45 config.py
  284 total
==============

📊 Tool execution completed. Processing results...

I found 3 Python files with a total of 284 lines of code:
- app.py: 150 lines
- utils.py: 89 lines
- config.py: 45 lines

───────────────────────────────────────
⏱ Duration: 3s
```

### Single Command Mode

Execute a single request and exit:

```bash
ollamacode "Read the README.md file and summarize it"
```

### Advanced Options

```bash
# Use specific model
ollamacode -m llama3 "Your prompt"

# Auto-approve all tool executions (for automation)
ollamacode -a "Run the test suite"

# Disable safe mode (allow any command)
ollamacode --unsafe "System maintenance task"

# Lower temperature for more focused responses
ollamacode -t 0.2 "Debug this code"
```

## 🎯 Real-World Examples

### Example 1: Code Analysis

```bash
ollamacode "Find all TODO comments in Python files and create a summary report"
```

The AI will:
1. Use `Glob` to find all .py files
2. Use `Grep` to search for TODO comments
3. Analyze the results
4. Use `Write` to create a report file

### Example 2: System Diagnostics

```bash
ollamacode "Check disk usage and list the 10 largest directories"
```

The AI will:
1. Use `Bash` to run `df -h`
2. Use `Bash` to run `du -sh * | sort -h | tail -10`
3. Analyze and explain the results

### Example 3: Code Refactoring

```bash
ollamacode "In config.py, change all occurrences of port 8080 to port 9090"
```

The AI will:
1. Use `Read` to check the file
2. Use `Edit` to make the changes
3. Confirm the modifications

### Example 4: Project Setup

```bash
ollamacode "Create a basic Python Flask project structure with main.py, config.py, and requirements.txt"
```

The AI will:
1. Use `Write` to create main.py with Flask boilerplate
2. Use `Write` to create config.py
3. Use `Write` to create requirements.txt with dependencies

## ⚙️ Configuration

Configuration file: `~/.config/ollamacode/config`

```bash
# Model to use (must be pulled in Ollama)
MODEL=coreEchoFlux

# Ollama API host (change for network installations)
OLLAMA_HOST=http://localhost:11434

# Generation parameters
TEMPERATURE=0.7
MAX_TOKENS=4096

# Safety settings
SAFE_MODE=true
AUTO_APPROVE=false
```

### Allowed Commands (Safe Mode)

Edit the allowlist in `/usr/local/lib/ollamacode/tool_executor.sh`:

```bash
ALLOWED_COMMANDS=("ls" "cat" "grep" "find" "git" "docker" "pwd" "date" ...)
```

## 🔧 Interactive Commands

While in interactive mode, you can use these commands:

| Command | Description |
|---------|-------------|
| `help` | Show available commands |
| `models` | List available Ollama models |
| `use MODEL` | Switch to a different model |
| `temp NUM` | Set temperature (0.0-2.0) |
| `safe on\|off` | Toggle safe mode |
| `auto on\|off` | Toggle auto-approve |
| `config` | Show current configuration |
| `clear` | Clear the screen |
| `exit` / `quit` | Exit ollamaCode |

## 🧠 How It Works

ollamaCode teaches your Ollama model about available tools through a detailed system prompt. When you make a request:

1. **AI Analyzes**: The model determines which tools (if any) are needed
2. **AI Outputs**: Tool calls in structured XML format
3. **Parser Extracts**: Tool calls are parsed from the response
4. **Executor Runs**: Tools are executed with safety checks
5. **Results Return**: Tool output is fed back to the AI
6. **AI Continues**: The model analyzes results and either uses more tools or provides the final answer

This creates an **agentic loop** where the AI can solve complex multi-step problems autonomously.

## 🆚 Comparison with Claude Code

| Feature | Claude Code | ollamaCode |
|---------|-------------|------------|
| AI Provider | Anthropic Cloud | Local Ollama |
| Tool Calling | ✅ Native | ✅ Implemented |
| Bash Execution | ✅ | ✅ |
| File Operations | ✅ | ✅ |
| Code Search | ✅ | ✅ |
| Cost | $$ (API) | Free (local) |
| Privacy | Cloud | 100% Local |
| Network Install | N/A | ✅ Supported |

## 🤝 Model Compatibility

Works best with instruction-tuned models that follow prompts well:

**Recommended:**
- llama3 / llama3.1
- codellama
- mistral
- mixtral
- custom fine-tuned models

**Tips:**
- Use higher context models (8k+) for complex tasks
- Lower temperature (0.2-0.5) for code tasks
- Higher temperature (0.7-1.0) for creative tasks

## 🐛 Troubleshooting

### "ollama is not running"
```bash
# Start Ollama service
ollama serve

# Or for network installations, check your OLLAMA_HOST
curl http://your-server:11434/api/tags
```

### "Command not in allowed list"
```bash
# Option 1: Disable safe mode
SAFE_MODE=false ollamacode

# Option 2: Add command to allowlist
# Edit /usr/local/lib/ollamacode/tool_executor.sh
```

### AI not using tools properly
- Try a different model (some models follow instructions better)
- Use lower temperature for more deterministic tool usage
- Provide more explicit instructions

## 📝 License

MIT License - See LICENSE file

## 👤 Author

Core.at - support@core.at

## 🙏 Credits

Inspired by Anthropic's Claude Code CLI

## 🔗 Links

- Ollama: https://ollama.ai
- Claude Code: https://claude.ai/code
- GitHub: https://github.com/core-at/ollamacode

---

**Ready to give your AI real capabilities?** Start with `ollamacode` and watch your local LLM become a true coding assistant!
