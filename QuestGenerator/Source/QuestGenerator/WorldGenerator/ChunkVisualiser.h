// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"

#include "ChunkData.h"


#include "ChunkVisualiser.generated.h"

UCLASS()
class QUESTGENERATOR_API AChunkVisualiser : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChunkVisualiser();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void GenerateChunk(
	const FChunkData* Data,    
	UMaterialInterface* TerrainMaterial,
	FIntPoint LODBorderDir);
	
	UPROPERTY()
	UProceduralMeshComponent* Mesh;
	
	// chunk settings		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float Scale = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	int Resolution = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	int ChunkUVScale = 100;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	int LOD = 1;
	
	UPROPERTY()
	float SeedOffsetX;
	
	UPROPERTY()
	float SeedOffsetY;
	
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FIntPoint WorldCoord = FIntPoint(0 , 0);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FIntPoint ChunkCoord = FIntPoint(0, 0);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	TArray<float> Heights;
	
	
	
	// ocean settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Edge Settings")
	bool bIsInGameWorld = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Edge Settings")
	float OceanDepth = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Edge Settings")
	float OceanFloorNoiseFrequency = 0.0002f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Edge Settings")
	float OceanFloorNoiseScale = 500.f;

};
