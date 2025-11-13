Name:           ollamacode
Version:        2.0.0
Release:        1%{?dist}
Summary:        Interactive CLI for Ollama (Claude Code style with Tool Calling)

License:        MIT
URL:            https://github.com/core-at/ollamacode
Source0:        %{name}-%{version}.tar.gz

BuildArch:      noarch
Requires:       bash >= 4.0
Requires:       curl
Requires:       jq
Requires:       ollama

%description
ollamaCode is an interactive command-line interface for Ollama,
inspired by Claude Code. It provides a rich CLI experience for
interacting with local LLM models through Ollama with full tool calling support.

Features:
- Interactive chat mode with AI tool calling
- AI can execute bash commands, read/write/edit files
- Multiple model support
- Safe mode with command allowlisting
- Configuration management
- Session history
- Statistics and metrics
- Cross-platform (Linux, macOS)

%prep
%setup -q

%build
# Nothing to build - pure bash script

%install
rm -rf %{buildroot}

# Create directories
install -d %{buildroot}%{_bindir}
install -d %{buildroot}%{_datadir}/%{name}
install -d %{buildroot}%{_docdir}/%{name}
install -d %{buildroot}%{_mandir}/man1

# Install binary and libraries
install -m 0755 bin/ollamacode-new %{buildroot}%{_bindir}/ollamacode
install -d %{buildroot}/usr/local/lib/ollamacode
install -m 0644 lib/*.sh %{buildroot}/usr/local/lib/ollamacode/

# Install documentation
install -m 0644 docs/README.md %{buildroot}%{_docdir}/%{name}/
install -m 0644 docs/LICENSE %{buildroot}%{_docdir}/%{name}/

# Install man page
install -m 0644 docs/ollamacode.1 %{buildroot}%{_mandir}/man1/

%files
%{_bindir}/ollamacode
/usr/local/lib/ollamacode/*.sh
%{_datadir}/%{name}
%doc %{_docdir}/%{name}/README.md
%license %{_docdir}/%{name}/LICENSE
%{_mandir}/man1/ollamacode.1*

%changelog
* Thu Oct 16 2025 Core.at <support@core.at> - 2.0.0-1
- Major release: Added tool calling support
- AI can now execute bash commands
- AI can read, write, and edit files
- Added Glob and Grep tools for file operations
- Safe mode with command allowlisting
- Auto-approve mode for automation
- Improved conversation loop with iterative tool calling
- Enhanced system prompts for better AI tool usage

* Thu Oct 16 2025 Core.at <support@core.at> - 1.0.0-1
- Initial release
- Interactive CLI mode
- Model switching
- Configuration management
- Session history
- Statistics display
