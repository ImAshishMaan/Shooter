#include "Items/Item.h"

#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"

AItem::AItem() {
	PrimaryActorTick.bCanEverTick = true;

	ItemMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMeshComp"));
	SetRootComponent(ItemMeshComp);
	
	CollisionBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBoxComp"));
	CollisionBoxComp->SetupAttachment(ItemMeshComp);

	PickupWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidgetComp"));
	PickupWidgetComp->SetupAttachment(Get);
	
}

void AItem::BeginPlay() {
	Super::BeginPlay();
}

void AItem::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}
