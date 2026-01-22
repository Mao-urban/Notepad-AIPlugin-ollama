//this file is part of notepad_aiplug
//Copyright (C)2025

#include "ProviderFactory.h"
#include "OpenAIProvider.h"
#include "ClaudeProvider.h"
#include "GeminiProvider.h"
#include "Utils/Config.h"

AIProviderPtr ProviderFactory::createProvider(int index)
{
    Config& config = Config::getInstance();
    const auto& providers = config.getProviders();

    if (index < 0 || index >= static_cast<int>(providers.size()))
        return nullptr;

    const AIProviderConfig& providerConfig = providers[index];
    return createProviderFromConfig(providerConfig);
}

AIProviderPtr ProviderFactory::createProvider(ProviderType type)
{
    Config& config = Config::getInstance();
    AIProviderPtr provider;

    switch (type)
    {
        case ProviderType::OpenAI:
        {
            auto openai = std::make_unique<OpenAIProvider>();
            openai->setApiKey(config.getOpenAIKey());
            openai->setModel(config.getOpenAIModel());
            provider = std::move(openai);
            break;
        }
        case ProviderType::Claude:
        {
            auto claude = std::make_unique<ClaudeProvider>();
            claude->setApiKey(config.getClaudeKey());
            claude->setModel(config.getClaudeModel());
            provider = std::move(claude);
            break;
        }
        case ProviderType::Gemini:
        {
            auto gemini = std::make_unique<GeminiProvider>();
            gemini->setApiKey(config.getGeminiKey());
            gemini->setModel(config.getGeminiModel());
            provider = std::move(gemini);
            break;
        }
        default:
            return nullptr;
    }

    return provider;
}

AIProviderPtr ProviderFactory::createProviderFromConfig(const AIProviderConfig& config)
{
    AIProviderPtr provider;
    Config& globalConfig = Config::getInstance();

    switch (config.apiType)
    {
        case APIType::OpenAI:
        {
            auto openai = std::make_unique<OpenAIProvider>();
            openai->setApiKey(config.apiKey);
            openai->setModel(globalConfig.getOpenAIModel());
            if (!config.customEndpoint.empty())
            {
                openai->setEndpoint(config.customEndpoint);
            }
            provider = std::move(openai);
            break;
        }
        case APIType::Claude:
        {
            auto claude = std::make_unique<ClaudeProvider>();
            claude->setApiKey(config.apiKey);
            claude->setModel(globalConfig.getClaudeModel());
            if (!config.customEndpoint.empty())
            {
                claude->setEndpoint(config.customEndpoint);
            }
            provider = std::move(claude);
            break;
        }
        case APIType::Gemini:
        {
            auto gemini = std::make_unique<GeminiProvider>();
            gemini->setApiKey(config.apiKey);
            gemini->setModel(globalConfig.getGeminiModel());
            if (!config.customEndpoint.empty())
            {
                gemini->setEndpoint(config.customEndpoint);
            }
            provider = std::move(gemini);
            break;
        }
        default:
            return nullptr;
    }

    return provider;
}

std::string ProviderFactory::getProviderName(int index)
{
    Config& config = Config::getInstance();
    const auto& providers = config.getProviders();

    if (index >= 0 && index < static_cast<int>(providers.size()))
    {
        return providers[index].name;
    }

    return "Unknown";
}
