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
	
}

void AItem::BeginPlay() {
	Super::BeginPlay();
	PickupWidgetComp->SetVisibility(false);
	
	SetActiveStars();
	
	// Hide widget
	AreaSphereComp->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlapBegin);
	AreaSphereComp->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
	
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

void AItem::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}
