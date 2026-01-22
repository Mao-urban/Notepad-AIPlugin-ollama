//this file is part of notepad_aiplug
//Copyright (C)2025

#ifndef PROVIDERFACTORY_H
#define PROVIDERFACTORY_H

#include "AIProvider.h"
#include "Utils/Config.h"

class ProviderFactory
{
public:
    // Create a provider by index from Config (supports dynamic provider list)
    static AIProviderPtr createProvider(int index);

    // Create a provider by type (legacy, uses built-in providers)
    static AIProviderPtr createProvider(ProviderType type);

    // Create a provider from config struct
    static AIProviderPtr createProviderFromConfig(const AIProviderConfig& config);

    // Get provider count from Config
    static int getProviderCount() {
        return static_cast<int>(Config::getInstance().getProviders().size());
    }

    // Get provider name by index from Config
    static std::string getProviderName(int index);
};

#endif // PROVIDERFACTORY_H
