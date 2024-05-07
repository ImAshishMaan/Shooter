#include "Items/Item.h"

#include "ShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
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

	AreaSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphereComp"));
	AreaSphereComp->SetupAttachment(RootComponent);

	ItemName = FString("Default");
	ItemCount = 0;
	ItemRarity = EItemRarity::EIR_Common;
	ItemState = EItemState::EIS_Pickup;
	
}

void AItem::BeginPlay() {
	Super::BeginPlay();
	PickupWidgetComp->SetVisibility(false);
	
	SetActiveStars();
	
	// Hide widget
	AreaSphereComp->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlapBegin);
	AreaSphereComp->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	SetItemProperties(ItemState);
}

void AItem::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {

	if(OtherActor) {
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if(ShooterCharacter) {
			ShooterCharacter->IncrementOverlappedItemCount(1);
		}
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	
	if(OtherActor) {
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if(ShooterCharacter) {
			ShooterCharacter->IncrementOverlappedItemCount(-1);
		}
	}
}

void AItem::SetActiveStars() {
	for(int32 i = 0; i <= 5; i++) {
		ActiveStars.Add(false);
	}

	switch(ItemRarity) {
	case EItemRarity::EIR_Damaged:
		ActiveStars[1] = true;
		break;
	case EItemRarity::EIR_Common:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		break;
	case EItemRarity::EIR_UnCommon:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		break;
	case EItemRarity::EIR_Rare:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		break;
	case EItemRarity::EIR_Legendary:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		ActiveStars[5] = true;
		break;
		
		
	}
}

void AItem::SetItemProperties(EItemState State) {
	switch(State) {
	case EItemState::EIS_Pickup:
		ItemMeshComp->SetSimulatePhysics(false);
		ItemMeshComp->SetVisibility(true);
		ItemMeshComp->SetEnableGravity(false);
		ItemMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		AreaSphereComp->SetCollisionResponseToAllChannels(ECR_Overlap);
		AreaSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		CollisionBoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBoxComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		CollisionBoxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EItemState::EIS_Equipped:
		ItemMeshComp->SetSimulatePhysics(false);
		ItemMeshComp->SetVisibility(true);
		ItemMeshComp->SetEnableGravity(false);
		ItemMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		AreaSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		AreaSphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		CollisionBoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Falling:
		ItemMeshComp->SetSimulatePhysics(true);
		ItemMeshComp->SetEnableGravity(true);
		ItemMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		ItemMeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		
		
		AreaSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		AreaSphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		CollisionBoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}

void AItem::SetItemState(EItemState State) {
	ItemState = State;
	SetItemProperties(ItemState);	
}

void AItem::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}
