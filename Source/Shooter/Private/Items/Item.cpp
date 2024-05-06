#include "Items/Item.h"

#include "Components/BoxComponent.h"

AItem::AItem() {
	PrimaryActorTick.bCanEverTick = true;

	ItemMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMeshComp"));
	SetRootComponent(ItemMeshComp);
	
	CollisionBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBoxComp"));
	CollisionBoxComp->SetupAttachment(ItemMeshComp);

	
}

void AItem::BeginPlay() {
	Super::BeginPlay();
}

void AItem::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}
