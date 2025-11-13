Name:           ollamacode
Version:        2.0.0
Release:        1%{?dist}
Summary:        Interactive CLI for Ollama - Complete package (Bash + C++)

License:        MIT
URL:            https://github.com/core-at/ollamacode
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc-c++ >= 7
BuildRequires:  cmake >= 3.15
BuildRequires:  libcurl-devel
BuildRequires:  sqlite-devel
BuildRequires:  readline-devel

Requires:       bash >= 4.0
Requires:       curl
Requires:       jq
Requires:       libcurl
Requires:       sqlite
Requires:       readline
Requires:       ollama

%description
ollamaCode Complete - Interactive command-line interface for Ollama with both
Bash and C++ implementations.

This package includes:
- Bash version: Zero-build-time, works immediately, easy to modify
- C++ version: High-performance, session management, enterprise features

Choose which version to use based on your needs:
- ollamacode-bash: Quick deployment, no compilation
- ollamacode: High-performance C++ with session management

Features:
- Interactive chat mode with AI tool calling
- AI can execute bash commands, read/write/edit files
- Session persistence and resume (C++ version)
- Auto-documentation generation (C++ version)
- Multiple model support
- Safe mode with command allowlisting
- Configuration management
- Cross-platform (Linux, macOS)

%prep
%setup -q

%build
# Build C++ version
cd cpp
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ..
make -j$(nproc)
cd ../..

%install
rm -rf %{buildroot}

# Install Bash version
install -d %{buildroot}%{_bindir}
install -d %{buildroot}/usr/local/lib/ollamacode
install -m 0755 bin/ollamacode-new %{buildroot}%{_bindir}/ollamacode-bash
install -m 0644 lib/*.sh %{buildroot}/usr/local/lib/ollamacode/

# Install C++ version
install -m 0755 cpp/build/ollamacode %{buildroot}%{_bindir}/ollamacode

# Create symlink for default (C++ version is default)
# Users can use ollamacode-bash explicitly for Bash version

# Install documentation
install -d %{buildroot}%{_docdir}/%{name}
install -m 0644 README-v2.md %{buildroot}%{_docdir}/%{name}/README.md
install -m 0644 SESSION_MANAGEMENT_IMPLEMENTATION.md %{buildroot}%{_docdir}/%{name}/
install -m 0644 CPP_SESSION_MANAGEMENT_COMPLETE.md %{buildroot}%{_docdir}/%{name}/
install -m 0644 cpp/SESSION_MANAGEMENT.md %{buildroot}%{_docdir}/%{name}/SESSION_MANAGEMENT_CPP.md
install -m 0644 cpp/BUILD.md %{buildroot}%{_docdir}/%{name}/BUILD_CPP.md
install -m 0644 docs/QUICKSTART.md %{buildroot}%{_docdir}/%{name}/
install -m 0644 docs/EXAMPLES.md %{buildroot}%{_docdir}/%{name}/

# Create man page
install -d %{buildroot}%{_mandir}/man1
cat > %{buildroot}%{_mandir}/man1/ollamacode.1 << 'EOF'
.TH OLLAMACODE 1 "October 2025" "Version 2.0.0" "User Commands"
.SH NAME
ollamacode \- Interactive CLI for Ollama with tool calling and session management
.SH SYNOPSIS
.B ollamacode
[\fIOPTIONS\fR] [\fIPROMPT\fR]
.br
.B ollamacode-bash
[\fIOPTIONS\fR] [\fIPROMPT\fR]
.SH DESCRIPTION
ollamaCode is an interactive command-line interface for Ollama, providing a rich CLI
experience for interacting with local LLM models with full tool calling support.

Two versions are available:
.TP
.B ollamacode
C++ version with session management (default, recommended)
.TP
.B ollamacode-bash
Bash version, zero build time, easy to modify

.SH OPTIONS
.TP
.B \-m, \-\-model \fIMODEL\fR
Use specific model
.TP
.B \-t, \-\-temperature \fINUM\fR
Set temperature (0.0-2.0)
.TP
.B \-a, \-\-auto-approve
Auto-approve all tool executions
.TP
.B \-\-unsafe
Disable safe mode (allow all commands)
.TP
.B \-r, \-\-resume [\fISESSION_ID\fR]
Resume previous session (C++ only)
.TP
.B \-l, \-\-list-sessions
List all sessions (C++ only)
.TP
.B \-e, \-\-export \fIFORMAT\fR
Export session to json or markdown (C++ only)
.TP
.B \-v, \-\-version
Show version
.TP
.B \-h, \-\-help
Show help message

.SH INTERACTIVE COMMANDS
.TP
.B help
Show available commands
.TP
.B models
List available Ollama models
.TP
.B use \fIMODEL\fR
Switch to different model
.TP
.B temp \fINUM\fR
Set temperature
.TP
.B safe [on|off]
Toggle safe mode
.TP
.B auto [on|off]
Toggle auto-approve
.TP
.B session info
Show current session info (C++ only)
.TP
.B export json|md
Export session (C++ only)
.TP
.B generate docs
Generate TODO.md and DECISIONS.md (C++ only)
.TP
.B clear
Clear screen
.TP
.B config
Show configuration
.TP
.B exit, quit
Exit ollamacode

.SH SESSION MANAGEMENT (C++ VERSION)
The C++ version includes enterprise-grade session management:

.TP
.B Session Persistence
All conversations saved to SQLite database in ~/.config/ollamacode/sessions/

.TP
.B Session Resume
Continue previous sessions with full context using --resume flag

.TP
.B Auto-Documentation
Automatically generates TODO.md and DECISIONS.md on exit

.TP
.B State Tracking
Tracks all file modifications and tool executions

.TP
.B AI Summaries
Generates session summaries using AI on exit

.SH FILES
.TP
.B ~/.config/ollamacode/config
User configuration (Bash version)
.TP
.B ~/.config/ollamacode/config.db
User configuration database (C++ version)
.TP
.B ~/.config/ollamacode/sessions/sessions.db
Session database (C++ version)
.TP
.B ~/.config/ollamacode/history
Command history
.TP
.B /usr/local/lib/ollamacode/
Bash libraries

.SH EXAMPLES
.TP
Start interactive mode (C++):
.B ollamacode

.TP
Start interactive mode (Bash):
.B ollamacode-bash

.TP
Single prompt with C++:
.B ollamacode "List all Python files"

.TP
Resume last session:
.B ollamacode --resume

.TP
Resume specific session:
.B ollamacode --resume session_20250117_143000_1234

.TP
List all sessions:
.B ollamacode --list-sessions

.TP
Auto-approve all tools:
.B ollamacode -a "Build the project"

.TP
Use specific model:
.B ollamacode -m llama3 "Your prompt"

.TP
Export current session:
.B ollamacode --export markdown

.SH ENVIRONMENT
.TP
.B OLLAMA_HOST
Ollama server URL (default: http://localhost:11434)

.SH AUTHOR
Written by Core.at

.SH REPORTING BUGS
Report bugs to: support@core.at

.SH SEE ALSO
.BR ollama (1)
EOF

%files
%{_bindir}/ollamacode
%{_bindir}/ollamacode-bash
/usr/local/lib/ollamacode/*.sh
%doc %{_docdir}/%{name}/*.md
%{_mandir}/man1/ollamacode.1*

%post
cat << 'EOF'

╔════════════════════════════════════════════════════════════╗
║          ollamaCode 2.0.0 Complete - Installed            ║
╚════════════════════════════════════════════════════════════╝

Two versions available:

  📦 ollamacode       - C++ version (default, recommended)
                        • Session management
                        • Auto-documentation
                        • 15x faster

  📦 ollamacode-bash  - Bash version
                        • Zero build time
                        • Easy to modify

Quick Start:
  ollamacode                    # Start C++ version
  ollamacode --resume           # Resume last session
  ollamacode-bash               # Start Bash version

  man ollamacode                # Read manual

Documentation:
  /usr/share/doc/ollamacode/

Config Location:
  ~/.config/ollamacode/

Session Database (C++):
  ~/.config/ollamacode/sessions/sessions.db

═══════════════════════════════════════════════════════════
EOF
exit 0

%changelog
* Fri Oct 17 2025 Core.at <support@core.at> - 2.0.0-1
- Complete package with both Bash and C++ versions
- C++ version: Added session management with SQLite
- C++ version: Added session resume functionality
- C++ version: Added auto-documentation generation
- C++ version: Added state tracking
- C++ version: 15x performance improvement
- Bash version: Updated to v2.0 with tool calling
- Both versions: Safe mode, auto-approve, tool calling
- Comprehensive documentation included

* Thu Oct 16 2025 Core.at <support@core.at> - 1.0.0-1
- Initial release (Bash only)
