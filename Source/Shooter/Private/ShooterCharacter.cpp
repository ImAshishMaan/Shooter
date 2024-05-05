#include "Shooter/Public/ShooterCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

AShooterCharacter::AShooterCharacter() {
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true; // Rotate arm based on controller
	CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 50.0f);

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
}

void AShooterCharacter::BeginPlay() {
	Super::BeginPlay();
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
			FHitResult ScreenTraceHit;
			const FVector Start = CrosshairWorldPosition;
			const FVector End = CrosshairWorldPosition + CrosshairWorldDirection * 50'000.0f;

			FVector BeamEndPoint = End;
			GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);
			
			if(ScreenTraceHit.bBlockingHit) {
				BeamEndPoint = ScreenTraceHit.Location; // Set the endpoint to the location of the hit
				if(ImpactEffect) {
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, ScreenTraceHit.Location);
				}
			}
			if(BeamParticleEffect) {
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticleEffect, SocketTransform);
				if(Beam) {
					Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
				}
			}
		}
		
		/*FHitResult FireHit;
		const FVector FireStart = SocketTransform.GetLocation();
		const FQuat Rotation { SocketTransform.GetRotation() };
		const FVector RotationAxis { Rotation.GetAxisX() };
		const FVector FireEnd { FireStart + RotationAxis * 50'000.0f };

		FVector BeamEndPoint = FireEnd;
		
		GetWorld()->LineTraceSingleByChannel(FireHit, FireStart, FireEnd, ECollisionChannel::ECC_Visibility);
		if(FireHit.bBlockingHit) {
			BeamEndPoint = FireHit.ImpactPoint;
				
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, FireHit.ImpactPoint);
		}
		if(BeamParticleEffect) {
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticleEffect, SocketTransform);
			if(Beam) {
				Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
			}
		}
		
		}
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if(HipFireMontage && AnimInstance) {
			AnimInstance->Montage_Play(HipFireMontage);
			AnimInstance->Montage_JumpToSection("StartFire", HipFireMontage);
		}*/
	}
}

void AShooterCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
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
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireWeapon);
	
}
