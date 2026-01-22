//this file is part of notepad_aiplug
//Copyright (C)2025

#include "OpenAIProvider.h"
#include "Http/HttpClient.h"
#include "nlohmann/json.hpp"
#include <stdexcept>

using json = nlohmann::json;

const char* OpenAIProvider::API_ENDPOINT = "https://api.openai.com/v1/chat/completions";

OpenAIProvider::OpenAIProvider()
    : _model("gpt-4o")
{
}

void OpenAIProvider::setApiKey(const std::string& key)
{
    _apiKey = key;
}

std::string OpenAIProvider::getApiKeyMasked() const
{
    if (_apiKey.empty())
        return "";
    if (_apiKey.length() <= 8)
        return std::string(_apiKey.length(), '*');
    return _apiKey.substr(0, 4) + std::string(_apiKey.length() - 8, '*') + _apiKey.substr(_apiKey.length() - 4);
}

bool OpenAIProvider::hasApiKey() const
{
    return !_apiKey.empty();
}

void OpenAIProvider::setModel(const std::string& model)
{
    _model = model;
}

std::string OpenAIProvider::getModel() const
{
    return _model;
}

std::vector<std::string> OpenAIProvider::getAvailableModels() const
{
    return {
        "gpt-4o",
        "gpt-4o-mini",
        "gpt-4-turbo",
        "gpt-4",
        "gpt-3.5-turbo"
    };
}

std::string OpenAIProvider::sendPrompt(const std::string& prompt, const std::string& context)
{
    if (_apiKey.empty())
    {
        throw std::runtime_error("OpenAI API key not set. Please configure it in Settings.");
    }

    // Build the full prompt with context
    std::string fullPrompt = prompt;
    if (!context.empty())
    {
        fullPrompt = context + "\n\nUser request: " + prompt;
    }

    // Build request JSON
    json requestBody;
    requestBody["model"] = _model;
    requestBody["messages"] = json::array();
    requestBody["messages"].push_back({
        {"role", "system"},
        {"content", "You are a helpful coding assistant integrated into Notepad++. "
                    "Help the user with their code-related questions and tasks. "
                    "When providing code, be concise and provide only the relevant code. "
                    "If the user asks you to modify code, provide the complete modified code."}
    });
    requestBody["messages"].push_back({
        {"role", "user"},
        {"content", fullPrompt}
    });
    requestBody["max_tokens"] = 2048;

    // Send request
    HttpClient http;
    http.setHeader("Content-Type", "application/json");
    http.setHeader("Authorization", "Bearer " + _apiKey);

    // Use custom endpoint if set, otherwise use default
    std::string endpoint = _customEndpoint.empty() ? API_ENDPOINT : _customEndpoint;

    try {
        std::string response = http.post(endpoint, requestBody.dump());

        json responseJson = json::parse(response);

        if (responseJson.contains("error"))
        {
            std::string errorMsg = responseJson["error"]["message"].get<std::string>();
            throw std::runtime_error("API error: " + errorMsg + " [endpoint: " + endpoint + "]");
        }

        if (responseJson.contains("choices") && responseJson["choices"].size() > 0)
        {
            return responseJson["choices"][0]["message"]["content"].get<std::string>();
        }

        throw std::runtime_error("Unexpected response format [endpoint: " + endpoint + "]");
    }
    catch (const std::exception& e)
    {
        // Re-throw with endpoint info if not already included
        std::string what = e.what();
        if (what.find("endpoint:") == std::string::npos)
        {
            throw std::runtime_error(std::string(e.what()) + " [endpoint: " + endpoint + "]");
        }
        throw;
    }
}
