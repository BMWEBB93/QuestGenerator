// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGenerator.h"

// Sets default values
AWorldGenerator::AWorldGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWorldGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWorldGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWorldGenerator::GenerateWorld()
{
	// Delete current chunks
	for (auto& Pair : ChunkVisualisers)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->Destroy();
		}
	}
	ChunkVisualisers.Empty();
	
	// Delete current ocean chunks
	for (auto& Pair : OceanChunks)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->Destroy();
		}
	}
	OceanChunks.Empty();
	
	// Ensure resolution is 32, 64, 128 ect.
	ChunkResolution = FMath::Max(ChunkResolution, 1);
	ChunkResolution = 1 << FMath::CeilLogTwo(ChunkResolution);
	ChunkSize = ChunkScale * ChunkResolution;
	
	// Generate Land
	FVector GenerationStartPoint = FVector(-ChunkSize * WorldSize / 2, -ChunkSize * WorldSize / 2, 0);
	for (int y = 0; y < WorldSize; y++)
	{
		for (int x = 0; x < WorldSize; x++)
		{
			FVector SpawnLocation = GenerationStartPoint + FVector(x * ChunkSize, y * ChunkSize, 0);
			
			AChunkVisualiser* Chunk = 
				GetWorld()->SpawnActor<AChunkVisualiser>(
					AChunkVisualiser::StaticClass(),
					SpawnLocation,
					FRotator::ZeroRotator
				);
			
			Chunk->Resolution = ChunkResolution;
			Chunk->Scale = ChunkScale;
			Chunk->ChunkUVScale = ChunkUVScale;
			
			ChunkVisualisers.Add(FIntPoint(x, y), Chunk);
			
			Chunk->GenerateChunk(TerrainMaterial);
		}
	}	
	
	// Generate Ocean
	for (int y = 0; y < WorldSize; y++)
	{
		for (int x = 0; x < WorldSize; x++)
		{
			FVector SpawnLocation = GenerationStartPoint + FVector(x * ChunkSize, y * ChunkSize, 0);
			
			AOceanChunk* OceanChunk = 
				GetWorld()->SpawnActor<AOceanChunk>(
					AOceanChunk::StaticClass(),
					SpawnLocation,
					FRotator::ZeroRotator
				);
						
			OceanChunk->Resolution = ChunkResolution;
			OceanChunk->Scale = ChunkScale;
			OceanChunk->UVScale = ChunkUVScale;
			
			OceanChunks.Add(FIntPoint(x, y), OceanChunk);

			OceanChunk->GenerateChunk(OceanMaterial);
		}
	}	
}

