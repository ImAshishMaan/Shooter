#include "Shooter/Public/ShooterCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Items/Item.h"
#include "Items/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
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
	
}

void AShooterCharacter::BeginPlay() {
	Super::BeginPlay();
	
	if(FollowCamera) {
		CameraDefaultFOV = FollowCamera->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
	EquipWeapon(SpawnDefaultWeapon());
}

void AShooterCharacter::TraceForItems() {
	if(bShouldTraceForItems) {
		FHitResult ItemTraceRestult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceRestult, HitLocation);
		if(ItemTraceRestult.bBlockingHit) {
			AItem* HitItem = Cast<AItem>(ItemTraceRestult.GetActor());
			if(HitItem && HitItem->GetPickUpWidget()) {
				HitItem->GetPickUpWidget()->SetVisibility(true);
			}
			if(TraceHitItemLastFrame) {
				if(HitItem != TraceHitItemLastFrame) {
					TraceHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
				}
			}
			TraceHitItemLastFrame = HitItem;
		}
	} else if(TraceHitItemLastFrame) {
		TraceHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
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
	if(FireSound) {
		UGameplayStatics::PlaySound2D(this, FireSound);
		const FTransform SocketTransform = GetMesh()->GetSocketTransform(MuzzleFlashSocket);
		if(MuzzleFlashEffect) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlashEffect, SocketTransform);
		}
		
		FVector BeamEndPoint;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEndPoint);
		if(bBeamEnd) {
			if(ImpactEffect) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, BeamEndPoint);
			}
		
			if(BeamParticleEffect) {
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticleEffect, SocketTransform);
				if(Beam) {
					Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
				}
			}
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if(HipFireMontage && AnimInstance) {
				AnimInstance->Montage_Play(HipFireMontage);
				AnimInstance->Montage_JumpToSection("StartFire", HipFireMontage);
			}
		}
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
	StartFireTimer();
}
void AShooterCharacter::FireButtonReleased() {
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer() {
	if(bShouldFire) {
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
	}
}
void AShooterCharacter::AutoFireReset() {
	bShouldFire = true;
	if(bFireButtonPressed) {
		StartFireTimer();
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
	if(WeaponToEquip && WeaponToEquip->GetItemState() == EItemState::EIS_Pickup) {
		
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if(HandSocket) {
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
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
	if(EquippedWeapon) {
		DropWeapon();
	}
}
void AShooterCharacter::SelectButtonReleased() {
	
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
