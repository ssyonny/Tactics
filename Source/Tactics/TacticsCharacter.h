// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "TacticsCharacter.generated.h"

/**
 *  A controllable top-down perspective character
 */
UCLASS(Blueprintable, BlueprintType)
class TACTICS_API ATacticsCharacter : public ACharacter
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
	void PerformAttack(); // Remove UFUNCTION to prevent blueprint override
	
	/** Test function to bypass blueprint override */
	void TestPerformAttack();
	
	/** Emergency test function with unique name */
	void EmergencyAttackTest();

	/** Returns the camera component **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }

	/** Returns the Camera Boom component **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

protected:

	/** Combat properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float BaseDamage = 20.0f;

	/** Attack animation montages */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	class UAnimMontage* AttackMontage1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	class UAnimMontage* AttackMontage2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	class UAnimMontage* AttackMontage3;

	/** Attack visual effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effects")
	class UNiagaraSystem* AttackTrailEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effects")
	class UNiagaraSystem* AttackImpactEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Effects")
	class UNiagaraComponent* TrailComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float AttackCooldown = 0.5f;

	/** Screen shake effect for attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effects")
	TSubclassOf<class UCameraShakeBase> AttackCameraShake;

	/** Current attack cooldown timer */
	float CurrentAttackCooldown = 0.0f;

	/** Check if can attack */
	bool CanAttack() const;

	/** Apply damage to target using damage formula */
	UFUNCTION(BlueprintCallable, Category="Combat")
	float CalculateDamage(float TargetArmor) const;

	/** Play random attack animation */
	UFUNCTION(BlueprintCallable, Category="Animation")
	void PlayAttackAnimation();

	/** Called when attack animation finishes */
	UFUNCTION(BlueprintCallable, Category="Animation")
	void OnAttackAnimationFinished();

	/** Spawn attack visual effects */
	UFUNCTION(BlueprintCallable, Category="Effects")
	void SpawnAttackEffects();

	/** Spawn impact effect at location */
	UFUNCTION(BlueprintCallable, Category="Effects")
	void SpawnImpactEffect(FVector Location);

	/** Get attack direction based on mouse cursor position */
	UFUNCTION(BlueprintCallable, Category="Combat")
	FVector GetAttackDirection() const;

	/** Force immediate rotation to attack direction */
	UFUNCTION(BlueprintCallable, Category="Combat")
	void ForceRotateToDirection(FVector Direction);

private:
	/** Store last attack direction for consistency */
	FVector LastAttackDirection = FVector::ForwardVector;

};

