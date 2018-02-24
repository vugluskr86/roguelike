// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include <random>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "DungeonMapActor.generated.h"

class UInstancedStaticMeshComponent;

UENUM(BlueprintType)	
enum class ETileType : uint8
{
   TE_Unused UMETA(DisplayName="Unused"),
   TE_DirtWall UMETA(DisplayName="DirtWall"),
	TE_DirtFloor UMETA(DisplayName="DirtFloor"),
   TE_Corridor	UMETA(DisplayName="Corridor"),
   TE_Door UMETA(DisplayName="Door"),
   TE_UpStairs UMETA(DisplayName="UpStairs"),
   TE_DownStairs UMETA(DisplayName="DownStairs")
};

USTRUCT(BlueprintType)
struct FTileMesh
{
   GENERATED_BODY()

   FTileMesh() : StaticMesh(nullptr), Material(nullptr) {}

   UPROPERTY(EditAnywhere) UStaticMesh* StaticMesh;
   UPROPERTY(EditAnywhere) UMaterial* Material;
};

USTRUCT(BlueprintType)
struct FTilesDefenition
{
   GENERATED_BODY()
   UPROPERTY(EditAnywhere) FTileMesh DirtWall;
   UPROPERTY(EditAnywhere) FTileMesh DirtFloor;
   UPROPERTY(EditAnywhere) FTileMesh Corridor;
   UPROPERTY(EditAnywhere) FTileMesh Door;
   UPROPERTY(EditAnywhere) FTileMesh UpStairs;
   UPROPERTY(EditAnywhere) FTileMesh DownStairs;
};

using RngT = std::mt19937;

enum class Direction
{
   North, South, East, West,
};

UCLASS()
class ROGUELIKE_API ADungeonMapActor : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ADungeonMapActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

   // Generator properties
   UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = MapProperties) int32 Seed;
   UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = MapProperties) int32 XSize;
   UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = MapProperties) int32 YSize;
   UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = MapProperties) int32 MaxFeatures;
   UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = MapProperties) int32 ChanceRoom;
   UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = MapProperties) int32 ChanceCorridor;
	UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = MapProperties) FTilesDefenition MeshDefenitions;
   
   UPROPERTY() TArray<UInstancedStaticMeshComponent*> InstancedStaticMeshComponents;

   // Methods
   UFUNCTION(BlueprintCallable, Category = MapMethods) void SetCell(int32 x, int32 y, ETileType celltype);
   UFUNCTION(BlueprintCallable, Category = MapMethods) ETileType GetCell(int32 x, int32 y) const;
   UFUNCTION(BlueprintCallable, Category = MapMethods) void SetCells(int32 xStart, int32 yStart, int32 xEnd, int32 yEnd, ETileType cellType);
   UFUNCTION(BlueprintCallable, Category = MapMethods) bool IsXInBounds(int32 x) const;
   UFUNCTION(BlueprintCallable, Category = MapMethods) bool IsYInBounds(int32 y) const;
   UFUNCTION(BlueprintCallable, Category = MapMethods) bool IsAreaUnused(int32 xStart, int32 yStart, int32 xEnd, int32 yEnd);
   UFUNCTION(BlueprintCallable, Category = MapMethods) bool IsAdjacent(int32 x, int32 y, ETileType tile);
   UFUNCTION(BlueprintCallable, Category = MapMethods) void Build();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

private:
   std::vector<ETileType> Data_;
   RngT rnd_;

   void Generate();
   int32 GetRandomInt(int32 min, int32 max);
   Direction GetRandomDirection();
   bool MakeCorridor(int32 x, int32 y, int32 maxLength, Direction direction);
   bool MakeRoom(int32 x, int32 y, int32 xMaxLength, int32 yMaxLength, Direction direction);
   bool MakeFeature(int32 x, int32 y, int32 xmod, int32 ymod, Direction direction);
   bool MakeFeature();
   bool MakeStairs(ETileType tile);
   bool MakeDungeon();

   UInstancedStaticMeshComponent* BuildInstancedMesh(ETileType tile);
};
