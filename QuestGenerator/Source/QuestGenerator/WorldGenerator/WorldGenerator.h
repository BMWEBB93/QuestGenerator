// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "ChunkVisualiser.h"
#include "OceanChunk.h"

#include "WorldGenerator.generated.h"

UCLASS()
class QUESTGENERATOR_API AWorldGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Generation")
	void GenerateWorld();	
	

	UPROPERTY()
	TMap<FIntPoint, AOceanChunk*> OceanChunks;
	
	UPROPERTY()
	TMap<FIntPoint, AChunkVisualiser*> ChunkVisualisers;
	
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Settings")
	UMaterialInterface* TerrainMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Settings")
	UMaterialInterface* OceanMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
	int WorldSize = 16; // value is in chunks

	UPROPERTY()
	float ChunkSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Settings")
	float ChunkScale = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Settings")
	int ChunkResolution = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Settings")
	float ChunkUVScale = 200;
};
