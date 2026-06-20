# AI Assistant Plugin for Notepad++

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](https://notepad-plus-plus.org/)
[![Notepad++](https://img.shields.io/badge/Notepad++-Plugin-green.svg)](https://notepad-plus-plus.org/)

A Notepad++ plugin that integrates AI capabilities (ChatGPT, Claude, Gemini, Ollama) directly into the editor.


## Features

- **Dockable AI Chat Panel** - Opens on the right side of Notepad++
- **Multiple AI Providers** - Support for OpenAI (ChatGPT), Anthropic (Claude), Ollama and Google (Gemini)
- **Custom Endpoints** - Use OpenRouter, local LLMs, or any OpenAI-compatible API
- **Context-Aware** - Automatically includes file content and selected text in prompts
- **Apply AI Suggestions** - Apply AI-generated code changes directly to your document
- **Keyboard Shortcut** - `Ctrl+Shift+A` to toggle the AI panel

## Table of Contents

- [Installation](#installation)
- [Quick Start](#quick-start)
- [Usage](#usage)
- [Configuration](#configuration)
- [Building from Source](#building-from-source)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## Installation

### From Release

1. Download the latest release from [Releases](../../releases)
2. Extract the appropriate DLL for your Notepad++ version:
   - **64-bit Notepad++**: Use `NPPAIPlugin-Ollama.dll`
3. Copy the DLL to your Notepad++ plugins folder:
   ```
   %ProgramFiles%\Notepad++\plugins\NPPAIPlugin-Ollama\NPPAIPlugin-Ollama.dll
   ```
4. Restart Notepad++
5. Configure your API keys in **Plugins > AI Assistant > Settings**
6. Configure AI models on ```%App-Data%\Notepad++\plugins\config\notepad_aiplug.ini```

### From Source

See [Building from Source](#building-from-source) below.

## Quick Start

1. Open the AI Panel: Press `Ctrl+Shift+A` or go to **Plugins > AI Assistant > Show AI Panel**
2. Go to **Plugins > AI Assistant > Settings** and enter your API key
3. Select a provider from the dropdown
4. Type your question and press **Send**

## Usage

### Opening the AI Panel

- Press `Ctrl+Shift+A`, or
- Go to **Plugins > AI Assistant > Show AI Panel**

The panel will dock on the right side of Notepad++.

### Basic Workflow

1. **Select a provider** from the dropdown (ChatGPT, Claude, Gemini, Ollama or custom)
2. **Type your question** in the prompt box at the bottom
3. Click **Send** or press `Enter`
4. View the AI response in the chat history

### Using File Context

Check **"Include file context"** to send your current file to the AI:

| Selection State | What Gets Sent |
|----------------|----------------|
| Text selected | Only the selected text |
| No selection | Entire file content with file path and language |

This allows the AI to understand your code and provide relevant suggestions.

### Applying AI Suggestions

When the AI provides code:

1. Click **Apply** to insert/replace code in your document
2. If text was selected, it replaces the selection
3. If no selection, it replaces the entire document

### Custom Endpoints

You can use any OpenAI-compatible API by setting a custom URL:

<details>
<summary><b>OpenRouter Example</b></summary>

- **Provider**: ChatGPT (OpenAI) - uses OpenAI API format
- **API Key**: Your OpenRouter API key (`sk-or-v1-...`)
- **Custom URL**: `https://openrouter.ai/api/v1/chat/completions`

</details>

<details>
<summary><b>Local LLM Example (LM Studio)</b></summary>

- **Provider**: ChatGPT (OpenAI)
- **API Key**: `not-needed` (or leave empty if your server doesn't require it) #Not needed for Ollama
- **Custom URL**: `http://localhost:11434/v1/chat/completions`

</details>

### Adding Custom Providers

1. Open **Settings**
2. Enter a **new name** in the Provider Name field
3. Enter the **API Key** and **Custom URL**
4. Click **Add Provider**

Custom providers appear in the dropdown alongside built-in providers.

## Configuration

Settings are stored in:
```
%APPDATA%\Notepad++\plugins\config\notepad_aiplug.ini
```

### Getting API Keys

| Provider | Link |
|----------|------|
| OpenAI (ChatGPT) | [platform.openai.com/api-keys](https://platform.openai.com/api-keys) |
| Anthropic (Claude) | [console.anthropic.com](https://console.anthropic.com/) |
| Google (Gemini) | [makersuite.google.com/app/apikey](https://makersuite.google.com/app/apikey) |
| OpenRouter | [openrouter.ai/keys](https://openrouter.ai/keys) |

### Default API Endpoints

| Provider | Default URL |
|----------|-------------|
| OpenAI | `https://api.openai.com/v1/chat/completions` |
| Claude | `https://api.anthropic.com/v1/messages` |
| Gemini | `https://generativelanguage.googleapis.com/v1/models/{model}:generateContent` |

## Building from Source

### Prerequisites

- Visual Studio 2019 or 2022 with **C++ Desktop Development** workload
- Windows 10 SDK

### Build Steps

```bash
# Clone the repository
git clone https://github.com/user/notepad_aiplug.git
cd notepad_aiplug

# Open solution in Visual Studio
start notepad_aiplug.sln
```

1. Select configuration:
   - **Release|x64** for 64-bit Notepad++
2. Build the solution (`Ctrl+Shift+B`)
3. Copy the output DLL:
   - **64-bit**: `bin64\notepad_aiplug.dll`

## Project Structure

```
notepad_aiplug/
├── src/
│   ├── PluginDefinition.cpp    # Main plugin logic
│   ├── NppPluginDemo.cpp       # DLL entry point
│   ├── DockingFeature/         # UI dialogs
│   │   ├── AIPanelDlg.*        # AI chat panel
│   │   ├── SettingsDlg.*       # Settings dialog
│   │   └── DiffViewDlg.*       # Diff viewer
│   ├── AIProviders/            # AI provider implementations
│   │   ├── OpenAIProvider.*    # ChatGPT / OpenAI-compatible
│   │   ├── ClaudeProvider.*    # Claude
│   │   └── GeminiProvider.*    # Gemini
|   |   └── OllamaProvider.*    # Ollama 
│   ├── Http/
│   │   └── HttpClient.*        # WinHTTP wrapper
│   └── Utils/
│       └── Config.*            # Settings management
├── include/                    # Notepad++/Scintilla headers
├── res/                        # Dialog resources
├── lib/nlohmann/json.hpp       # JSON library
└── NPPAIPlugin-Ollama.slnx     # Solution file
```

## Troubleshooting

<details>
<summary><b>"API key not set" error</b></summary>

Go to **Settings** and enter your API key for the selected provider.

</details>

<details>
<summary><b>"HTTP error 401" (Unauthorized)</b></summary>

- Check that your API key is correct
- For OpenRouter: Make sure you're using the OpenRouter key, not an OpenAI key

</details>

<details>
<summary><b>"HTTP error 402" (Payment Required)</b></summary>

- Your API account needs credits
- For OpenRouter: Add credits at [openrouter.ai/settings/credits](https://openrouter.ai/settings/credits)

</details>

<details>
<summary><b>Custom endpoint not working</b></summary>

- Ensure the URL is correct and includes the full path (e.g., `/v1/chat/completions`)
- Check that the endpoint uses OpenAI-compatible API format

</details>

<details>
<summary><b>Plugin not appearing in Notepad++</b></summary>

- Ensure the DLL is in the correct folder: `plugins\notepad_aiplug\notepad_aiplug.dll`
- Check that you're using the correct version (32-bit vs 64-bit)
- Restart Notepad++

</details>

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the GPL v2 License - see the [LICENSE](LICENSE) file for details.

## Author

**Jiawei Qi** - 2026
**Mao Urban** - 2026

## Acknowledgments

- [Notepad++ Plugin Template](https://github.com/npp-plugins/plugintemplate)
- [nlohmann/json](https://github.com/nlohmann/json) - JSON for Modern C++
