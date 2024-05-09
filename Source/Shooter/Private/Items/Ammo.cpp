#include "Items/Ammo.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AAmmo::AAmmo() {

	AmmoMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMeshComp"));
	SetRootComponent(AmmoMeshComp);

	CollisionBoxComp->SetupAttachment(GetRootComponent());
	PickupWidgetComp->SetupAttachment(GetRootComponent());
	AreaSphereComp->SetupAttachment(GetRootComponent());
	
}

void AAmmo::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	
}

void AAmmo::BeginPlay() {
	Super::BeginPlay();
	
}
