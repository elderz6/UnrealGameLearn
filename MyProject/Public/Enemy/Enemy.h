// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterTypes.h"
#include "Characters/BaseCharacter.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class UPawnSensingComponent;
class FInputActionValue;
struct FAIRequestID;
struct FPathFollowingResult;

UCLASS()
class MYPROJECT_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();
	virtual void Tick(float DeltaTime) override;

	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

	virtual void Die() override;

	bool InTargetRange(AActor* Target, double Radius);

	//Defined in blueprints
	UFUNCTION(BlueprintImplementableEvent)
	void OnDie();

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);

	void UnbindPatrolEvent();

	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);

	virtual void Attack() override;

	virtual void AttackEnd() override;

	bool IsChasing();

	bool IsAttacking();

	bool IsDead();

	bool IsEngaged();
	
	/*
		Montage Functions
	*/

	virtual void PlayAttackMontage(float PlayRate = 1) override;

	virtual void PlayDeathMontage() override;

	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose;

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState;


private:
	void SetHealthBarVisibility(bool Visible);

	void ChasePlayer();

	void ClearPatrolTimer();

	void StartAttackTimer();

	void ClearAttackTimer();

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMin = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMax = 1.f;

	UPROPERTY()
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere)
	double CombatRadius = 500.f;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	double PatrolSpeed = 125.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	double ChaseSpeed = 300.f;

	UPROPERTY(EditAnywhere)
	double AttackRadius = 100.f;

	class FDelegateHandle MoveCompleteHandle;

	/*
	*	Components
	*/

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthWidget;
	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;

	/*
	*	Navigation
	*/
	bool IsPatrolling = false;

	UPROPERTY()
	class AAIController* EnemyController;

	//Current Patrol target
	UPROPERTY(EditInstanceOnly, Category = "Ai Navigation")
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "Ai Navigation")
	TArray<AActor*> PatrolTargets;

	TArray<AActor*> RemainingPatrolTargets;

	FTimerHandle PatrolTimer;
	void PatrolTimerFinished();

	void BindPatrolEvent();

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMin = 2.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMax = 3.f;

	void MoveToTarget(AActor* Target);

	AActor* ChoosePatrolTarget();

	void CheckCombatTarget();

	void CheckPatrolTarget();

	void StartPatrol();

};
