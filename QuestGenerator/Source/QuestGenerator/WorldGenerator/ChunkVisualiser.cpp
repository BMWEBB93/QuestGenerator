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

void AChunkVisualiser::GenerateChunk(UMaterialInterface* TerrainMaterial)
{
	Mesh->ClearMeshSection(0);

	TArray<FVector> Vertices;
	TArray<int32> TrianglesInner;

	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> Colours;
	TArray<FProcMeshTangent> Tangents;  
	
	// Create Vertices
	for (int y = 0; y <= Resolution; y++)
	{
		for (int x = 0; x <= Resolution; x++)
		{
			float Height = 0.f;
			int Index = y * Resolution + x;	
			
			
			Vertices.Add(FVector(x * Scale, y * Scale, Height));
			UVs.Add(FVector2D((float)x / ChunkUVScale * Scale, (float)y / ChunkUVScale * Scale));                

		}
	}
	
	// Triangles
	for (int y = 0; y < Resolution; y++)
	{
		for (int x = 0; x < Resolution; x++)
		{
			int BottomLeft = y * (Resolution + 1) + x;
			int BottomRight = y * (Resolution + 1) + x + 1;
			int TopLeft = (y + 1) * (Resolution + 1) + x;
			int TopRight = (y + 1) * (Resolution + 1) + x + 1;

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

