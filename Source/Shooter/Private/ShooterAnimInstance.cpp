#include "Shooter/Public/ShooterAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Shooter/Public/ShooterCharacter.h"

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime) {
	if(ShooterCharacter == nullptr) {
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}

	if(ShooterCharacter) {
		FVector Velocity = ShooterCharacter->GetVelocity();
		Velocity.Z = 0;
		Speed = Velocity.Size();
		
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();
		
		bIsAccelerating = ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0; // 0 if no acceleration
	}
	
}

void UShooterAnimInstance::NativeInitializeAnimation() {
	Super::NativeInitializeAnimation();
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	
}
