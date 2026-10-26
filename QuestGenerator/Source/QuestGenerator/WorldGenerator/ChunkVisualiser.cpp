// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkVisualiser.h"

#include "KismetProceduralMeshLibrary.h"


// Sets default values
AChunkVisualiser::AChunkVisualiser()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
}

// Called when the game starts or when spawned
void AChunkVisualiser::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AChunkVisualiser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UFUNCTION(CallInEditor)
void AChunkVisualiser:: GenerateChunk(
const FChunkData* Data,    
UMaterialInterface* TerrainMaterial,
FIntPoint LODBorderDir)
{
	Mesh->ClearMeshSection(0);
	
	float ChunkSize = Resolution * Scale;

	TArray<FVector> Vertices;
	TArray<int32> TrianglesInner;

	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> Colours;
	TArray<FProcMeshTangent> Tangents;  
	
	// amount of verts x and y represent in LOD
	int LODStep =  1 << (LOD - 1);
	
	int LODAdjustedResolution = Resolution / LODStep;
	float LODAdjustedScale = Scale * LODStep;
	
	// Create Vertices
	for (int y = 0; y <= LODAdjustedResolution; y++)
	{
		
		for (int x = 0; x <= LODAdjustedResolution; x++)
		{			
			// 
			bool bIsLeftEdge = x * LODStep == 0;
			bool bIsRightEdge = x * LODStep == Resolution;
			bool bIsBottomEdge = y * LODStep == 0;
			bool bIsTopEdge = y * LODStep == Resolution;
						
			
			auto GetHeight = [&](int SampleX, int SampleY) -> float
			{
				int Index = SampleY * LODStep * (Resolution + 1) + SampleX * LODStep;
				
				if (bIsInGameWorld)
				{
					if (Data)
					{
						return Data->Heights[Index];
					}					
				}
				
				// get the world coordinate of vetices
				float WorldX = WorldCoord.X * ChunkSize + x * LODAdjustedScale;
				float WorldY = WorldCoord.Y * ChunkSize + y * LODAdjustedScale;
				
				float OceanNoise1 = (FMath::PerlinNoise2D(FVector2D(
					WorldX + SeedOffsetX, 
					WorldY + SeedOffsetY)  * OceanFloorNoiseFrequency) + 1) / 2;
				
				
				float OceanNoise2 = (FMath::PerlinNoise2D(FVector2D(
					WorldX + SeedOffsetX, 
					WorldY + SeedOffsetY)  * OceanFloorNoiseFrequency * 8) + 1) / 2;
				
				return  -OceanDepth + (OceanNoise1 * OceanFloorNoiseScale) + (OceanNoise2 * OceanFloorNoiseScale/5);
			};
						
			
			float Height = GetHeight(x, y);
			
			//&& x % LODStep == 0
			
		   // Vert is in bottom edge and needs to be adjusted, skips verts on true coordinates
            if (LODBorderDir.Y < 0 && bIsBottomEdge && x % 2 != 0)
            {                    
                int32 PrevX = (x / 2) * 2;                   
                float PrevHeight =  GetHeight(PrevX, y);
                
                int32 NextX = FMath::Min(PrevX + 2, LODAdjustedResolution);
                float NextHeight = GetHeight(NextX, y);
                
                float HeightRatio = float(x - PrevX) / float(NextX - PrevX);
                Height = FMath::Lerp(PrevHeight, NextHeight, HeightRatio);                    
            }
            
            // Vert is in top edge and needs to be adjusted
            if (LODBorderDir.Y > 0 && bIsTopEdge && x % 2 != 0)
            {
                int32 PrevX = (x / 2) * 2;                   
                float PrevHeight = GetHeight(PrevX, y);
                
                int32 NextX = FMath::Min(PrevX + 2, LODAdjustedResolution);
                float NextHeight = GetHeight(NextX, y);;
                
                float HeightRatio = float(x - PrevX) / float(NextX - PrevX);
                Height = FMath::Lerp(PrevHeight, NextHeight, HeightRatio);                    
            }
            
            // Vert is in Right edge and needs to be adjusted
            if (LODBorderDir.X > 0 && bIsRightEdge && y % 2 != 0)
            {
                int32 PrevY = (y / 2) * 2;
                float PrevHeight = GetHeight(x, PrevY);
                
                int32 NextY = FMath::Min(PrevY + 2, LODAdjustedResolution);                 
                float NextHeight = GetHeight(x, NextY);
                
                float HeightRatio = float(y - PrevY) / float(NextY - PrevY);
                Height = FMath::Lerp(PrevHeight, NextHeight, HeightRatio);                    
            }
            
            // Vert is in Left edge and needs to be adjusted
            if (LODBorderDir.X < 0 && bIsLeftEdge && y % 2 != 0)
            {
                int32 PrevY = (y / 2) * 2;
                float PrevHeight = GetHeight(x, PrevY);
                
                int32 NextY = FMath::Min(PrevY + 2, LODAdjustedResolution);                  
                float NextHeight = GetHeight(x, NextY);
                
                float HeightRatio = float(y - PrevY) / float(NextY - PrevY);
                Height = FMath::Lerp(PrevHeight, NextHeight, HeightRatio);                    
            }
			         
			
			Vertices.Add(FVector(x * Scale * LODStep, y * Scale * LODStep, Height));
			UVs.Add(FVector2D(
				(float)x / ChunkUVScale * LODAdjustedScale,
				(float)y / ChunkUVScale * LODAdjustedScale)
				);                

		}
	}
	
	// Triangles
	for (int y = 0; y < LODAdjustedResolution; y++)
	{
		for (int x = 0; x < LODAdjustedResolution; x++)
		{
			int BottomLeft = y * (LODAdjustedResolution + 1) + x;
			int BottomRight = y * (LODAdjustedResolution + 1) + x + 1;
			int TopLeft = (y + 1) * (LODAdjustedResolution + 1) + x;
			int TopRight = (y + 1) * (LODAdjustedResolution + 1) + x + 1;

			TrianglesInner.Add(BottomLeft);
			TrianglesInner.Add(TopLeft);
			TrianglesInner.Add(TopRight);

			TrianglesInner.Add(TopRight);
			TrianglesInner.Add(BottomRight);
			TrianglesInner.Add(BottomLeft);
		}
	}
        
	// create tangents
	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
		Vertices,
		TrianglesInner,
		UVs,
		Normals,
		Tangents
	);
        
	// Create mesh
	Mesh->CreateMeshSection_LinearColor(
		0,          // section index
		Vertices,
		TrianglesInner,
		Normals,
		UVs,
		Colours,
		Tangents,
		true        // collision
	);
        
	Mesh->SetMaterial(0, TerrainMaterial);
}

