#include "Items/Item.h"

#include "ShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Curves/CurveVector.h"

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

	ZCurveTime = 0.7f;
	ItemInterpStartLocation = FVector(0.f);
	CameraTargetLocation = FVector(0.f);
	bInterping = false;

	ItemInterpX = 0.f;
	ItemInterpY = 0.f;
	MaterialIndex = 0;
	
	bCanChangeCustomDepth = true;

	GlowAmount = 150.f;
	FresnelExponent = 3.f;
	FresnelReflectFraction = 4.f;
	PulseCurveTime = 5.f;

	SlotIndex = 0;

	bCharacterInventoryFull = false;
	
}

void AItem::BeginPlay() {
	Super::BeginPlay();
	PickupWidgetComp->SetVisibility(false);
	
	SetActiveStars();
	
	// Hide widget
	AreaSphereComp->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlapBegin);
	AreaSphereComp->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	SetItemProperties(ItemState);

	InitializeCustomDepth();

	StartPulseTimer();
}

void AItem::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	
	ItemInterp(DeltaTime);
	UpdatePulse();
}

void AItem::PlayEquipSound(bool bForcePlaySound) {
	if(Character) {
		if(bForcePlaySound) {
			
		}
	}
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
		PickupWidgetComp->SetVisibility(false);
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
		ItemMeshComp->SetVisibility(true);
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
	case EItemState::EIS_EquipInterping:
		PickupWidgetComp->SetVisibility(false);
		
		ItemMeshComp->SetSimulatePhysics(false);
		ItemMeshComp->SetEnableGravity(false);
		ItemMeshComp->SetVisibility(true);
		ItemMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		AreaSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		AreaSphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		CollisionBoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;

	case EItemState::EIS_PickedUp:
		PickupWidgetComp->SetVisibility(false);
		
		ItemMeshComp->SetSimulatePhysics(false);
		ItemMeshComp->SetVisibility(false);
		ItemMeshComp->SetEnableGravity(false);
		ItemMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		AreaSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		AreaSphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		CollisionBoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
			
			
	}
}


void AItem::ResetPulseTimer() {
	StartPulseTimer();
}

void AItem::StartPulseTimer() {
	if(ItemState == EItemState::EIS_Pickup) {
		GetWorldTimerManager().SetTimer(PulseTimer, this, &AItem::ResetPulseTimer, PulseCurveTime);
	}
}

void AItem::SetItemState(EItemState State) {
	ItemState = State;
	SetItemProperties(ItemState);	
}

void AItem::StartItemCurve(AShooterCharacter* Shooter) {
	Character = Shooter;

	//Pick upSound
	/*if(PickupSound) {
		UGameplayStatics::PlaySound2D(this, PickupSound);
	}*/
	
	ItemInterpStartLocation = GetActorLocation();
	bInterping = true;
	SetItemState(EItemState::EIS_EquipInterping);
	GetWorldTimerManager().ClearTimer(PulseTimer);
	
	GetWorldTimerManager().SetTimer(
		ItemInterpTimer,
		this,
		&AItem::FinishInterping,
		ZCurveTime
	);

	bCanChangeCustomDepth = false;
}

void AItem::FinishInterping() {
	bInterping = false;
	if(Character) {
		Character->GetPickUpItem(this);
	}
	DisableGlowMaterial();
	bCanChangeCustomDepth = true;
	DisableCustomDepth();

}

void AItem::ItemInterp(float DeltaTime) {
	if(!bInterping) return;
	
	if(Character && ItemZCurve) {
		
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
		const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);
		FVector ItemLocation = ItemInterpStartLocation;
		const FVector CameraInterpLocation = Character->GetCameraInterpLocation();
		const FVector ItemToCamera { FVector(0.f, 0.f, (CameraInterpLocation - ItemLocation).Z) };
		const float DeltaZ = ItemToCamera.Size();

		const FVector CurrentLocation { GetActorLocation() };
		const float InterpXValue = FMath::FInterpTo(CurrentLocation.X, CameraInterpLocation.X, DeltaTime, 30.f);
		const float InterpYValue = FMath::FInterpTo(CurrentLocation.Y, CameraInterpLocation.Y, DeltaTime, 30.f);

		ItemLocation.X = InterpXValue;
		ItemLocation.Y = InterpYValue;

		ItemLocation.Z += CurveValue * DeltaZ; 
		SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);
		
	}
}

void AItem::EnableCustomDepth() {
	if(bCanChangeCustomDepth)
		ItemMeshComp->SetRenderCustomDepth(true);
}

void AItem::DisableCustomDepth() {
	if(bCanChangeCustomDepth)
		ItemMeshComp->SetRenderCustomDepth(false);
}

void AItem::InitializeCustomDepth() {
	DisableCustomDepth();
}

void AItem::OnConstruction(const FTransform& Transform) {

	//Path to the item rarity data table
	FString RarityTablePath("/Script/Engine.DataTable'/Game/_Game/DataTables/ItemRarityDataTable.ItemRarityDataTable'");
	UDataTable* RarityTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *RarityTablePath));
	if(RarityTableObject) {
		FItemRarityTable* RarityRow = nullptr;
		switch(ItemRarity) {
			case EItemRarity::EIR_Damaged:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Damaged"), TEXT(""));
				break;
			case EItemRarity::EIR_Common:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Common"), TEXT(""));
				break;
			case EItemRarity::EIR_UnCommon:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Uncommon"), TEXT(""));
				break;
			case EItemRarity::EIR_Rare:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Rare"), TEXT(""));
				break;
			case EItemRarity::EIR_Legendary:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Legendary"), TEXT(""));
				break;
		}

		if(RarityRow) {
			GlowColor = RarityRow->GlowColor;
			LightColor = RarityRow->LightColor;
			DarkColor = RarityRow->DarkColor;
			NumberOfStars = RarityRow->NumberOfStars;
			IconBackground = RarityRow->IconBackground;
			if(GetItemMesh()) {
				GetItemMesh()->SetCustomDepthStencilValue(RarityRow->CustomDepthStencil);
				GetItemMesh()->MarkRenderStateDirty();
			}
		}
	}
	if(MaterialInstance) {
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);
		DynamicMaterialInstance->SetVectorParameterValue("FresnelColor", GlowColor);
		ItemMeshComp->SetMaterial(MaterialIndex, DynamicMaterialInstance);
		EnableGlowMaterial();
	}
}

void AItem::EnableGlowMaterial() {
	if(DynamicMaterialInstance) {
		DynamicMaterialInstance->SetScalarParameterValue("GlowBlendAlpha", 0.f);
	}
}

void AItem::UpdatePulse() {
	float ElapsedTime{};
	FVector CurveValue{};
	switch(ItemState) {
		case EItemState::EIS_Pickup:
			if(PulseCurve) {
				ElapsedTime = GetWorldTimerManager().GetTimerElapsed(PulseTimer);
				CurveValue = PulseCurve->GetVectorValue(ElapsedTime);
			}
			break;
		case EItemState::EIS_EquipInterping:
			if(InterpPulseCurve) {
				ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
				CurveValue = InterpPulseCurve->GetVectorValue(ElapsedTime);
			}
			break;
	}
	if(DynamicMaterialInstance) {
		DynamicMaterialInstance->SetScalarParameterValue("GlowAmount", CurveValue.X * GlowAmount);
		DynamicMaterialInstance->SetScalarParameterValue("FresnelExponent", CurveValue.Y * FresnelExponent);
		DynamicMaterialInstance->SetScalarParameterValue("FresnelReflectFraction", CurveValue.Z * FresnelReflectFraction);
	}
}

void AItem::DisableGlowMaterial() {
	if(DynamicMaterialInstance) {
		DynamicMaterialInstance->SetScalarParameterValue("GlowBlendAlpha", 1.f);
	}
}


