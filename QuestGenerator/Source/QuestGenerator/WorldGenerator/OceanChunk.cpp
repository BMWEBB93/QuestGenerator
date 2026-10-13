// Fill out your copyright notice in the Description page of Project Settings.


#include "OceanChunk.h"
#include "KismetProceduralMeshLibrary.h"


// Sets default values
AOceanChunk::AOceanChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
}

// Called when the game starts or when spawned
void AOceanChunk::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOceanChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOceanChunk::GenerateChunk(UMaterialInterface* OceanMaterial)
{
	if (Mesh)
	{
		Mesh->ClearMeshSection(0);
	}
	

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> Colours;
	TArray<FProcMeshTangent> Tangents;

    

	// Vertices
	for (int y = 0; y <= Resolution; y++)
	{
		for (int x = 0; x <= Resolution; x++)
		{
			Vertices.Add(FVector(x * Scale, y * Scale, OceanLevel));

			UVs.Add(FVector2D((float)x / UVScale * Scale, (float)y / UVScale * Scale));            
			int32 Index = x + y * (Resolution + 1);            		
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

			Triangles.Add(BottomLeft);
			Triangles.Add(TopLeft);
			Triangles.Add(TopRight);

			Triangles.Add(TopRight);
			Triangles.Add(BottomRight);
			Triangles.Add(BottomLeft);
            
		}
	}

   

	// Create mesh
	Mesh->CreateMeshSection_LinearColor(
		0,          // section index
		Vertices,
		Triangles,
		Normals,
		UVs,
		Colours,
		Tangents,
		false        // collision
	);

	Mesh->SetMaterial(0, OceanMaterial);
}

