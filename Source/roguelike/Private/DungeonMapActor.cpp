// Fill out your copyright notice in the Description page of Project Settings.

#include "DungeonMapActor.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
ADungeonMapActor::ADungeonMapActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

   Seed = std::random_device()();
   XSize = 80;
   YSize = 25;
   MaxFeatures = 100;
   ChanceRoom = 75;
   ChanceCorridor = 25;

   InstancedStaticMeshComponents.Empty();
}

// Called when the game starts or when spawned
void ADungeonMapActor::BeginPlay()
{
	Super::BeginPlay();
	
   Build();
}

// Called every frame
void ADungeonMapActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

#if WITH_EDITOR
void ADungeonMapActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
   Build();
   Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR


void ADungeonMapActor::SetCell(int32 x, int32 y, ETileType celltype) 
{
   Data_[x + XSize * y] = celltype;
}

ETileType ADungeonMapActor::GetCell(int32 x, int32 y) const
{
   return Data_[x + XSize * y];
}

void ADungeonMapActor::SetCells(int32 xStart, int32 yStart, int32 xEnd, int32 yEnd, ETileType cellType) 
{
   for (auto y = yStart; y != yEnd + 1; ++y)
      for (auto x = xStart; x != xEnd + 1; ++x)
         SetCell(x, y, cellType);
}

bool ADungeonMapActor::IsXInBounds(int32 x) const 
{
   return x >= 0 && x < XSize;
}

bool ADungeonMapActor::IsYInBounds(int32 y) const
{
   return y >= 0 && y < YSize;
}

bool ADungeonMapActor::IsAreaUnused(int32 xStart, int32 yStart, int32 xEnd, int32 yEnd) 
{
   for (auto y = yStart; y != yEnd + 1; ++y)
      for (auto x = xStart; x != xEnd + 1; ++x)
         if (GetCell(x, y) != ETileType::TE_Unused)
            return false;

   return true;
}

bool ADungeonMapActor::IsAdjacent(int32 x, int32 y, ETileType tile) 
{
   return
      GetCell(x - 1, y) == tile || GetCell(x + 1, y) == tile ||
      GetCell(x, y - 1) == tile || GetCell(x, y + 1) == tile;
}

void ADungeonMapActor::SetCellMeta(int32 x, int32 y, FTileMeta celltype)
{
   Meta_[x + XSize * y] = celltype;
}

FTileMeta ADungeonMapActor::GetCellMeta(int32 x, int32 y) const
{
   return Meta_[x + XSize * y];
}

void ADungeonMapActor::Build() 
{
   Generate();

   for (int32 i = 0; i < InstancedStaticMeshComponents.Num(); ++i) {
      if (InstancedStaticMeshComponents[i]) {
         InstancedStaticMeshComponents[i]->ClearInstances();
      }
   }
   InstancedStaticMeshComponents.Empty();

   InstancedStaticMeshComponents.Add(BuildInstancedMesh(ETileType::TE_DirtWall));
   InstancedStaticMeshComponents.Add(BuildInstancedMesh(ETileType::TE_DirtFloor));
   InstancedStaticMeshComponents.Add(BuildInstancedMesh(ETileType::TE_Corridor));
   InstancedStaticMeshComponents.Add(BuildInstancedMesh(ETileType::TE_Door));
   InstancedStaticMeshComponents.Add(BuildInstancedMesh(ETileType::TE_UpStairs));
   InstancedStaticMeshComponents.Add(BuildInstancedMesh(ETileType::TE_DownStairs));

   const int32 DirtWall_Index   = 0;
   const int32 DirtFloor_Index  = 1;
   const int32 Corridor_Index   = 2;
   const int32 Door_Index       = 3;
   const int32 UpStairs_Index   = 4;
   const int32 DownStairs_Index = 5;

   FTransform local = GetTransform();

   auto scale = 10.0f;
   FVector ScaleVector(scale, scale, scale);
   FRotator Rotator(0.0f,0.0f,0.0f);
     
   for (auto y = 0; y != YSize; y++) {
      for (auto x = 0; x != XSize; x++) {
         switch (GetCell(x, y)) {
            case ETileType::TE_DirtFloor: {
               FVector size = MeshDefenitions.DirtFloor.StaticMesh->GetBoundingBox().GetSize();
               FVector SpawnPosVector = local.GetLocation() + FVector(x * size.X * scale, y * size.Y * scale, 0.0f);
               FTransform Transform(Rotator, SpawnPosVector, ScaleVector);
               InstancedStaticMeshComponents[DirtFloor_Index]->AddInstance(Transform);
               break;
            }
            case ETileType::TE_Corridor: {
               FVector size = MeshDefenitions.Corridor.StaticMesh->GetBoundingBox().GetSize();
               FVector SpawnPosVector = local.GetLocation() + FVector(x * size.X * scale, y * size.Y * scale, 0.0f);
               FTransform Transform(Rotator, SpawnPosVector, ScaleVector);
               InstancedStaticMeshComponents[Corridor_Index]->AddInstance(Transform);
               break;
            }
            case ETileType::TE_DirtWall: {
               FVector size = MeshDefenitions.DirtWall.StaticMesh->GetBoundingBox().GetSize();
               FVector sizeF = MeshDefenitions.DirtFloor.StaticMesh->GetBoundingBox().GetSize();
               FVector SpawnPosVector = local.GetLocation() + FVector(x * sizeF.X * scale, y * size.Y * scale, 0.0f);
               FTransform Transform(Rotator, SpawnPosVector, ScaleVector);
               InstancedStaticMeshComponents[DirtWall_Index]->AddInstance(Transform);
               break;
            }
         }
      }
   }
}

UInstancedStaticMeshComponent* ADungeonMapActor::BuildInstancedMesh(ETileType tile)
{
   UInstancedStaticMeshComponent* Proxy = NewObject<UInstancedStaticMeshComponent>(this);
   Proxy->RegisterComponent();
   Proxy->SetFlags(RF_Transactional);

   switch (tile)
   {
   case ETileType::TE_Unused:
      break;
   case ETileType::TE_DirtWall:
      if(MeshDefenitions.DirtWall.StaticMesh) Proxy->SetStaticMesh(MeshDefenitions.DirtWall.StaticMesh);
      if(MeshDefenitions.DirtWall.Material) Proxy->SetMaterial(0, UMaterialInstanceDynamic::Create(MeshDefenitions.DirtWall.Material, this));
      break;
   case ETileType::TE_DirtFloor:
      if (MeshDefenitions.DirtFloor.StaticMesh) Proxy->SetStaticMesh(MeshDefenitions.DirtFloor.StaticMesh);
      if (MeshDefenitions.DirtFloor.Material) Proxy->SetMaterial(0, UMaterialInstanceDynamic::Create(MeshDefenitions.DirtFloor.Material, this));
      break;
   case ETileType::TE_Corridor:
      if (MeshDefenitions.Corridor.StaticMesh) Proxy->SetStaticMesh(MeshDefenitions.Corridor.StaticMesh);
      if (MeshDefenitions.Corridor.Material) Proxy->SetMaterial(0, UMaterialInstanceDynamic::Create(MeshDefenitions.Corridor.Material, this));
      break;
   case ETileType::TE_Door:
      if (MeshDefenitions.Door.StaticMesh) Proxy->SetStaticMesh(MeshDefenitions.Door.StaticMesh);
      if (MeshDefenitions.Door.Material) Proxy->SetMaterial(0, UMaterialInstanceDynamic::Create(MeshDefenitions.Door.Material, this));
      break;
   case ETileType::TE_UpStairs:
      if (MeshDefenitions.UpStairs.StaticMesh) Proxy->SetStaticMesh(MeshDefenitions.UpStairs.StaticMesh);
      if (MeshDefenitions.UpStairs.Material) Proxy->SetMaterial(0, UMaterialInstanceDynamic::Create(MeshDefenitions.UpStairs.Material, this));
      break;
   case ETileType::TE_DownStairs:
      if (MeshDefenitions.DownStairs.StaticMesh) Proxy->SetStaticMesh(MeshDefenitions.DownStairs.StaticMesh);
      if (MeshDefenitions.DownStairs.Material) Proxy->SetMaterial(0, UMaterialInstanceDynamic::Create(MeshDefenitions.DownStairs.Material, this));
      break;
   }
   
   return Proxy;
}

void ADungeonMapActor::Generate() 
{
   Data_.empty();
   Data_ = std::vector<ETileType>(XSize * YSize, ETileType::TE_Unused);
   
   Meta_.empty();
   Meta_.reserve(XSize * YSize);
   
   rnd_.seed(Seed);
   MakeDungeon();
}

int32 ADungeonMapActor::GetRandomInt(int32 min, int32 max)
{
   return std::uniform_int_distribution<int32>(min, max)(rnd_);
}

EDirection ADungeonMapActor::GetRandomDirection()
{
   return EDirection(std::uniform_int_distribution<int>(0, 3)(rnd_));
}

bool ADungeonMapActor::MakeCorridor(int32 x, int32 y, int32 maxLength, EDirection direction) 
{
   auto length = GetRandomInt(2, maxLength);

   auto xStart = x;
   auto yStart = y;

   auto xEnd = x;
   auto yEnd = y;

   if (direction == EDirection::DE_North)
      yStart = y - length;
   else if (direction == EDirection::DE_East)
      xEnd = x + length;
   else if (direction == EDirection::DE_South)
      yEnd = y + length;
   else if (direction == EDirection::DE_West)
      xStart = x - length;

   if (!IsXInBounds(xStart) || !IsXInBounds(xEnd) || !IsYInBounds(yStart) || !IsYInBounds(yEnd))
      return false;

   if (!IsAreaUnused(xStart, yStart, xEnd, yEnd))
      return false;

   SetCells(xStart, yStart, xEnd, yEnd, ETileType::TE_Corridor);

   return true;
}

bool ADungeonMapActor::MakeRoom(int32 x, int32 y, int32 xMaxLength, int32 yMaxLength, EDirection direction) 
{
   // Minimum room size of 4x4 tiles (2x2 for walking on, the rest is walls)
   auto xLength = GetRandomInt(4, xMaxLength);
   auto yLength = GetRandomInt(4, yMaxLength);

   auto xStart = x;
   auto yStart = y;

   auto xEnd = x;
   auto yEnd = y;

   if (direction == EDirection::DE_North)
   {
      yStart = y - yLength;
      xStart = x - xLength / 2;
      xEnd = x + (xLength + 1) / 2;
   }
   else if (direction == EDirection::DE_East)
   {
      yStart = y - yLength / 2;
      yEnd = y + (yLength + 1) / 2;
      xEnd = x + xLength;
   }
   else if (direction == EDirection::DE_South)
   {
      yEnd = y + yLength;
      xStart = x - xLength / 2;
      xEnd = x + (xLength + 1) / 2;
   }
   else if (direction == EDirection::DE_West)
   {
      yStart = y - yLength / 2;
      yEnd = y + (yLength + 1) / 2;
      xStart = x - xLength;
   }

   if (!IsXInBounds(xStart) || !IsXInBounds(xEnd) || !IsYInBounds(yStart) || !IsYInBounds(yEnd))
      return false;

   if (!IsAreaUnused(xStart, yStart, xEnd, yEnd))
      return false;

   //SetCellMeta()
   
   SetCells(xStart, yStart, xEnd, yEnd, ETileType::TE_DirtWall);
   SetCells(xStart + 1, yStart + 1, xEnd - 1, yEnd - 1, ETileType::TE_DirtFloor);

   return true;
}

bool ADungeonMapActor::MakeFeature(int32 x, int32 y, int32 xmod, int32 ymod, EDirection direction)
{
   // Choose what to build
   auto chance = GetRandomInt(0, 100);

   if (chance <= ChanceRoom)
   {
      if (MakeRoom(x + xmod, y + ymod, 8, 6, direction))
      {
         SetCell(x, y, ETileType::TE_Door);

         // Remove wall next to the door.
         SetCell(x + xmod, y + ymod, ETileType::TE_DirtFloor);

         return true;
      }

      return false;
   }
   else
   {
      if (MakeCorridor(x + xmod, y + ymod, 6, direction))
      {
         SetCell(x, y, ETileType::TE_Door);

         return true;
      }

      return false;
   }
}

bool ADungeonMapActor::MakeFeature()
{
   auto tries = 0;
   auto maxTries = 1000;

   for (; tries != maxTries; ++tries)
   {
      // Pick a random wall or corridor tile.
      // Make sure it has no adjacent doors (looks weird to have doors next to each other).
      // Find a direction from which it's reachable.
      // Attempt to make a feature (room or corridor) starting at this point.

      int x = GetRandomInt(1, XSize - 2);
      int y = GetRandomInt(1, YSize - 2);

      if (GetCell(x, y) != ETileType::TE_DirtWall && GetCell(x, y) != ETileType::TE_Corridor)
         continue;

      if (IsAdjacent(x, y, ETileType::TE_Door))
         continue;

      if (GetCell(x, y + 1) == ETileType::TE_DirtFloor || GetCell(x, y + 1) == ETileType::TE_Corridor)
      {
         if (MakeFeature(x, y, 0, -1, EDirection::DE_North))
            return true;
      }
      else if (GetCell(x - 1, y) == ETileType::TE_DirtFloor || GetCell(x - 1, y) == ETileType::TE_Corridor)
      {
         if (MakeFeature(x, y, 1, 0, EDirection::DE_East))
            return true;
      }
      else if (GetCell(x, y - 1) == ETileType::TE_DirtFloor || GetCell(x, y - 1) == ETileType::TE_Corridor)
      {
         if (MakeFeature(x, y, 0, 1, EDirection::DE_South))
            return true;
      }
      else if (GetCell(x + 1, y) == ETileType::TE_DirtFloor || GetCell(x + 1, y) == ETileType::TE_Corridor)
      {
         if (MakeFeature(x, y, -1, 0, EDirection::DE_West))
            return true;
      }
   }

   return false;
}

bool ADungeonMapActor::MakeStairs(ETileType tile) 
{
   auto tries = 0;
   auto maxTries = 10000;

   for (; tries != maxTries; ++tries)
   {
      int x = GetRandomInt(1, XSize - 2);
      int y = GetRandomInt(1, YSize - 2);

      if (!IsAdjacent(x, y, ETileType::TE_DirtFloor) && !IsAdjacent(x, y, ETileType::TE_Corridor))
         continue;

      if (IsAdjacent(x, y, ETileType::TE_Door))
         continue;

      SetCell(x, y, tile);

      return true;
   }

   return false;
}

bool ADungeonMapActor::MakeDungeon() 
{
   // Make one room in the middle to start things off.
   MakeRoom(XSize / 2, YSize / 2, 8, 6, GetRandomDirection());

   for (auto features = 1; features != MaxFeatures; ++features)
   {
      if (!MakeFeature())
      {
        // std::cout << "Unable to place more features (placed " << features << ")." << std::endl;
         break;
      }
   }

   if (!MakeStairs(ETileType::TE_UpStairs)) {
      // std::cout << "Unable to place up stairs." << std::endl;
   }
      

   if (!MakeStairs(ETileType::TE_DownStairs)) {
      // std::cout << "Unable to place down stairs." << std::endl;
   }

   return true;
}