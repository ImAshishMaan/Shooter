#include "Items/Ammo.h"

#include "ShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AAmmo::AAmmo() {

	AmmoMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMeshComp"));
	SetRootComponent(AmmoMeshComp);

	AmmoCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AmmoCollisionSphere"));
	AmmoCollisionSphere->SetupAttachment(GetRootComponent());
	AmmoCollisionSphere->SetSphereRadius(50.f);

	CollisionBoxComp->SetupAttachment(GetRootComponent());
	PickupWidgetComp->SetupAttachment(GetRootComponent());
	AreaSphereComp->SetupAttachment(GetRootComponent());
	
}

void AAmmo::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	
}

void AAmmo::BeginPlay() {
	Super::BeginPlay();

	AmmoCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAmmo::AmmoSphereOverlap);
	
}

void AAmmo::SetItemProperties(EItemState State) {
	Super::SetItemProperties(State);

	switch(State) {
	case EItemState::EIS_Pickup:
		AmmoMeshComp->SetSimulatePhysics(false);
		AmmoMeshComp->SetVisibility(true);
		AmmoMeshComp->SetEnableGravity(false);
		AmmoMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		break;
	case EItemState::EIS_Equipped:
		AmmoMeshComp->SetSimulatePhysics(false);
		AmmoMeshComp->SetVisibility(true);
		AmmoMeshComp->SetEnableGravity(false);
		AmmoMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		break;
	case EItemState::EIS_Falling:
		AmmoMeshComp->SetSimulatePhysics(true);
		AmmoMeshComp->SetEnableGravity(true);
		AmmoMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AmmoMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		AmmoMeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		
		break;
	case EItemState::EIS_EquipInterping:
		AmmoMeshComp->SetSimulatePhysics(false);
		AmmoMeshComp->SetEnableGravity(false);
		AmmoMeshComp->SetVisibility(true);
		AmmoMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		break;
	}
}

void AAmmo::EnableCustomDepth() {
	AmmoMeshComp->SetRenderCustomDepth(true);
}

void AAmmo::DisableCustomDepth() {
	AmmoMeshComp->SetRenderCustomDepth(false);
}

void AAmmo::AmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {

	if(OtherActor) {
		auto OverlappedCharacter = Cast<AShooterCharacter>(OtherActor);
		if(OverlappedCharacter) {
			StartItemCurve(OverlappedCharacter);
			AmmoCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}
