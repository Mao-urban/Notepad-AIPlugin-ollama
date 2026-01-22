//this file is part of notepad_aiplug
//Copyright (C)2025

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <windows.h>

// API type enumeration
enum class APIType
{
    OpenAI = 0,
    Claude = 1,
    Gemini = 2
};

// Edit mode enumeration
enum class EditMode
{
    DiffFirst = 0,
    DirectEdit = 1
};

// AI Provider configuration
struct AIProviderConfig
{
    std::string name;           // Display name (e.g., "ChatGPT (OpenAI)")
    std::string apiKey;         // API key (can be empty)
    APIType apiType;            // API type (OpenAI, Claude, Gemini)
    std::string customEndpoint; // Optional custom API endpoint URL
    bool isBuiltIn;             // true for default 3 providers (cannot delete)

    AIProviderConfig()
        : apiType(APIType::OpenAI), isBuiltIn(false) {}

    AIProviderConfig(const std::string& n, APIType t, bool builtIn = false, const std::string& endpoint = "")
        : name(n), apiType(t), customEndpoint(endpoint), isBuiltIn(builtIn) {}

    bool hasApiKey() const { return !apiKey.empty(); }
};

class Config
{
public:
    // Singleton access
    static Config& getInstance();

    // Initialize with config file path
    void init(const std::wstring& configDir);

    // Load configuration from file
    void load();

    // Save configuration to file
    void save();

    // Get config file path (for debugging)
    std::wstring getConfigPath() const { return _configPath; }

    // Provider management
    std::vector<AIProviderConfig>& getProviders() { return _providers; }
    const std::vector<AIProviderConfig>& getProviders() const { return _providers; }
    void addProvider(const AIProviderConfig& provider);
    void removeProvider(size_t index);
    void updateProvider(size_t index, const AIProviderConfig& provider);
    AIProviderConfig* getProvider(size_t index);

    // Get provider by index (for backward compatibility)
    std::string getApiKey(size_t index) const;
    void setApiKey(size_t index, const std::string& key);

    // Default provider index
    int getDefaultProvider() const { return _defaultProvider; }
    void setDefaultProvider(int provider) { _defaultProvider = provider; }

    // Edit mode
    EditMode getEditMode() const { return _editMode; }
    void setEditMode(EditMode mode) { _editMode = mode; }

    // Legacy getters for backward compatibility
    std::string getOpenAIKey() const;
    std::string getClaudeKey() const;
    std::string getGeminiKey() const;
    std::string getOpenAIModel() const { return "gpt-4o"; }
    std::string getClaudeModel() const { return "claude-sonnet-4-20250514"; }
    std::string getGeminiModel() const { return "gemini-1.5-pro"; }

private:
    Config();
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    void initDefaultProviders();

    std::wstring _configPath;

    // Providers list
    std::vector<AIProviderConfig> _providers;

    // Preferences
    int _defaultProvider = 0;
    EditMode _editMode = EditMode::DiffFirst;

    // Helper functions
    std::string readIniString(const std::wstring& section, const std::wstring& key, const std::string& defaultValue);
    int readIniInt(const std::wstring& section, const std::wstring& key, int defaultValue);
    void writeIniString(const std::wstring& section, const std::wstring& key, const std::string& value);
    void writeIniInt(const std::wstring& section, const std::wstring& key, int value);

    // Simple XOR obfuscation for API keys
    std::string obfuscate(const std::string& str);
    std::string deobfuscate(const std::string& str);
};

#endif // CONFIG_H
