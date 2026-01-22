//this file is part of notepad_aiplug
//Copyright (C)2025

#include "GeminiProvider.h"
#include "Http/HttpClient.h"
#include "nlohmann/json.hpp"
#include <stdexcept>

using json = nlohmann::json;

const char* GeminiProvider::API_BASE_URL = "https://generativelanguage.googleapis.com/v1/models/";

GeminiProvider::GeminiProvider()
    : _model("gemini-1.5-pro")
{
}

void GeminiProvider::setApiKey(const std::string& key)
{
    _apiKey = key;
}

std::string GeminiProvider::getApiKeyMasked() const
{
    if (_apiKey.empty())
        return "";
    if (_apiKey.length() <= 8)
        return std::string(_apiKey.length(), '*');
    return _apiKey.substr(0, 4) + std::string(_apiKey.length() - 8, '*') + _apiKey.substr(_apiKey.length() - 4);
}

bool GeminiProvider::hasApiKey() const
{
    return !_apiKey.empty();
}

void GeminiProvider::setModel(const std::string& model)
{
    _model = model;
}

std::string GeminiProvider::getModel() const
{
    return _model;
}

std::vector<std::string> GeminiProvider::getAvailableModels() const
{
    return {
        "gemini-1.5-pro",
        "gemini-1.5-flash",
        "gemini-1.5-flash-8b",
        "gemini-1.0-pro"
    };
}

std::string GeminiProvider::sendPrompt(const std::string& prompt, const std::string& context)
{
    if (_apiKey.empty())
    {
        throw std::runtime_error("Gemini API key not set. Please configure it in Settings.");
    }

    // Build the full prompt with context
    std::string fullPrompt = prompt;
    if (!context.empty())
    {
        fullPrompt = context + "\n\nUser request: " + prompt;
    }

    // Add system instruction to prompt
    std::string systemPrompt = "You are a helpful coding assistant integrated into Notepad++. "
                               "Help the user with their code-related questions and tasks. "
                               "When providing code, be concise and provide only the relevant code. "
                               "If the user asks you to modify code, provide the complete modified code.\n\n";

    // Build request JSON
    json requestBody;
    requestBody["contents"] = json::array();
    requestBody["contents"].push_back({
        {"parts", json::array({{{"text", systemPrompt + fullPrompt}}})}
    });
    requestBody["generationConfig"] = {
        {"maxOutputTokens", 4096}
    };

    // Build URL with API key
    // Use custom endpoint if set, otherwise use default
    std::string url;
    if (!_customEndpoint.empty())
    {
        url = _customEndpoint;
        // Append API key if not already in URL
        if (url.find("key=") == std::string::npos)
        {
            url += (url.find('?') == std::string::npos ? "?" : "&");
            url += "key=" + _apiKey;
        }
    }
    else
    {
        url = std::string(API_BASE_URL) + _model + ":generateContent?key=" + _apiKey;
    }

    // Send request
    HttpClient http;
    http.setHeader("Content-Type", "application/json");

    std::string response = http.post(url, requestBody.dump());

    // Parse response
    try
    {
        json responseJson = json::parse(response);

        if (responseJson.contains("error"))
        {
            std::string errorMsg = responseJson["error"]["message"].get<std::string>();
            throw std::runtime_error("Gemini API error: " + errorMsg);
        }

        if (responseJson.contains("candidates") && responseJson["candidates"].size() > 0)
        {
            auto& candidate = responseJson["candidates"][0];
            if (candidate.contains("content") && candidate["content"].contains("parts"))
            {
                std::string result;
                for (const auto& part : candidate["content"]["parts"])
                {
                    if (part.contains("text"))
                    {
                        result += part["text"].get<std::string>();
                    }
                }
                return result;
            }
        }

        throw std::runtime_error("Unexpected response format from Gemini API");
    }
    catch (const json::exception& e)
    {
        throw std::runtime_error("Failed to parse Gemini response: " + std::string(e.what()));
    }
}
