// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Pickup.h"
#include "Net/UnrealNetwork.h"
#include "Characters/MLCharacter.h"
#include "Inventory/InventoryComponent.h"
#include "Engine/AssetManager.h"
#include "Components/BoxComponent.h"



// Sets default values
APickup::APickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	BoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>("BoxCollisionComponent");
	BoxCollisionComponent->SetupAttachment(RootComponent);
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	StaticMeshComponent->SetupAttachment(BoxCollisionComponent);
}

void APickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickup, ItemInfo);
}

void APickup::OnRep_ItemInfo()
{
	UAssetManager::GetStreamableManager().RequestAsyncLoad(ItemInfo.DataAsset.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this, [this]() {
		if (UInventoryDataAsset* dataAsset = ItemInfo.DataAsset.Get())
		{
			UAssetManager::GetStreamableManager().RequestAsyncLoad(dataAsset->StaticMesh.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this, [this, dataAsset]() {
				if (UStaticMesh* staticMesh = dataAsset->StaticMesh.Get())
				{
					StaticMeshComponent->SetStaticMesh(staticMesh);
				}
				}));
		}
		}));
}

void APickup::OnInteract(AMLCharacter* InteractingCharacter)
{
	UInventoryComponent* InventoryComponent = InteractingCharacter->GetInventoryComponent();
	InventoryComponent->Server_AddItemFromPickup(ItemInfo, this);
}

FText APickup::GetInteractionText() const
{
	return ItemInfo.DataAsset.LoadSynchronous()->ItemName;
}
