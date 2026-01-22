//this file is part of notepad_aiplug
//Copyright (C)2025

#include "ClaudeProvider.h"
#include "Http/HttpClient.h"
#include "nlohmann/json.hpp"
#include <stdexcept>

using json = nlohmann::json;

const char* ClaudeProvider::API_ENDPOINT = "https://api.anthropic.com/v1/messages";

ClaudeProvider::ClaudeProvider()
    : _model("claude-sonnet-4-20250514")
{
}

void ClaudeProvider::setApiKey(const std::string& key)
{
    _apiKey = key;
}

std::string ClaudeProvider::getApiKeyMasked() const
{
    if (_apiKey.empty())
        return "";
    if (_apiKey.length() <= 8)
        return std::string(_apiKey.length(), '*');
    return _apiKey.substr(0, 4) + std::string(_apiKey.length() - 8, '*') + _apiKey.substr(_apiKey.length() - 4);
}

bool ClaudeProvider::hasApiKey() const
{
    return !_apiKey.empty();
}

void ClaudeProvider::setModel(const std::string& model)
{
    _model = model;
}

std::string ClaudeProvider::getModel() const
{
    return _model;
}

std::vector<std::string> ClaudeProvider::getAvailableModels() const
{
    return {
        "claude-sonnet-4-20250514",
        "claude-3-5-sonnet-20241022",
        "claude-3-5-haiku-20241022",
        "claude-3-opus-20240229",
        "claude-3-haiku-20240307"
    };
}

std::string ClaudeProvider::sendPrompt(const std::string& prompt, const std::string& context)
{
    if (_apiKey.empty())
    {
        throw std::runtime_error("Claude API key not set. Please configure it in Settings.");
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
    requestBody["max_tokens"] = 4096;
    requestBody["system"] = "You are a helpful coding assistant integrated into Notepad++. "
                            "Help the user with their code-related questions and tasks. "
                            "When providing code, be concise and provide only the relevant code. "
                            "If the user asks you to modify code, provide the complete modified code.";
    requestBody["messages"] = json::array();
    requestBody["messages"].push_back({
        {"role", "user"},
        {"content", fullPrompt}
    });

    // Send request
    HttpClient http;
    http.setHeader("Content-Type", "application/json");
    http.setHeader("x-api-key", _apiKey);
    http.setHeader("anthropic-version", "2023-06-01");

    // Use custom endpoint if set, otherwise use default
    std::string endpoint = _customEndpoint.empty() ? API_ENDPOINT : _customEndpoint;
    std::string response = http.post(endpoint, requestBody.dump());

    // Parse response
    try
    {
        json responseJson = json::parse(response);

        if (responseJson.contains("error"))
        {
            std::string errorMsg = responseJson["error"]["message"].get<std::string>();
            throw std::runtime_error("Claude API error: " + errorMsg);
        }

        if (responseJson.contains("content") && responseJson["content"].size() > 0)
        {
            // Claude returns an array of content blocks
            std::string result;
            for (const auto& block : responseJson["content"])
            {
                if (block["type"] == "text")
                {
                    result += block["text"].get<std::string>();
                }
            }
            return result;
        }

        throw std::runtime_error("Unexpected response format from Claude API");
    }
    catch (const json::exception& e)
    {
        throw std::runtime_error("Failed to parse Claude response: " + std::string(e.what()));
    }
}
