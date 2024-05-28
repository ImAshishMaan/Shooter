#include "Shooter/Public/ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Items/Ammo.h"
#include "Items/Item.h"
#include "Items/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Shooter/AmmoType.h"
#include "Sound/SoundCue.h"

AShooterCharacter::AShooterCharacter() {
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.0f;
	CameraBoom->bUsePawnControlRotation = true; // Rotate arm based on controller
	CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 70.0f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Don't rotate mesh when the controller rotates. Let the controller only affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;

	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;
	MuzzleFlashSocket = "BarrelSocket";
	bAiming = false;
	
	CameraDefaultFOV = 0.0f;
	CameraZoomedFOV = 35.0f;
	ZoomInterpSpeed = 20.0f;
	CameraCurrentFOV = 0.0f;
	
	HipTurnRate = 90.0f;
	HipLookUpRate = 90.0f;
	AimingTurnRate = 20.0f;
	AimingLookUpRate = 20.0f;

	MouseHipTurnRate = 1.0f;
	MouseHipLookUpRate = 1.0f;
	MouseAimingTurnRate = 0.2f;
	MouseAimingLookUpRate = 0.2f;

	AutomaticFireRate = 0.1f;
	bShouldFire = true;
	bFireButtonPressed = false;

	bShouldTraceForItems = false;

	CameraInterpDistance = 250.0f;
	CameraInterpElevation = 65.0f;

	Starting9mmAmmo = 85;
	StartingARAmmo = 120;

	CombatState = ECombatState::ECS_Unoccupied;

}

void AShooterCharacter::BeginPlay() {
	Super::BeginPlay();
	
	if(FollowCamera) {
		CameraDefaultFOV = FollowCamera->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
	EquipWeapon(SpawnDefaultWeapon());
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial(); 
	
	InitializeAmmoMap();
}

void AShooterCharacter::TraceForItems() {
	if(bShouldTraceForItems) {
		FHitResult ItemTraceRestult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceRestult, HitLocation);
		if(ItemTraceRestult.bBlockingHit) {
			TraceHitItem = Cast<AItem>(ItemTraceRestult.GetActor());
			if(TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping) {
				TraceHitItem = nullptr;
			}
			if(TraceHitItem && TraceHitItem->GetPickUpWidget()) {
				TraceHitItem->GetPickUpWidget()->SetVisibility(true);
				TraceHitItem->EnableCustomDepth();
			}
			if(TraceHitItemLastFrame) {
				if(TraceHitItem != TraceHitItemLastFrame) {
					TraceHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
					TraceHitItemLastFrame->DisableCustomDepth();
				}
			}
			TraceHitItemLastFrame = TraceHitItem;
		}
	} else if(TraceHitItemLastFrame) {
		TraceHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
		TraceHitItemLastFrame->DisableCustomDepth();
	}
}

void AShooterCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	CameraZoomAim(DeltaTime);

	SetLookRates();
	TraceForItems();
}

void AShooterCharacter::SetLookRates() {
	if(bAiming) {
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}else {
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CameraZoomAim(float DeltaTime) {
	if(bAiming) {
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	} else {
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	FollowCamera->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::MoveForward(float Value) {
	if(Controller && Value != 0.0f) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}
void AShooterCharacter::MoveRight(float Value) {
	if(Controller && Value != 0.0f) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::TurnAtRate(float Rate) {
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}
void AShooterCharacter::LookUpAtRate(float Rate) {
	AddControllerPitchInput(Rate *  BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::FireWeapon() {
	if(!EquippedWeapon) return;
	if(CombatState != ECombatState::ECS_Unoccupied) return;
	if(WeaponHasAmmo()) {
		PlayFireSound();
		SendBullet();
		PlayGunFireMontage();
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();
	}
}

void AShooterCharacter::SendBullet() {
	const FTransform SocketTransform = EquippedWeapon->GetItemMesh()->GetSocketTransform(MuzzleFlashSocket);
	
	FVector BeamEndPoint;
	bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEndPoint);
	if(bBeamEnd) {
		if(ImpactEffect) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, BeamEndPoint);
		}

		if(MuzzleFlashEffect) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlashEffect, SocketTransform);
		}
	
		if(BeamParticleEffect) {
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticleEffect, SocketTransform);
			if(Beam) {
				Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
			}
		}
	}
}

void AShooterCharacter::PlayFireSound() {
	if(FireSound) {
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AShooterCharacter::PlayGunFireMontage() {
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(HipFireMontage && AnimInstance) {
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection("StartFire", HipFireMontage);
	}
}


bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation) {

	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);

	if(bCrosshairHit) {
		OutBeamLocation = CrosshairHitResult.Location;
	}else {
		//OutBeamLocation = MuzzleSocketLocation;
	}

	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart = MuzzleSocketLocation;
	const FVector StartToEnd = OutBeamLocation - MuzzleSocketLocation;
	const FVector WeaponTraceEnd = MuzzleSocketLocation + StartToEnd * 1.25f;
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
	if(WeaponTraceHit.bBlockingHit) {
		WeaponTraceHit.Location = OutBeamLocation;
		return true;
	}
	return false;
}

void AShooterCharacter::AimButtonPressed() {
	bAiming = true;
}
void AShooterCharacter::AimButtonReleased() {
	bAiming = false;
}

void AShooterCharacter::FireButtonPressed() {
	bFireButtonPressed = true;
	FireWeapon();
}
void AShooterCharacter::FireButtonReleased() {
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer() {
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
}
void AShooterCharacter::AutoFireReset() {
	CombatState = ECombatState::ECS_Unoccupied;
	if(WeaponHasAmmo()) {
		if(bFireButtonPressed) {
			FireWeapon();
		}
	}else {
		ReloadWeapon();
	}
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate); 
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput); 
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAction("FKey", IE_Pressed, this, &AShooterCharacter::FKeyPressed);
	PlayerInputComponent->BindAction("1Key", IE_Pressed, this, &AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("2Key", IE_Pressed, this, &AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("3Key", IE_Pressed, this, &AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("4Key", IE_Pressed, this, &AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction("5Key", IE_Pressed, this, &AShooterCharacter::FiveKeyPressed);
	
}

void AShooterCharacter::Turn(float Value) {
	if(bAiming) {
		AddControllerYawInput(Value * AimingTurnRate);
	}else {
		AddControllerYawInput(Value * BaseTurnRate);
	}
}

void AShooterCharacter::LookUp(float Value) {
	if(bAiming) {
		AddControllerPitchInput(Value * AimingLookUpRate);
	}else {
		AddControllerPitchInput(Value * BaseLookUpRate);
	}
}


bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation) {
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);
	CrosshairLocation.Y -= 50.0f;

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
			CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if(bScreenToWorld) {
		FVector Start = CrosshairWorldPosition;
		FVector End = Start + CrosshairWorldDirection * 50'0000.f;
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		if(OutHitResult.bBlockingHit) {
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}

	return false;
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon() {
	if(DefaultWeaponClass) {
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}
	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip) {
	if(WeaponToEquip) {
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if(HandSocket) {
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}

		if(EquippedWeapon == nullptr) {
			// -1 = no equipped weapon
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
		} else {
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
		}
		
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}else {
		UE_LOG(LogTemp, Warning, TEXT("Weapon to equip is nullptr!"));		
	}
}

void AShooterCharacter::DropWeapon() {
	if(EquippedWeapon) {
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
		
		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectButtonPressed() {
	if(CombatState != ECombatState::ECS_Unoccupied) return;
	
	if(TraceHitItem) {
		TraceHitItem->StartItemCurve(this);
		TraceHitItem = nullptr;
	}
}
void AShooterCharacter::SelectButtonReleased() {
	
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap) {

	if(Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex()) {
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
		WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	}
	
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AShooterCharacter::InitializeAmmoMap() {
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo() {
	if(!EquippedWeapon) return false;
	
	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::ReloadButtonPressed() {
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon() {
	if(CombatState != ECombatState::ECS_Unoccupied) return;
	if(EquippedWeapon == nullptr) return;

	if(CarryingAmmo()) {
		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if(AnimInstance && ReloadAnimMontage) {
			AnimInstance->Montage_Play(ReloadAnimMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection(), ReloadAnimMontage);
		}
	}
	
}

bool AShooterCharacter::CarryingAmmo() {
	if(EquippedWeapon == nullptr) return false;
	
	auto AmmoType = EquippedWeapon->GetAmmoType();
	if(AmmoMap.Contains(AmmoType)) {
		return AmmoMap[AmmoType] > 0;
	}
	return false;
}


void AShooterCharacter::FinishReloading() {
	CombatState = ECombatState::ECS_Unoccupied;
	if(EquippedWeapon == nullptr) return;

	const auto AmmoType = EquippedWeapon->GetAmmoType();
	
	// Update ammo map
	if(AmmoMap.Contains(EquippedWeapon->GetAmmoType())) {
		int32 CurrentAmmo = AmmoMap[AmmoType];
		const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();
		if(MagEmptySpace > CurrentAmmo) {
			// Reload the mag with all the ammo
			EquippedWeapon->ReloadAmmo(CurrentAmmo);
			CurrentAmmo = 0;
			AmmoMap.Add(AmmoType, CurrentAmmo);
		}else {
			// fill the mag
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CurrentAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CurrentAmmo);
		}
	}
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount) {
	if(OverlappedItemCount + Amount <= 0) {
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}else {
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

FVector AShooterCharacter::GetCameraInterpLocation() {
	const FVector CameraWorldLocation { FollowCamera->GetComponentLocation() };
	const FVector CameraForward { FollowCamera->GetForwardVector() };

	return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector(0.0f, 0.0f, CameraInterpElevation);
}

void AShooterCharacter::PickUpAmmo(AAmmo* Ammo) {
	if(AmmoMap.Find(Ammo->GetAmmoType())) {
		int32 AmmoCount = AmmoMap[Ammo->GetAmmoType()];
		AmmoCount += Ammo->GetItemCount();
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if(EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType()) {
		if(EquippedWeapon->GetAmmo() == 0) {
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

void AShooterCharacter::FKeyPressed() {
	if(EquippedWeapon->GetSlotIndex() == 0) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}
void AShooterCharacter::OneKeyPressed() {
	if(EquippedWeapon->GetSlotIndex() == 1) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}
void AShooterCharacter::TwoKeyPressed() {
	if(EquippedWeapon->GetSlotIndex() == 2) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}
void AShooterCharacter::ThreeKeyPressed() {
	if(EquippedWeapon->GetSlotIndex() == 3) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}
void AShooterCharacter::FourKeyPressed() {
	if(EquippedWeapon->GetSlotIndex() == 4) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
}
void AShooterCharacter::FiveKeyPressed() {
	if(EquippedWeapon->GetSlotIndex() == 5) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}
void AShooterCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex) {
	if((CurrentItemIndex == NewItemIndex) || (NewItemIndex >= Inventory.Num()) || (CombatState != ECombatState::ECS_Unoccupied)) return;

	auto OldEquippedWeapon = EquippedWeapon;
	auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
	EquipWeapon(NewWeapon);

	OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
	NewWeapon->SetItemState(EItemState::EIS_Equipped);
	
	
}

void AShooterCharacter::GetPickUpItem(AItem* Item) {
	auto Weapon = Cast<AWeapon>(Item);
	if(Weapon) {
		if(Inventory.Num() < INVENTORY_CAPACITY) {
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}else {
			SwapWeapon(Weapon);
		}
	}

	auto Ammo = Cast<AAmmo>(Item);
	if(Ammo) {
		PickUpAmmo(Ammo);
	}
}
