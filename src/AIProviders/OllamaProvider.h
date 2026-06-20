#pragma once

#include "AIProvider.h"

class OllamaProvider : public AIProvider
{
public:
    OllamaProvider();
	
	ProviderType getType() const override;
	

    std::string sendPrompt(
        const std::string& prompt,
        const std::string& context = "") override;

    std::string getName() const override;

    void setApiKey(const std::string& apiKey) override;
    std::string getApiKeyMasked() const override;
    bool hasApiKey() const override;

    void setModel(const std::string& model) override;
    std::string getModel() const override;

    std::vector<std::string> getAvailableModels() const override;

private:
    static const char* API_ENDPOINT;
    std::string _model;
};
