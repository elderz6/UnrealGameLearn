#include "Enemy/Enemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/AttributeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AIController.h"
#include "NavigationData.h"
#include "Items/Weapons/Weapon.h"
#include "MyProject/DebugMacros.h"
#include "Items/Soul.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetGenerateOverlapEvents(true);
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	
	HealthWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthWidget"));
	HealthWidget->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SightRadius = 4000.f;
	PawnSensing->SetPeripheralVisionAngle(90.f);

	EnemyState = EEnemyState::EES_Idle;
}


void AEnemy::Die()
{
	Super::Die();
	EnemyState = EEnemyState::EES_Dead;
	SpawnSoul();
	ClearAttackTimer();
	SetLifeSpan(5.f);
	EquippedWeapon->SetLifeSpan(5.f);
	RotateTowardsPlayer(false);
	SetHealthBarVisibility(false);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	OnDie();
}

void AEnemy::SpawnSoul()
{
	UWorld* World = GetWorld();

	if (World && SoulClass)
	{
		const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 25.f);
		ASoul* SpawnedSoul = World->SpawnActor<ASoul>(SoulClass, SpawnLocation, GetActorRotation());
		if (SpawnedSoul)
		{
			SpawnedSoul->SetSouls(Attributes->GetSouls());
			SpawnedSoul->UpdateNiagaraVariables();
		}
	}
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	if (Target == nullptr) return false;
	const double Distance = (Target->GetActorLocation() - GetActorLocation()).Size();
	return Distance <= Radius;
}


int32 AEnemy::PlayDeathMontage()
{
	Super::PlayDeathMontage();

	return 0;
}

void AEnemy::SetHealthBarVisibility(bool Visible)
{
	if (HealthWidget)
		HealthWidget->SetVisibility(Visible);
}

void AEnemy::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();
	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RHandSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

/*

	Combat

*/

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	if(!IsDead())
		SetHealthBarVisibility(true);
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (HealthWidget)
	{
		HandleDamage(DamageAmount);
		HealthWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}

	CombatTarget = EventInstigator->GetPawn();
	ChasePlayer();

	return DamageAmount;
}

void AEnemy::ChasePlayer()
{
	if (CombatTarget)
	{
		UnbindPatrolEvent();
		EnemyState = EEnemyState::EES_Chasing;
		ClearAttackTimer();
		ClearPatrolTimer();
		GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
		MoveToTarget(CombatTarget);
	}
}

void AEnemy::Attack()
{
	Super::Attack();

	if (CombatTarget == nullptr) return;

	if (!IsDead())
	{
		EnemyState = EEnemyState::EES_Engaged;
		PlayAttackMontage();
	}
}

void AEnemy::AttackEnd()
{
	if (!IsDead())
	{
		EnemyState = EEnemyState::EES_Idle;
		CheckCombatTarget();
	}
}

void AEnemy::PlayAttackMontage(float PlayRate)
{
	Super::PlayAttackMontage();
}


void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}


void AEnemy::CheckCombatTarget()
{
	if (!InTargetRange(CombatTarget, CombatRadius))
	{
		if (!IsEngaged())
		{
			StartPatrol();
			BindPatrolEvent();
		}
	}
	else if (!InTargetRange(CombatTarget, AttackRadius) && !IsChasing())
	{
		if (!IsEngaged()) ChasePlayer();
	}
	else if (InTargetRange(CombatTarget, AttackRadius) && !IsEngaged() && !IsDead())
	{
		float Remain = GetWorldTimerManager().GetTimerRemaining(AttackTimer);
		if (Remain > 0) return;
		StartAttackTimer();
	}
}


void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}


bool AEnemy::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsDead()
{
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsEngaged()
{
	return EnemyState == EEnemyState::EES_Engaged;
}

/*
	Patrol Movement
*/

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	if (IsChasing() || IsDead() ) return;

	if (SeenPawn->ActorHasTag("Dead")) return;

	if (SeenPawn->ActorHasTag(FName("SlashCharacter")))
	{
		if (InTargetRange(SeenPawn, CombatRadius) && !IsAttacking() && !IsEngaged())
		{
			CombatTarget = SeenPawn;
			ChasePlayer();
		}
	}
}
void AEnemy::BindPatrolEvent()
{
	MoveCompleteHandle = EnemyController->GetPathFollowingComponent()->OnRequestFinished.AddUObject(this, &AEnemy::OnMoveCompleted);
}
void AEnemy::UnbindPatrolEvent()
{
	if (MoveCompleteHandle.IsValid())
	{
		EnemyController->GetPathFollowingComponent()->OnRequestFinished.Remove(MoveCompleteHandle);
	}
}

void AEnemy::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	EnemyState = EEnemyState::EES_Idle;
	RemainingPatrolTargets.Remove(PatrolTarget);
	UnbindPatrolEvent();
	GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, 3.f);
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return;
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(75.f);
	EnemyController->MoveTo(MoveRequest);
}

AActor* AEnemy::ChoosePatrolTarget()
{
	if (RemainingPatrolTargets.Num() == 0)
	{
		RemainingPatrolTargets = PatrolTargets;
	}
	const int32 NumPatrolTargets = RemainingPatrolTargets.Num();
	const int32 Selection = FMath::RandRange(0, NumPatrolTargets - 1);
	return RemainingPatrolTargets[Selection];
}

void AEnemy::CheckPatrolTarget()
{
	if (EnemyState == EEnemyState::EES_Patrolling) return;

	if (InTargetRange(PatrolTarget, PatrolRadius))
	{
		EnemyState = EEnemyState::EES_Patrolling;
		PatrolTarget = ChoosePatrolTarget();
		const float WaitTime = FMath::RandRange(WaitMin, WaitMax);
		BindPatrolEvent();
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}

void AEnemy::StartPatrol()
{
	ClearAttackTimer();
	CombatTarget = nullptr;
	SetHealthBarVisibility(false);
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
	RemainingPatrolTargets = PatrolTargets;
	MoveToTarget(PatrolTarget);
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	SetHealthBarVisibility(false);
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;

	EnemyController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);

	PatrolTarget = ChoosePatrolTarget();
	SpawnDefaultWeapon();

	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}

	Tags.Add(FName("Enemy"));
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;

	if (EnemyState > EEnemyState::EES_Patrolling)
		CheckCombatTarget();
	else
		CheckPatrolTarget();
}
