#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Shooter/AmmoType.h"
#include "Ammo.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AAmmo : public AItem {
	GENERATED_BODY()

public:
	AAmmo();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	virtual void SetItemProperties(EItemState State) override;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* AmmoMeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

public:
	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() { return AmmoMeshComp; }
	FORCEINLINE EAmmoType GetAmmoType() { return AmmoType; }
};
