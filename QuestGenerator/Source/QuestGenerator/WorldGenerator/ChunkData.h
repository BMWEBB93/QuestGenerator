// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkData.generated.h"

/**
 * 
 */
USTRUCT()
struct QUESTGENERATOR_API FChunkData
{
	GENERATED_BODY()
	
	
	UPROPERTY()
	TArray<float> Heights; // Heightmap 
	
	UPROPERTY()
	FIntPoint WorldCoord = FIntPoint(0, 0);
};
