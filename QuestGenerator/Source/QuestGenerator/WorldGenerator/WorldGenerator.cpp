// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"


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

	// Get the player and set position in middle of chunk
	Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0); // get the player	
}



// Called every frame
void AWorldGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Get player if it isn't set
	if (!Player)
	{
		Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		if (Player)
		{
			PlayerChunkCoord = FIntPoint(
				FMath::FloorToInt(Player->GetActorLocation().X / ChunkSize),
				FMath::FloorToInt(Player->GetActorLocation().Y / ChunkSize)
			);
			LastPlayerCoord = PlayerChunkCoord; 
		}
	}

	// get the players current chunk position
	PlayerChunkCoord = FIntPoint(
		FMath::FloorToInt(Player->GetActorLocation().X / ChunkSize),
		FMath::FloorToInt(Player->GetActorLocation().Y / ChunkSize)
		);
	
	// find if the player has moved to another chunk
	FIntPoint ChunkShiftDirection = PlayerChunkCoord - LastPlayerCoord;	
	ShiftsToMake += ChunkShiftDirection;
	LastPlayerCoord = PlayerChunkCoord;
	
	// shift the chunks towards the direction the player moves
	HandleChunkShifting();
	
	
	double StartTime = FPlatformTime::Seconds();
	
	// Generate queued chunks (Land)
	
	// get max chunks to build this tick
	int ChunksToBuild = FMath::Min(BuildsPerTick, PendingChunkRebuilds.Num());
	
	if (FPlatformTime::Seconds() - StartTime  < TimePerTick) // if within time limit
	{
		for (int i = 0; i < ChunksToBuild; i++)  // loop through build queue
		{
			// stop generating if exceeded the time limit
			if (FPlatformTime::Seconds() - StartTime >= TimePerTick) break;

			// Check if chunk is in game world
			FChunkData* data = nullptr;			
			data = ChunkData.Find(PendingChunkRebuilds[0]->WorldCoord);			
			PendingChunkRebuilds[0]->bIsInGameWorld = (data != nullptr);
			
			// generate the chunk
			PendingChunkRebuilds[0]->GenerateChunk(data, TerrainMaterial, PendingChunkRebuilds[0]->LODBoundary);		

			// remove from build queue
			PendingChunkRebuilds.RemoveAt(0);
		}
	}
	

	// Generate queued chunks (Ocean)
	
	// get max chunks to build this tick
	int OceanChunksToBuild = FMath::Min(BuildsPerTick, PendingOceanRebuilds.Num());
	
	if (FPlatformTime::Seconds() - StartTime <= TimePerTick) // if within time limit
	{
		for (int i = 0; i < OceanChunksToBuild; i++)  // loop through build queue
		{
			// if time has been exceeded stop generating chunks
			if (FPlatformTime::Seconds() - StartTime > TimePerTick) break;		
			
			PendingOceanRebuilds[0]->GenerateChunk(OceanMaterial);
			PendingOceanRebuilds.RemoveAt(0); 
		}
	}
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
	for (int y = 0; y < WorldSize; y++)
	{
		for (int x = 0; x < WorldSize; x++)
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
			Chunk->LODBoundary = LODBoundary;		
			
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
			float Noise1 =
				FMath::PerlinNoise2D(
					FVector2D(
						WorldX + SeedOffsetX,
						WorldY + SeedOffsetY
					) * Noise1Frequency
				);
			
			
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


void AWorldGenerator::HandleChunkShifting()
{
	if (ShiftsToMake.X > 0)
	{
		ShiftsToMake.X--;
		ShiftLandChunks(FIntPoint(1,0));
		ShiftOceanChunks(FIntPoint(1, 0));
	}
	if (ShiftsToMake.X < 0)
	{
		ShiftsToMake.X++;
		ShiftLandChunks(FIntPoint(-1, 0));
		ShiftOceanChunks(FIntPoint(-1, 0));
	}
	if (ShiftsToMake.Y > 0)
	{
		ShiftsToMake.Y--;
		ShiftLandChunks(FIntPoint(0, 1));
		ShiftOceanChunks(FIntPoint(0, 1));
	}
	if (ShiftsToMake.Y < 0)
	{
		ShiftsToMake.Y++; ;
		ShiftLandChunks(FIntPoint(0, -1));
		ShiftOceanChunks(FIntPoint(0, -1));
	}
}

void AWorldGenerator::ShiftLandChunks(FIntPoint Direction)
{
	// total render distance of chunks of all LODs
	int TotalChunkViewDistance = 
		LOD1RenderDistance + 
			LOD2RenderDistance +
				LOD3RenderDistance + 
					LOD4RenderDistance;
	
	// Size of loaded chunks across one axis (in chunks)
	int ChunkAxisSize = (TotalChunkViewDistance * 2) + 1;
	
	// Distance chunk should move when being wrapped
	float ChunkMoveDistance = ChunkAxisSize * ChunkSize;
	
	// loop through rendered chunks
	for (auto& Pair : ChunkVisualisers)
	{
		AChunkVisualiser* Chunk = Pair.Value;
		
		bool bShouldRegenerate = false; // should chunk rebuild mesh
		bool bWrapChunk = false; // should chunk move to other edge
		
		// check if chunk should be moved
		if (Direction.X > 0)
		{
			bWrapChunk = Chunk->ChunkCoord.X == -TotalChunkViewDistance;
		}
		else if (Direction.X < 0)
		{
			bWrapChunk = Chunk->ChunkCoord.X == TotalChunkViewDistance;
		}
		else if (Direction.Y > 0)
		{
			bWrapChunk = Chunk->ChunkCoord.Y == -TotalChunkViewDistance;
		}
		else if (Direction.Y < 0)
		{
			bWrapChunk = Chunk->ChunkCoord.Y == TotalChunkViewDistance;
		}
		
		
		if (bWrapChunk) // if chunk is moved to other side of chunks
		{
			// move chunk to new edge
			FVector NewLocation =
				Chunk->GetActorLocation() +
				FVector(
					Direction.X * ChunkMoveDistance,
					Direction.Y * ChunkMoveDistance,
					0.f);

			Chunk->SetActorLocation(NewLocation);
			
			// Update Chunk Coordinates
			Chunk->ChunkCoord = FIntPoint(
				Chunk->ChunkCoord.X + (ChunkAxisSize - 1) * Direction.X,
				Chunk->ChunkCoord.Y + (ChunkAxisSize - 1) * Direction.Y);
			
			Chunk->WorldCoord = FIntPoint(
				Chunk->WorldCoord.X + ChunkAxisSize * Direction.X, 
				Chunk->WorldCoord.Y + ChunkAxisSize * Direction.Y);
				
			
			bShouldRegenerate = true; // wrapped chunks always need to rebuild mesh
			
			// clears the mesh of chunk 
			if (Chunk->Mesh)
			{
				Chunk->Mesh->ClearMeshSection(0);				
			}
		}
		else // chunk didn't move 
		{
			Chunk->ChunkCoord -= Direction; // update coord from player
		}
		
		// Update LODs and regenerate if changed
		int LastLOD = Chunk->LOD;
		int NewLOD = GetLOD(Chunk->ChunkCoord);
		if (NewLOD != LastLOD)
		{
			Chunk->LOD = NewLOD;
			bShouldRegenerate = true;
		}
		
		// get the direction of any LOD transitions
		FIntPoint LastLODBoundary = Chunk->LODBoundary;
		FIntPoint NewLODBoundary = GetLODTransitionBoundary(Chunk->LOD, Chunk->ChunkCoord);
		Chunk->LODBoundary = NewLODBoundary;
		if (LastLODBoundary != NewLODBoundary)
		{
			bShouldRegenerate = true;
		}
		
		if (bShouldRegenerate)
		{
			PendingChunkRebuilds.Add(Chunk);
		}
	}
	
	// sort chunks with LOD 1 generating first
	PendingChunkRebuilds.Sort(
		[](AChunkVisualiser& A, AChunkVisualiser& B)
		{
			return FMath::Max(
				FMath::Abs(A.ChunkCoord.X), 
				FMath::Abs(A.ChunkCoord.Y)) < FMath::Max(
					FMath::Abs(B.ChunkCoord.X), 
					FMath::Abs(B.ChunkCoord.Y));
		});	
}

void AWorldGenerator::ShiftOceanChunks(FIntPoint Direction)
{
	// total render distance of chunks of all LODs
	int TotalChunkViewDistance = 
	LOD1RenderDistance + 
		LOD2RenderDistance +
			LOD3RenderDistance + 
				LOD4RenderDistance;
	
	// Size of loaded chunks across one axis (in chunks)
	int ChunkAxisSize = (TotalChunkViewDistance * 2) + 1;
	
	// Distance chunk should move when being wrapped
	float ChunkMoveDistance = ChunkAxisSize * ChunkSize;
	
	// loop through rendered chunks
	for (auto& Pair : OceanChunks)
	{
		AOceanChunk* Chunk = Pair.Value;
		
		bool bShouldRegenerate = false; // should chunk rebuild mesh
		bool bWrapChunk = false; // should chunk move to other edge
		
		// check if chunk should be moved
		if (Direction.X > 0)
		{
			bWrapChunk = Chunk->ChunkCoord.X == -TotalChunkViewDistance;
		}
		else if (Direction.X < 0)
		{
			bWrapChunk = Chunk->ChunkCoord.X == TotalChunkViewDistance;
		}
		else if (Direction.Y > 0)
		{
			bWrapChunk = Chunk->ChunkCoord.Y == -TotalChunkViewDistance;
		}
		else if (Direction.Y < 0)
		{
			bWrapChunk = Chunk->ChunkCoord.Y == TotalChunkViewDistance;
		}
		
		if (bWrapChunk) // if chunk is moved to other side of chunks
		{
			// move chunk to new edge
			FVector NewLocation =
				Chunk->GetActorLocation() +
				FVector(
					Direction.X * ChunkMoveDistance,
					Direction.Y * ChunkMoveDistance,
					0.f);

			Chunk->SetActorLocation(NewLocation);
			
			// Update Chunk Coordinates
			Chunk->ChunkCoord = FIntPoint(
				Chunk->ChunkCoord.X + (ChunkAxisSize - 1) * Direction.X,
				Chunk->ChunkCoord.Y + (ChunkAxisSize - 1) * Direction.Y);
			
			Chunk->WorldCoord = FIntPoint(
				Chunk->WorldCoord.X + ChunkAxisSize * Direction.X, 
				Chunk->WorldCoord.Y + ChunkAxisSize * Direction.Y);
			
			
			
			bShouldRegenerate = true; // wrapped chunks always need to rebuild mesh
			
			// clears the mesh of chunk 
			if (Chunk->Mesh)
			{
				Chunk->Mesh->ClearMeshSection(0);				
			}
		}
		else // chunk didn't move 
		{
			Chunk->ChunkCoord -= Direction; // update coord from player
		}
		
		// Update LODs and regenerate if changed
		int LastLOD = Chunk->LOD;
		int NewLOD = GetLOD(Chunk->ChunkCoord);
		if (NewLOD != LastLOD)
		{
			Chunk->LOD = NewLOD;
			bShouldRegenerate = true;
		}
				
		if (bShouldRegenerate)
		{
			PendingOceanRebuilds.Add(Chunk);
		}
	}
	
	// sort chunks with LOD 1 generating first
	PendingOceanRebuilds.Sort(
		[](AOceanChunk& A, AOceanChunk& B)
		{
			return FMath::Max(
				FMath::Abs(A.ChunkCoord.X), 
				FMath::Abs(A.ChunkCoord.Y)) < FMath::Max(
					FMath::Abs(B.ChunkCoord.X), 
					FMath::Abs(B.ChunkCoord.Y));
		});
}


