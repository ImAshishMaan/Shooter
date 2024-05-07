#include "Items/Item.h"

#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"

AItem::AItem() {
	PrimaryActorTick.bCanEverTick = true;

	ItemMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMeshComp"));
	SetRootComponent(ItemMeshComp);
	
	CollisionBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBoxComp"));
	CollisionBoxComp->SetupAttachment(ItemMeshComp);
	CollisionBoxComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBoxComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidgetComp"));
	PickupWidgetComp->SetupAttachment(RootComponent);
	
}

void AItem::BeginPlay() {
	Super::BeginPlay();

	// Hide widget
	PickupWidgetComp->SetVisibility(false);
	
}

void AItem::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}
