// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "ChunkVisualiser.h"
#include "OceanChunk.h"
#include "ChunkData.h"

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
	
	void GenerateHeightmap(FChunkData& chunkData);
	
	int GetLOD(FIntPoint Coord);
	FIntPoint GetLODTransitionBoundary(int LOD, FIntPoint Coord);

	// Handle shifting chunks depending on player "shifts to make"
	void HandleChunkShifting();
	
	// shift land chuks when player moves to a new chunk
	void ShiftLandChunks(FIntPoint Direction);

	// shift ocean chuks when player moves to a new chunk
	void ShiftOceanChunks(FIntPoint Direction);
	
	UPROPERTY()
	TMap<FIntPoint, FChunkData> ChunkData;
	
	UPROPERTY()
	TMap<FIntPoint, AOceanChunk*> OceanChunks;
	
	UPROPERTY()
	TMap<FIntPoint, AChunkVisualiser*> ChunkVisualisers;
	
	// build queues for land chunks
	UPROPERTY()
	TArray<AChunkVisualiser*> PendingChunkRebuilds;
	
	// build queues for ocean chunks
	UPROPERTY()
	TArray<AOceanChunk*> PendingOceanRebuilds;
	
	
	// player reference used to move chunks
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ACharacter* Player;

	UPROPERTY()
	FIntPoint LastPlayerCoord = FIntPoint(0, 0);

	UPROPERTY()
	FIntPoint PlayerChunkCoord = FIntPoint(0, 0);
	

	
	/////////////////////////
	/// World Settings
	////////////////////////
	
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
	int WorldSize = 16; // value is in chunks

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
	int Seed = 12345;
	
	// how many chunks can rebuild per tick
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
	int BuildsPerTick = 2;
	
	// time allowed for chunk rebuilds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
	float TimePerTick = 0.0333f;
	
	
	UPROPERTY()
	float SeedOffsetX;
	
	UPROPERTY()
	float SeedOffsetY;
	
	// height before noise is applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
	float ContinentBaseHeight = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
	float OceanDepth = 1000.f;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
	float OceanFloorNoiseFrequency = 0.0002f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
	float OceanFloorNoiseScale = 500.f;
	
	
	// Noise 1
	// frequency of sampled noise for heightmap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
	float Noise1Frequency = 0.00005f;

	// ratio of height this noise affects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
	float Noise1Scale = 2000.f;

	// Noise 2
	// frequency of sampled noise for heightmap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
	float Noise2Frequency = 0.0002f;

	// ratio of height this noise affects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
	float Noise2Scale = 400.f;

	// Noise 3
	// frequency of sampled noise for heightmap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
	float Noise3Frequency = 0.005f;

	// ratio of height this noise affects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
	float Noise3Scale = 50.f;
		
	
	/////////////////////////
	/// Chunk settings
	/////////////////////////
	
	UPROPERTY()
	float ChunkSize;
	
	UPROPERTY()
	FIntPoint ShiftsToMake = FIntPoint(0,0 );
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Settings")
	UMaterialInterface* TerrainMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Settings")
	UMaterialInterface* OceanMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Settings")
	float ChunkScale = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Settings")
	int ChunkResolution = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Settings")
	float ChunkUVScale = 200;
	
	
	////////////////////////////
	/// Lod settings
	////////////////////////////
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lod")
	int LOD1RenderDistance = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lod")
	int LOD2RenderDistance = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lod")
	int LOD3RenderDistance = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lod")
	int LOD4RenderDistance = 10;
};
