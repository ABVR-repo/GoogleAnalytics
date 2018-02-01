// Google Analytics Provider
// Created by Patryk Stepniewski
// Copyright (c) 2014-2018 gameDNA Ltd. All Rights Reserved.

#pragma once

#include "CoreUObject.h"
#include "Engine.h"
#include "GoogleAnalyticsSettings.generated.h"

UCLASS(config = Engine, defaultconfig)
class GOOGLEANALYTICS_API UGoogleAnalyticsSettings : public UObject
{
	GENERATED_BODY()

public:
	UGoogleAnalyticsSettings(const FObjectInitializer& ObjectInitializer);

	/** Enable IDFA Collection - allows to track personal user informations on iOS */
	UPROPERTY(Config, EditAnywhere, Category = "Google Analytics", DisplayName = "Enable IDFA Collection")
	bool bEnableIDFACollection;
};
