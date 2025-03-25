// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "BaseCharacter.h"
#include "Interfaces/PickupInterface.h"
#include "SlashCharacter.generated.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UGroomComponent;
class UAnimMontage;
class AItem;
class ASoul;
class ATreasure;

UCLASS()
class MYPROJECT_API ASlashCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	ASlashCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	bool IsDodging();
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void AddSouls(ASoul* Souls) override;
	virtual void AddGold(ATreasure* Treasure) override;

	/*
	* Input Actions
	*/
	virtual void Jump() override;
	virtual void Attack() override;
	void Dodge();
	void UpdateStaminaBar();
	void EKeyPressed();

protected:
	virtual void BeginPlay() override;

	virtual void Die() override;

	void InitializePlayerOverlay(APlayerController* PlayerController);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* SlashCharacterMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* EKeyAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DodgeAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	/*
		Montage Functions
	*/
	virtual int32 PlayDeathMontage() override;
	virtual void PlayAttackMontage(float Playrate = 1) override;
	void PlayEquipMontage(const FName& SectionName);

	virtual void AttackEnd() override;

	UFUNCTION(BlueprintCallable)
	void SheatheWeapon();

	UFUNCTION(BlueprintCallable)
	void UnSheatheWeapon();

	bool IsIdle();
	bool CanUnequip();
	bool CanEquip();
	bool HasStamina();

	virtual bool IsDead() override;

private:
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState	ActionState = EActionState::EAS_Idle;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Hair;

	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Eyebrows;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	void SetHUDHealth();

	class UPlayerOverlay* PlayerOverlay;

	/*
		Animation Montages
	*/

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage;

public:
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
};
