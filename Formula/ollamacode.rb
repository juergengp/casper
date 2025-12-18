class Ollamacode < Formula
  desc "Local AI coding assistant powered by Ollama - Claude Code experience, zero cloud"
  homepage "https://github.com/juergengp/ollamaCode"
  version "2.0.3"
  license "MIT"

  on_macos do
    if Hardware::CPU.arm?
      url "https://github.com/juergengp/ollamaCode/releases/download/v2.0.3/ollamacode-macos-arm64"
      sha256 "b7cdf80ff105056b323e3259a32ebb0b7a86fadf4ed83d06013e4d000a057677"
    else
      url "https://github.com/juergengp/ollamaCode/releases/download/v2.0.3/ollamacode-macos-x86_64"
      sha256 "6531cc1df42b66eb9714d170981c87ee255f9a1a5e9b6bac36ca9b73b3afa5e4"
    end
  end

  depends_on :macos

  def install
    binary_name = Hardware::CPU.arm? ? "ollamacode-macos-arm64" : "ollamacode-macos-x86_64"
    bin.install Dir["*"].first => "ollamacode"
  end

  test do
    assert_match "ollamaCode version", shell_output("#{bin}/ollamacode --version")
  end
end
