// Google Analytics Provider
// Created by Patryk Stepniewski
// Copyright (c) 2014-2019 gameDNA Ltd. All Rights Reserved.

#pragma once

#include "Runtime/Analytics/Analytics/Public/Interfaces/IAnalyticsProviderModule.h"
#include "Core.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGoogleAnalytics, Log, All);

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
