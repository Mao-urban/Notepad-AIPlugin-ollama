//this file is part of notepad_aiplug
//Copyright (C)2025

#include "Config.h"
#include <shlwapi.h>
#include <shlobj.h>
#include <vector>
#include <stdexcept>

#pragma comment(lib, "shlwapi.lib")

Config::Config()
{
    initDefaultProviders();
}

Config& Config::getInstance()
{
    static Config instance;
    return instance;
}

void Config::initDefaultProviders()
{
    _providers.clear();

    // Add 4 built-in providers
    _providers.push_back(AIProviderConfig("ChatGPT (OpenAI)", APIType::OpenAI, true));
    _providers.push_back(AIProviderConfig("Claude (Anthropic)", APIType::Claude, true, "https://api.anthropic.com/v1/messages"));
    _providers.push_back(AIProviderConfig("Gemini (Google)", APIType::Gemini, true));
	_providers.push_back(AIProviderConfig("Ollama",APIType::Ollama, true, "http://127.0.0.1:11434/v1/chat/completions"));
}

void Config::init(const std::wstring& configDir)
{
    // Create config directory if it doesn't exist
    if (!PathFileExistsW(configDir.c_str()))
    {
        SHCreateDirectoryExW(NULL, configDir.c_str(), NULL);
    }

    _configPath = configDir + L"\\notepad_aiplug.ini";
    load();
}

void Config::load()
{
    if (_configPath.empty())
        return;

    // Re-initialize default providers
    initDefaultProviders();

    // Load API keys and custom endpoints for built-in providers
    if (_providers.size() >= 4)
    {
        _providers[0].apiKey = deobfuscate(readIniString(L"APIKeys", L"OpenAI", ""));
        _providers[1].apiKey = deobfuscate(readIniString(L"APIKeys", L"Claude", ""));
        _providers[2].apiKey = deobfuscate(readIniString(L"APIKeys", L"Gemini", ""));

        // Load custom endpoints for built-in providers (empty string means use default)
        std::string openaiEndpoint = readIniString(L"Endpoints", L"OpenAI", "");
        std::string claudeEndpoint = readIniString(L"Endpoints", L"Claude", "");
        std::string geminiEndpoint = readIniString(L"Endpoints", L"Gemini", "");
		std::string ollamaEndpoint = readIniString(L"Endpoints", L"Ollama", "");

        if (!openaiEndpoint.empty())
            _providers[0].customEndpoint = openaiEndpoint;
        if (!claudeEndpoint.empty())
            _providers[1].customEndpoint = claudeEndpoint;
        if (!geminiEndpoint.empty())
            _providers[2].customEndpoint = geminiEndpoint;
		if (!ollamaEndpoint.empty())
			_providers[3].customEndpoint = ollamaEndpoint;
    }

    // Load custom providers count
    int customCount = readIniInt(L"CustomProviders", L"Count", 0);

    // Load custom providers
    for (int i = 0; i < customCount; i++)
    {
        std::wstring section = L"CustomProvider" + std::to_wstring(i);

        AIProviderConfig provider;
        provider.name = readIniString(section, L"Name", "");
        provider.apiKey = deobfuscate(readIniString(section, L"APIKey", ""));
        provider.apiType = static_cast<APIType>(readIniInt(section, L"APIType", 0));
        provider.customEndpoint = readIniString(section, L"CustomEndpoint", "");
        provider.isBuiltIn = false;

        if (!provider.name.empty())
        {
            _providers.push_back(provider);
        }
    }

    // Load preferences
    _defaultProvider = readIniInt(L"Preferences", L"DefaultProvider", 0);
    _editMode = static_cast<EditMode>(readIniInt(L"Preferences", L"EditMode", 0));
	//
	_openAIModel = readIniString( L"Models", L"OpenAI", "gpt-4o");
	_claudeModel = readIniString( L"Models", L"Claude", "claude-sonnet-4-20250514");
	_geminiModel = readIniString( L"Models", L"Gemini", "gemini-1.5-pro");
	_ollamaModel = readIniString( L"Models", L"Ollama", "starcoder2:3b");

    // Validate default provider index
    if (_defaultProvider < 0 || _defaultProvider >= static_cast<int>(_providers.size()))
    {
        _defaultProvider = 0;
    }
}

void Config::save()
{
    if (_configPath.empty())
        return;

    // Save API keys and custom endpoints for built-in providers
    if (_providers.size() >= 4)
    {
        writeIniString(L"APIKeys", L"OpenAI", obfuscate(_providers[0].apiKey));
        writeIniString(L"APIKeys", L"Claude", obfuscate(_providers[1].apiKey));
        writeIniString(L"APIKeys", L"Gemini", obfuscate(_providers[2].apiKey));

        // Save custom endpoints for built-in providers
        writeIniString(L"Endpoints", L"OpenAI", _providers[0].customEndpoint);
        writeIniString(L"Endpoints", L"Claude", _providers[1].customEndpoint);
        writeIniString(L"Endpoints", L"Gemini", _providers[2].customEndpoint);
		writeIniString(L"Endpoints", L"Ollama", _providers[3].customEndpoint);
    }

    // Count custom providers (non-built-in)
    int customCount = 0;
    for (size_t i = 4; i < _providers.size(); i++)
    {
        if (!_providers[i].isBuiltIn)
        {
            customCount++;
        }
    }

    // Save custom provider count
    writeIniInt(L"CustomProviders", L"Count", customCount);
	//
	writeIniString( L"Models", L"OpenAI", _openAIModel);
	writeIniString( L"Models", L"Claude", _claudeModel);
	writeIniString( L"Models", L"Gemini", _geminiModel);
	writeIniString( L"Models", L"Ollama", _ollamaModel);

    // Save custom providers
    int customIndex = 0;
    for (size_t i = 4; i < _providers.size(); i++)
    {
        if (!_providers[i].isBuiltIn)
        {
            std::wstring section = L"CustomProvider" + std::to_wstring(customIndex);

            writeIniString(section, L"Name", _providers[i].name);
            writeIniString(section, L"APIKey", obfuscate(_providers[i].apiKey));
            writeIniInt(section, L"APIType", static_cast<int>(_providers[i].apiType));
            writeIniString(section, L"CustomEndpoint", _providers[i].customEndpoint);

            customIndex++;
        }
    }

    // Save preferences
    writeIniInt(L"Preferences", L"DefaultProvider", _defaultProvider);
    writeIniInt(L"Preferences", L"EditMode", static_cast<int>(_editMode));
}

void Config::addProvider(const AIProviderConfig& provider)
{
    _providers.push_back(provider);
}

void Config::removeProvider(size_t index)
{
    // Cannot remove built-in providers (first 4)
    if (index < 4 || index >= _providers.size())
        return;

    if (_providers[index].isBuiltIn)
        return;

    _providers.erase(_providers.begin() + index);

    // Adjust default provider if needed
    if (_defaultProvider >= static_cast<int>(_providers.size()))
    {
        _defaultProvider = 0;
    }
}

void Config::updateProvider(size_t index, const AIProviderConfig& provider)
{
    if (index >= _providers.size())
        return;

    // Preserve isBuiltIn flag for built-in providers
    bool wasBuiltIn = _providers[index].isBuiltIn;
    _providers[index] = provider;
    if (wasBuiltIn)
    {
        _providers[index].isBuiltIn = true;
    }
}

AIProviderConfig* Config::getProvider(size_t index)
{
    if (index >= _providers.size())
        return nullptr;

    return &_providers[index];
}

std::string Config::getApiKey(size_t index) const
{
    if (index >= _providers.size())
        return "";

    return _providers[index].apiKey;
}

void Config::setApiKey(size_t index, const std::string& key)
{
    if (index >= _providers.size())
        return;

    _providers[index].apiKey = key;
}

// Legacy getters for backward compatibility
std::string Config::getOpenAIKey() const
{
    if (_providers.size() > 0)
        return _providers[0].apiKey;
    return "";
}

std::string Config::getClaudeKey() const
{
    if (_providers.size() > 1)
        return _providers[1].apiKey;
    return "";
}

std::string Config::getGeminiKey() const
{
    if (_providers.size() > 2)
        return _providers[2].apiKey;
    return "";
}

std::string Config::readIniString(const std::wstring& section, const std::wstring& key, const std::string& defaultValue)
{
    wchar_t buffer[1024] = {0};
    GetPrivateProfileStringW(
        section.c_str(),
        key.c_str(),
        L"",
        buffer,
        1024,
        _configPath.c_str());

    // Convert wide string to narrow
    int len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 1)
        return defaultValue;

    std::string result(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &result[0], len, nullptr, nullptr);

    return result.empty() ? defaultValue : result;
}

int Config::readIniInt(const std::wstring& section, const std::wstring& key, int defaultValue)
{
    return GetPrivateProfileIntW(
        section.c_str(),
        key.c_str(),
        defaultValue,
        _configPath.c_str());
}

void Config::writeIniString(const std::wstring& section, const std::wstring& key, const std::string& value)
{
    // Convert narrow string to wide
    int len = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
    std::wstring wvalue(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, &wvalue[0], len);

    WritePrivateProfileStringW(
        section.c_str(),
        key.c_str(),
        wvalue.c_str(),
        _configPath.c_str());
}

void Config::writeIniInt(const std::wstring& section, const std::wstring& key, int value)
{
    writeIniString(section, key, std::to_string(value));
}

std::string Config::obfuscate(const std::string& str)
{
    if (str.empty())
        return "";

    // Simple XOR obfuscation (not secure, just basic)
    const unsigned char key = 0x5A;
    std::string result;
    result.reserve(str.length() * 2);

    for (char c : str)
    {
        unsigned char obfuscated = static_cast<unsigned char>(c) ^ key;
        char hex[3];
        sprintf_s(hex, "%02X", obfuscated);
        result += hex;
    }

    return result;
}

std::string Config::deobfuscate(const std::string& str)
{
    if (str.empty() || str.length() % 2 != 0)
        return "";

    const unsigned char key = 0x5A;
    std::string result;
    result.reserve(str.length() / 2);

    for (size_t i = 0; i < str.length(); i += 2)
    {
        unsigned char obfuscated = 0;
        sscanf_s(str.c_str() + i, "%02hhX", &obfuscated);
        result += static_cast<char>(obfuscated ^ key);
    }

    return result;
}
//
std::string Config::getOpenAIModel() const
{
    return _openAIModel;
}

std::string Config::getClaudeModel() const
{
    return _claudeModel;
}

std::string Config::getGeminiModel() const
{
    return _geminiModel;
}

std::string Config::getOllamaModel() const
{
    return _ollamaModel;
}

void Config::setOpenAIModel(const std::string& model)
{
    _openAIModel = model;
}

void Config::setClaudeModel(const std::string& model)
{
    _claudeModel = model;
}

void Config::setGeminiModel(const std::string& model)
{
    _geminiModel = model;
}

void Config::setOllamaModel(const std::string& model)
{
    _ollamaModel = model;
}