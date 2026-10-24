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
	
	
	// Create noise offset from world seed
	FRandomStream SeedStream(Seed);
	SeedOffsetX = SeedStream.FRandRange(-100000.f, 100000.f);
	SeedOffsetY = SeedStream.FRandRange(-100000.f, 100000.f);
	
	ChunkData.Empty();
	
	// create world data
	for (int y = 0; y <= WorldSize; y++)
	{
		for (int x = 0; x <= WorldSize; x++)
		{
			// create chunk coordinate
			FIntPoint ChunkCoord = FIntPoint(x - WorldSize / 2, y - WorldSize / 2);

			// create chunk data
			FChunkData NewChunkData;
			NewChunkData.WorldCoord = ChunkCoord;
			
			GenerateHeightmap(NewChunkData);

			// Add data to map
			ChunkData.Add(ChunkCoord, NewChunkData);			
		}
	}
	
	// Get amount of chunks loaded along 1 axis
	int TotalLoadedChunksSize = (
		(LOD1RenderDistance + LOD2RenderDistance + LOD3RenderDistance + LOD4RenderDistance)
		* 2)
		+ 1;
	
	
	
	// Start creating chunks
	for (int y = 0; y < TotalLoadedChunksSize; y++)
	{
		for (int x = 0; x < TotalLoadedChunksSize; x++)
		{		
			
			FIntPoint Coord = FIntPoint(x - TotalLoadedChunksSize / 2, y - TotalLoadedChunksSize / 2);
			
			// Spawn Chunk
			FVector SpawnLocation(
		   Coord.X * ChunkSize,
		   Coord.Y * ChunkSize,
		   0.0f);
			
			AChunkVisualiser* Chunk = 
				GetWorld()->SpawnActor<AChunkVisualiser>(
					AChunkVisualiser::StaticClass(),
					SpawnLocation,
					FRotator::ZeroRotator
				);
					
			Chunk->SetFolderPath(TEXT("WorldGeneration/Land"));
			
			// Find LOD of chunk
			Chunk->LOD = GetLOD(Coord);

			// Set chunk variables
			Chunk->Resolution = ChunkResolution;
			Chunk->Scale = ChunkScale;
			Chunk->ChunkUVScale = ChunkUVScale;
			
			Chunk->WorldCoord = Coord;
			Chunk->ChunkCoord = Coord;
			
			Chunk->OceanDepth = OceanDepth;
			
			// add chunk to map
			ChunkVisualisers.Add(FIntPoint(x, y), Chunk);			
			
			// get the direction of any LOD transitions
			FIntPoint LODBoundary = GetLODTransitionBoundary(Chunk->LOD, Coord);	
			
			// Get the chunk data
			FChunkData* Data = ChunkData.Find(Coord);
			
			Chunk->bIsInGameWorld = (Data != nullptr);
			
			// Generate mesh with terrain data
			Chunk->GenerateChunk(Data, TerrainMaterial, LODBoundary);
		}
	}	
	
	// Generate Ocean
	for (int y = 0; y < TotalLoadedChunksSize; y++)
	{
		for (int x = 0; x < TotalLoadedChunksSize; x++)
		{
			FIntPoint Coord = FIntPoint(x - TotalLoadedChunksSize / 2, y - TotalLoadedChunksSize / 2);
			
			// Spawn Chunk
			FVector SpawnLocation(
		   Coord.X * ChunkSize,
		   Coord.Y * ChunkSize,
		   0.0f);
			
			AOceanChunk* OceanChunk = 
				GetWorld()->SpawnActor<AOceanChunk>(
					AOceanChunk::StaticClass(),
					SpawnLocation,
					FRotator::ZeroRotator
				);
			
			OceanChunk->SetFolderPath(TEXT("WorldGeneration/Ocean"));
					
			OceanChunk->LOD = GetLOD(Coord);
			OceanChunk->WorldCoord = Coord;
			OceanChunk->ChunkCoord = Coord;
			
			OceanChunk->Resolution = ChunkResolution;
			OceanChunk->Scale = ChunkScale;
			OceanChunk->UVScale = ChunkUVScale;
			
			OceanChunks.Add(FIntPoint(x, y), OceanChunk);

			OceanChunk->GenerateChunk(OceanMaterial);
		}
	}	
}

void AWorldGenerator::GenerateHeightmap(FChunkData& chunkData)
{
	int VertexCount = ChunkResolution + 1;

	chunkData.Heights.Init(0.f, VertexCount * VertexCount); // Sets size of heightmap
	
	for (int y = 0; y <= ChunkResolution; y++)
	{
		for (int x = 0; x <= ChunkResolution; x++)
		{
			// get the coordinate of the vertex from the array
			int Index = x + (y * VertexCount);

			// get the world location of the vertex
			float WorldX = x * ChunkScale + (chunkData.WorldCoord.X * ChunkSize);
			float WorldY = y * ChunkScale + (chunkData.WorldCoord.Y * ChunkSize);
			
			// add a large scale noise that shapes continents
			float Noise1 = (
				FMath::PerlinNoise2D(
					FVector2D(
						WorldX + SeedOffsetX,
						WorldY + SeedOffsetY
					) * Noise1Frequency
				) + 1.0f
			) / 2.0f;
			
			
			float ContinentHeight = ContinentBaseHeight + (Noise1 * Noise1Scale);
			float Height = ContinentHeight;		

			// detail noise layers
			float Noise2 = FMath::PerlinNoise2D(FVector2D(WorldX + SeedOffsetX, WorldY + SeedOffsetY) * Noise2Frequency);
			float Noise3 = FMath::PerlinNoise2D(FVector2D(WorldX + SeedOffsetX, WorldY + SeedOffsetY) * Noise3Frequency);
			
			// Add noise together to get height
			Height += (Noise2 * Noise2Scale) + (Noise3 * Noise3Scale);		

			// set height in the chunk data heightmap
			chunkData.Heights[Index] = Height;
		}
	}

}

int AWorldGenerator::GetLOD(FIntPoint Coord)
{	
	int LOD = 1;
	if (FMath::Abs(Coord.X) > LOD1RenderDistance || FMath::Abs(Coord.Y) > LOD1RenderDistance)
	{
		LOD = 2;
	}
			
	if (FMath::Abs(Coord.X) > LOD1RenderDistance + LOD2RenderDistance ||
		FMath::Abs(Coord.Y) > LOD1RenderDistance + LOD2RenderDistance)
	{
		LOD = 3;
	}

	if (FMath::Abs(Coord.X) > LOD1RenderDistance + LOD2RenderDistance + LOD3RenderDistance || 
		FMath::Abs(Coord.Y) > LOD1RenderDistance + LOD2RenderDistance + LOD3RenderDistance)
	{
		LOD = 4;
	}
	return LOD;
}

FIntPoint AWorldGenerator::GetLODTransitionBoundary(int LOD, FIntPoint Coord)
{
	int CurrentLODMaxDistance = 0;
	FIntPoint LODBoundary = FIntPoint(0, 0);

	if (LOD == 4) return LODBoundary; // LOD 4 doesnt need stitching

	// Get the distance required for chunk to be on edge of lod
	switch (LOD)
	{
	case 1: CurrentLODMaxDistance = LOD1RenderDistance;
		break;

	case 2: CurrentLODMaxDistance = LOD1RenderDistance + LOD2RenderDistance;
		break;

	case 3: CurrentLODMaxDistance = LOD1RenderDistance + LOD2RenderDistance + LOD3RenderDistance;
		break;
	}

	// check chunk is on the edge
	if (Coord.X ==  CurrentLODMaxDistance) LODBoundary.X =  1;
	if (Coord.X == -CurrentLODMaxDistance) LODBoundary.X = -1;

	if (Coord.Y ==  CurrentLODMaxDistance) LODBoundary.Y =  1;
	if (Coord.Y == -CurrentLODMaxDistance) LODBoundary.Y = -1;	

	return LODBoundary;
}

