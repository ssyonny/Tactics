// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TacticsCharacter.generated.h"

/**
 *  A controllable top-down perspective character
 */
UCLASS(abstract)
class ATacticsCharacter : public ACharacter
{
	GENERATED_BODY()

private:

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

public:

	/** Constructor */
	ATacticsCharacter();

	/** Initialization */
	virtual void BeginPlay() override;

	/** Update */
	virtual void Tick(float DeltaSeconds) override;

	/** Perform basic attack */
	UFUNCTION(BlueprintCallable, Category="Combat")
	void PerformAttack();

	/** Returns the camera component **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }

	/** Returns the Camera Boom component **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

protected:

	/** Combat properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float BaseDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float AttackCooldown = 0.5f;

	/** Current attack cooldown timer */
	float CurrentAttackCooldown = 0.0f;

	/** Check if can attack */
	bool CanAttack() const;

	/** Apply damage to target using damage formula */
	UFUNCTION(BlueprintCallable, Category="Combat")
	float CalculateDamage(float TargetArmor) const;

};

