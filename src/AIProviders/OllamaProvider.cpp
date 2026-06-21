//this file is part of notepad_aiplug
//Copyright (C)2025

#include "OllamaProvider.h"
#include "Http/HttpClient.h"
#include "nlohmann/json.hpp"
#include <stdexcept>

using json = nlohmann::json;

const char* OllamaProvider::API_ENDPOINT = "http://127.0.0.1:11434/v1/chat/completions";

ProviderType OllamaProvider::getType() const
{
    return ProviderType::Ollama;
}

std::string OllamaProvider::getName() const
{
    return "Ollama";
}

OllamaProvider::OllamaProvider()
    : _model("")
{
}

void OllamaProvider::setApiKey(const std::string& key)
{
    //Not used by ollama 
}

std::string OllamaProvider::getApiKeyMasked() const
{
    return ""; //Not used by Ollama
}

bool OllamaProvider::hasApiKey() const
{
    return true;
}

void OllamaProvider::setModel(const std::string& model)
{
    _model = model;        
}

std::string OllamaProvider::getModel() const
{
    return _model;
}

std::vector<std::string> OllamaProvider::getAvailableModels() const
{
    std::vector<std::string> models;

    try
    {
        HttpClient http;

        std::string response =
            http.get("http://127.0.0.1:11434/api/tags");

        json responseJson = json::parse(response);

        if (responseJson.contains("models"))
        {
            for (const auto& model : responseJson["models"])
            {
                if (model.contains("name"))
                {
                    models.push_back(
                        model["name"].get<std::string>()
                    );
                }
            }
        }
    }
    catch (...)
    {
        models.push_back("starcoder2:3b");
    }

    return models;
}

std::string OllamaProvider::sendPrompt(const std::string& prompt, const std::string& context)
{
    // Build the full prompt with context
    std::string fullPrompt = prompt;
    if (!context.empty())
    {
        fullPrompt = prompt + "\n\n" + context  ;
    }

    // Build request JSON
    json requestBody;
	requestBody["stream"] = false;
    requestBody["model"] = _model;
    requestBody["messages"] = json::array();
    requestBody["messages"].push_back({
        {"role", "user"},
        {"content", fullPrompt}
    });
    //requestBody["max_tokens"] = 2048;
    
    // Send request
    HttpClient http;
    http.setHeader("Content-Type", "application/json");

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
