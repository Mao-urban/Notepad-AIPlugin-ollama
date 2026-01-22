//this file is part of notepad_aiplug
//Copyright (C)2025

#ifndef GEMINIPROVIDER_H
#define GEMINIPROVIDER_H

#include "AIProvider.h"

class GeminiProvider : public AIProvider
{
public:
    GeminiProvider();
    ~GeminiProvider() override = default;

    std::string sendPrompt(const std::string& prompt, const std::string& context = "") override;
    std::string getName() const override { return "Gemini (Google)"; }
    ProviderType getType() const override { return ProviderType::Gemini; }

    void setApiKey(const std::string& key) override;
    std::string getApiKeyMasked() const override;
    bool hasApiKey() const override;

    void setModel(const std::string& model) override;
    std::string getModel() const override;
    std::vector<std::string> getAvailableModels() const override;

private:
    std::string _apiKey;
    std::string _model;

    static const char* API_BASE_URL;
};

#endif // GEMINIPROVIDER_H
