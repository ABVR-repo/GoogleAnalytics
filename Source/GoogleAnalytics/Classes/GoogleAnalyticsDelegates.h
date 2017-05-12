// Ultimate Mobile Kit
// Created by Patryk Stepniewski
// Copyright (c) 2014-2017 gameDNA. All Rights Reserved.

#pragma once

#include "EngineMinimal.h"
#include "CoreMinimal.h"
#include "GoogleAnalyticsDelegates.generated.h"


USTRUCT(BlueprintType)
struct FCustomDimension
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	int32 Index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	FString Value;

	FCustomDimension()
	{
		Index = 0;
		Value = FString("");
	}
};


USTRUCT(BlueprintType)
struct FCustomMetric
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	int32 Index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	float Value;

	FCustomMetric()
	{
		Index = 0;
		Value = 0;
	}
};
