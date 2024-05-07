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
	
}

void AItem::BeginPlay() {
	Super::BeginPlay();
	PickupWidgetComp->SetVisibility(false);

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

void AItem::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}
