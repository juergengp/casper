# Session Management in ollamaCode C++

## Overview

The C++ version of ollamaCode includes comprehensive session management capabilities that enable:

1. **Session Persistence** - Save full conversations to SQLite database
2. **Session Resume** - Continue previous sessions with full context
3. **Auto-Documentation** - Generate TODO.md and DECISIONS.md from sessions
4. **State Tracking** - Track all file modifications and tool executions
5. **Session Summaries** - AI-generated summaries of what was accomplished

## Architecture

### Database Schema

Session data is stored in `~/.config/ollamacode/sessions/sessions.db` with the following tables:

```sql
-- Main sessions table
CREATE TABLE sessions (
    session_id TEXT PRIMARY KEY,
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL,
    model TEXT NOT NULL,
    working_directory TEXT,
    summary TEXT,
    is_active INTEGER DEFAULT 1
);

-- Conversation messages
CREATE TABLE messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    session_id TEXT NOT NULL,
    role TEXT NOT NULL,              -- 'user', 'assistant', 'tool'
    content TEXT NOT NULL,
    timestamp TEXT NOT NULL,
    FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
);

-- Tool executions
CREATE TABLE tool_executions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    session_id TEXT NOT NULL,
    tool_name TEXT NOT NULL,
    parameters TEXT,                 -- JSON
    output TEXT,
    exit_code INTEGER,
    timestamp TEXT NOT NULL,
    FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
);

-- File modifications
CREATE TABLE file_modifications (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    session_id TEXT NOT NULL,
    file_path TEXT NOT NULL,
    operation TEXT NOT NULL,         -- 'read', 'write', 'edit'
    timestamp TEXT NOT NULL,
    FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
);
```

### Key Classes

#### `SessionManager`
- Manages all session operations
- Handles database persistence
- Tracks conversation context
- Generates documentation

#### `Session`
- Represents a complete session
- Contains messages, tool executions, file modifications
- Serializable to JSON and Markdown

#### `Message`, `ToolExecution`, `FileModification`
- Data structures for session components
- JSON serialization support

## Usage

### Command Line Options

```bash
# List all sessions
ollamacode --list-sessions
ollamacode -l

# Resume last active session
ollamacode --resume
ollamacode -r

# Resume specific session
ollamacode --resume session_20250117_143000_1234
ollamacode -r session_20250117_143000_1234

# Export current session
ollamacode --export json        # Export to JSON
ollamacode --export markdown    # Export to Markdown

# Combined usage
ollamacode -r -m llama3 "Continue where we left off"
```

### Interactive Commands

During an interactive session, you can use:

```
session info           # Show current session statistics
export json            # Export session to session.json
export md              # Export session to session.md
generate docs          # Generate TODO.md, DECISIONS.md, SESSION_REPORT.md
```

### Session Lifecycle

#### 1. **Session Creation**
When you start ollamaCode, a new session is automatically created:

```cpp
std::string session_id = session_manager_->createSession(model, working_dir);
// Creates: session_20250117_143000_1234
```

#### 2. **Message Tracking**
All conversation is automatically tracked:

```cpp
// User messages
session_manager_->addUserMessage("List all Python files");

// AI responses
session_manager_->addAssistantMessage("I'll use the Glob tool...");

// Tool outputs
session_manager_->addToolMessage("Glob", "file1.py\nfile2.py");
```

#### 3. **Tool Execution Tracking**
Every tool execution is recorded:

```cpp
session_manager_->recordToolExecution(
    "Glob",                                 // tool name
    {{"pattern", "*.py"}},                  // parameters (JSON)
    "file1.py\nfile2.py\nfile3.py",        // output
    0                                        // exit code
);
```

#### 4. **File Modification Tracking**
File operations are tracked automatically:

```cpp
session_manager_->recordFileModification(
    "/path/to/file.txt",   // file path
    "write"                // operation: read, write, edit
);
```

#### 5. **Session Close**
On exit, the session is automatically closed:

```cpp
// Generate AI summary
std::string summary = /* AI-generated summary */;
session_manager_->generateSessionSummary(summary);

// Generate documentation
session_manager_->generateTodoMd();
session_manager_->generateDecisionsMd();

// Session saved and marked as inactive
session_manager_->closeSession();
```

## Features

### 1. Session Persistence

**What it does:**
- Saves all conversation messages to database
- Stores tool executions with parameters and results
- Tracks file modifications
- Preserves session metadata (model, timestamps, working directory)

**How it works:**
```cpp
// Automatically saves after each message/tool execution
session_manager_->saveSession();

// Or export to standalone file
session_manager_->exportSessionToJson("backup.json");
```

### 2. Session Resume

**What it does:**
- Loads previous session with full conversation history
- Restores context for AI to continue seamlessly
- Shows summary and recent messages on resume

**How it works:**
```bash
$ ollamacode --resume

✓ Resumed session: session_20250117_143000_1234

Session Info:
  Created: 2025-01-17 14:30:00
  Messages: 23
  Tools executed: 8
  Files modified: 3

Summary:
Implemented session management features for ollamaCode C++ version...

Recent Conversation:
user: How do I resume a session?
assistant: You can resume by using the --resume flag...
```

**Context restoration:**
```cpp
// Get recent messages for context
auto context = session_manager_->getConversationContext(10);  // last 10 messages

// Include in prompt to AI
std::string prompt = build_prompt_with_context(user_input, context);
```

### 3. Auto-Documentation

**What it does:**
- Generates `TODO.md` with tasks extracted from conversation
- Creates `DECISIONS.md` documenting technical decisions and actions
- Produces `SESSION_REPORT.md` with full session details

**How it works:**

**TODO.md:**
```markdown
# TODO List

**Generated from session:** session_20250117_143000_1234
**Date:** 2025-01-17 15:45:00

## Tasks

- [ ] Extract from conversation: 2025-01-17 14:35:00
- [ ] Extract from conversation: 2025-01-17 14:42:00

## Modified Files

- `/root/ollamaCode/cpp/src/session_manager.cpp`
- `/root/ollamaCode/cpp/include/session_manager.h`

## Tools Used

- Write
- Edit
- Bash
```

**DECISIONS.md:**
```markdown
# Technical Decisions

**Session:** session_20250117_143000_1234
**Date:** 2025-01-17 14:30:00

## Context

**Model:** llama3
**Working Directory:** /root/ollamaCode

## Summary

Implemented comprehensive session management...

## Key Actions

### Files Modified (3)

- **write**: `/root/ollamaCode/cpp/src/session_manager.cpp` (2025-01-17 14:35:00)
- **edit**: `/root/ollamaCode/cpp/include/cli.h` (2025-01-17 14:40:00)

### Tools Executed (8)

- **Write** (2025-01-17 14:35:00)
- **Edit** (2025-01-17 14:40:00)
- **Bash** (2025-01-17 14:45:00)
```

### 4. State Tracking

**What it does:**
- Records every file that was read, written, or edited
- Tracks all tool executions with full parameters
- Maintains timestamps for all actions
- Enables audit trail and replay capabilities

**Statistics:**
```cpp
int msg_count = session_manager_->getMessageCount();          // 23
int tool_count = session_manager_->getToolExecutionCount();    // 8
int file_count = session_manager_->getFileModificationCount(); // 3

auto files = session_manager_->getModifiedFiles();
// ["/path/file1.cpp", "/path/file2.h", "/path/file3.cpp"]

auto tools = session_manager_->getExecutedTools();
// ["Write", "Edit", "Bash", "Glob"]
```

### 5. Session Summaries

**What it does:**
- AI generates concise summary of session accomplishments
- Saved in session metadata
- Shown when resuming session
- Included in exported documentation

**How it works:**
```cpp
// On session exit, prompt AI for summary
std::string prompt = "Based on our conversation, generate a brief "
                     "2-3 sentence summary of what we accomplished.";

std::string summary = client_->generate(model, prompt, temp, max_tokens);

// Save to session
session_manager_->generateSessionSummary(summary);
```

**Example summary:**
```
Implemented comprehensive session management for ollamaCode C++ version,
including session persistence with SQLite, session resume functionality,
auto-documentation generation (TODO.md, DECISIONS.md), state tracking for
all file modifications and tool executions, and AI-generated session summaries.
```

## Session Workflow Example

### New Session

```bash
$ ollamacode

   ____  _ _                       ____          _
  / __ \| | | __ _ _ __ ___   __ _/ ___|___   __| | ___
 | |  | | | |/ _` | '_ ` _ \ / _` | |   / _ \ / _` |/ _ \
 | |__| | | | (_| | | | | | | (_| | |__| (_) | (_| |  __/
  \____/|_|_|\__,_|_| |_| |_|\__,_|\____\___/ \__,_|\___|

✓ Created new session: session_20250117_143000_1234

Current Configuration:
  Model:        llama3
  Host:         http://localhost:11434
  ...

You> Help me implement a new feature

🤔 Thinking...

I'll help you implement that feature. Let me start by understanding the current codebase...

🔧 Executing 2 tool(s)...

[Tool executions happen, conversation continues...]

You> exit

📊 Generating session summary...

✓ Session saved and documented
  - TODO.md generated
  - DECISIONS.md generated

👋 Goodbye!
```

### Resume Session

```bash
$ ollamacode --resume

✓ Resumed session: session_20250117_143000_1234

Session Info:
  Created: 2025-01-17 14:30:00
  Messages: 15
  Tools executed: 6
  Files modified: 2

Summary:
Helped implement a new feature by analyzing the codebase and making necessary modifications...

Recent Conversation:

user: Help me implement a new feature
assistant: I'll help you implement that feature...
...

You> Let's continue - can you now add tests?

[Session continues with full context...]
```

## API Reference

### SessionManager Methods

```cpp
// Lifecycle
std::string createSession(const std::string& model, const std::string& working_dir);
bool loadSession(const std::string& session_id);
bool saveSession();
bool closeSession();

// Message management
void addUserMessage(const std::string& content);
void addAssistantMessage(const std::string& content);
void addToolMessage(const std::string& tool_name, const std::string& content);

// Tool execution tracking
void recordToolExecution(const std::string& tool_name,
                        const json& parameters,
                        const std::string& output,
                        int exit_code);

// File modification tracking
void recordFileModification(const std::string& file_path,
                           const std::string& operation);

// Context retrieval
std::vector<Message> getConversationContext(int max_messages = 10) const;

// Session listing
std::vector<std::string> listSessions() const;
std::vector<std::string> listActiveSessions() const;
std::string getLastActiveSession() const;

// Documentation generation
bool generateTodoMd(const std::string& output_path = "") const;
bool generateDecisionsMd(const std::string& output_path = "") const;
bool generateSessionReport(const std::string& output_path = "") const;

// Export
bool exportSessionToJson(const std::string& file_path) const;
bool exportSessionToMarkdown(const std::string& file_path) const;

// Statistics
int getMessageCount() const;
int getToolExecutionCount() const;
int getFileModificationCount() const;
std::vector<std::string> getModifiedFiles() const;
std::vector<std::string> getExecutedTools() const;
```

## Configuration

### Database Location

- Database: `~/.config/ollamacode/sessions/sessions.db`
- Exports: Current working directory (unless specified)
- Auto-docs: Current working directory

### Default Behavior

- New session created on each start (unless `--resume` used)
- Session auto-saved after each message/tool execution
- Documentation auto-generated on exit
- Sessions marked inactive on exit
- Active sessions can be resumed

## Benefits

### For Users

1. **Never lose work** - All conversations persisted
2. **Seamless continuation** - Resume exactly where you left off
3. **Automatic documentation** - No manual note-taking needed
4. **Full audit trail** - See what was changed and when
5. **Context-aware AI** - AI remembers previous conversation

### For Development

1. **Debugging** - Full history of what happened
2. **Collaboration** - Share session exports with team
3. **Learning** - Review past sessions to improve prompts
4. **Compliance** - Audit trail of all changes
5. **Reproducibility** - Replay sessions to reproduce results

## Advanced Usage

### Exporting Sessions

```bash
# Export to JSON for programmatic processing
ollamacode --export json

# Export to Markdown for documentation
ollamacode --export markdown

# Within interactive mode
You> export json
✓ Session exported to session.json

You> export md
✓ Session exported to session.md
```

### Querying Sessions

```bash
# List all sessions
ollamacode --list-sessions

Available Sessions:

  - session_20250117_143000_1234
  - session_20250117_120000_5678
  - session_20250116_180000_9012

Active Sessions:

  - session_20250117_143000_1234 (active)
```

### Manual Documentation Generation

```bash
# In interactive mode
You> generate docs
✓ Documentation generated:
  - TODO.md
  - DECISIONS.md
  - SESSION_REPORT.md
```

### Session Statistics

```bash
# In interactive mode
You> session info

Session Info:
  ID: session_20250117_143000_1234
  Created: 2025-01-17 14:30:00
  Messages: 23
  Tools executed: 8
  Files modified: 3

  Modified files:
    - /root/ollamaCode/cpp/src/session_manager.cpp
    - /root/ollamaCode/cpp/include/session_manager.h
    - /root/ollamaCode/cpp/src/cli.cpp
```

## Implementation Notes

### Performance

- SQLite database is very fast (< 1ms for typical operations)
- Indices on session_id for quick queries
- Conversation context limited to last N messages to avoid bloat
- Large tool outputs truncated in summaries

### Storage

- Typical session: 10-50 KB
- With large tool outputs: up to 1 MB
- Database automatically vacuumed
- Old sessions can be deleted manually

### Thread Safety

- SessionManager is NOT thread-safe
- Designed for single-threaded CLI use
- If using in multi-threaded context, add mutex protection

## Future Enhancements

Possible future additions:

1. **Session search** - Search across all sessions
2. **Session tagging** - Organize sessions by project/topic
3. **Session diff** - Compare two sessions
4. **Session merge** - Combine multiple sessions
5. **Session replay** - Re-execute tools from session
6. **Web interface** - Browse sessions via web UI
7. **Session sharing** - Export/import session bundles
8. **Cloud sync** - Sync sessions across machines

## Troubleshooting

### Session not resuming

```bash
# Check if session exists
ollamacode --list-sessions

# Try resuming with explicit session ID
ollamacode --resume session_20250117_143000_1234
```

### Database errors

```bash
# Check database location
ls ~/.config/ollamacode/sessions/sessions.db

# Check permissions
chmod 644 ~/.config/ollamacode/sessions/sessions.db

# If corrupted, backup and delete
mv ~/.config/ollamacode/sessions/sessions.db{,.bak}
```

### Export failing

```bash
# Check write permissions in current directory
touch test.txt && rm test.txt

# Try exporting to specific path
ollamacode --export json > /tmp/session.json
```

## Conclusion

Session management in ollamaCode C++ provides enterprise-grade conversation tracking, context preservation, and automatic documentation generation. This enables:

- Long-running projects that span multiple sessions
- Seamless collaboration and knowledge sharing
- Full audit trails for compliance
- AI that remembers and learns from past interactions
- Automatic documentation of all work performed

All while maintaining the simplicity and speed of the command-line interface.
