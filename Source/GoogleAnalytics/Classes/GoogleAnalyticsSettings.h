// Google Analytics Provider
// Created by Patryk Stepniewski
// Copyright (c) 2014-2017 gameDNA. All Rights Reserved.

#pragma once

#include "GoogleAnalyticsSettings.generated.h"

UCLASS(config = Engine, defaultconfig)
class UGoogleAnalyticsSettings : public UObject
{
	GENERATED_BODY()

public:
	UGoogleAnalyticsSettings(const FObjectInitializer& ObjectInitializer);
};
