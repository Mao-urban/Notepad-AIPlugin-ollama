//this file is part of notepad_aiplug
//Copyright (C)2025

#ifndef AIPROVIDER_H
#define AIPROVIDER_H

#include <string>
#include <memory>
#include <vector>

// Provider type enumeration
enum class ProviderType
{
    OpenAI = 0,
    Claude = 1,
    Gemini = 2,
	Ollama = 3
};

// Abstract base class for AI providers
class AIProvider
{
public:
    virtual ~AIProvider() = default;

    // Send a prompt to the AI and get a response
    // @param prompt: The user's prompt
    // @param context: Optional context (file content, selected text, etc.)
    // @return: The AI's response
    virtual std::string sendPrompt(const std::string& prompt, const std::string& context = "") = 0;

    // Get the provider name
    virtual std::string getName() const = 0;

    // Get the provider type
    virtual ProviderType getType() const = 0;

    // Set the API key
    virtual void setApiKey(const std::string& key) = 0;

    // Get the API key (masked)
    virtual std::string getApiKeyMasked() const = 0;

    // Check if API key is set
    virtual bool hasApiKey() const = 0;

    // Set the model to use
    virtual void setModel(const std::string& model) = 0;

    // Get the current model
    virtual std::string getModel() const = 0;

    // Get available models
    virtual std::vector<std::string> getAvailableModels() const = 0;

    // Set custom endpoint URL (empty string = use default)
    virtual void setEndpoint(const std::string& endpoint) { _customEndpoint = endpoint; }

    // Get endpoint URL
    virtual std::string getEndpoint() const { return _customEndpoint; }

protected:
    std::string _customEndpoint;
};

// Type alias for provider pointer
using AIProviderPtr = std::unique_ptr<AIProvider>;

#endif // AIPROVIDER_H
