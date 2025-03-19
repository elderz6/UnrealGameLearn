#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "BaseCharacter.generated.h"

class UAttributeComponent;
class AWeapon;
UCLASS()
class MYPROJECT_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);


protected:
	virtual void BeginPlay() override;

	virtual void Die();

	virtual void Attack();

	virtual void CanAttack();

	virtual void HandleDamage(float Damage);

	bool IsAlive();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	void DirectionalHitReact(const FVector& ImpactPoint);

	void PlayHitSound(const FVector& ImpactPoint);

	void SpawnHitParticles(const FVector& ImpactPoint);

	void DisableCapsule();
	
	/*
		Montage Functions
	*/
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DeathMontage;

	virtual void PlayAttackMontage(float PlayRate = 1);

	virtual void PlayHitReactMontage(const FName& SectionName);

	virtual void PlayDeathMontage();

	/*
		Components
	*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;

	UPROPERTY(EditAnywhere, Category = Sounds)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = VisualEffects)
	UParticleSystem* HitParticles;

private:	

};
