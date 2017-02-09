// Google Analytics Provider
// Created by Patryk Stepniewski
// Copyright (c) 2014-2017 gameDNA. All Rights Reserved.

#pragma once

#include "Runtime/Analytics/Analytics/Public/Interfaces/IAnalyticsProviderModule.h"
#include "Core.h"

class IAnalyticsProvider;

class FAnalyticsGoogleAnalytics :
	public IAnalyticsProviderModule
{
	TSharedPtr<IAnalyticsProvider> Provider;

public:
	static inline FAnalyticsGoogleAnalytics& Get()
	{
		return FModuleManager::LoadModuleChecked< FAnalyticsGoogleAnalytics >("GoogleAnalytics");
	}

public:
	virtual TSharedPtr<IAnalyticsProvider> CreateAnalyticsProvider(const FAnalyticsProviderConfigurationDelegate& GetConfigValue) const override;

private:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
