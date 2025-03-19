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

#include "MyProject/DebugMacros.h"

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
	PawnSensing->SetPeripheralVisionAngle(45.f);

	EnemyState = EEnemyState::EES_Idle;
}


void AEnemy::Die()
{
	EnemyState = EEnemyState::EES_Dead;
	ClearAttackTimer();
	PlayDeathMontage();
	DisableCapsule();
	SetLifeSpan(5.f);
	SetHealthBarVisibility(false);
	GetMesh()->SetGenerateOverlapEvents(false);
	OnDie();
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	if (Target == nullptr) return false;
	const double Distance = (Target->GetActorLocation() - GetActorLocation()).Size();
	return Distance <= Radius;
}


void AEnemy::PlayDeathMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		const int32 Selection = FMath::RandRange(1, DeathMontage->GetNumSections());
		FName SectionName = DeathMontage->GetSectionName(Selection-1);
		DeathPose = EDeathPose(Selection - 1);

		AnimInstance->Montage_Play(DeathMontage, 1.5);
		AnimInstance->Montage_JumpToSection(SectionName, DeathMontage);
	}
}


void AEnemy::SetHealthBarVisibility(bool Visible)
{
	if (HealthWidget)
		HealthWidget->SetVisibility(Visible);
}

/*

	Combat

*/

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	SetHealthBarVisibility(true);
	
	if(IsAlive()) 
		DirectionalHitReact(ImpactPoint);
	else
		Die();

	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);

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
		UE_LOG(LogTemp, Warning, TEXT("Chasing Player"));
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
	EnemyState = EEnemyState::EES_Engaged;
	if (!IsDead())
		PlayAttackMontage();
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_Idle;
	CheckCombatTarget();
}

void AEnemy::PlayAttackMontage(float PlayRate)
{
	Super::PlayAttackMontage();
}


void AEnemy::StartAttackTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("Setting Attack timer"));
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
			UE_LOG(LogTemp, Warning, TEXT("Return to Patrol"));
			StartPatrol();
			BindPatrolEvent();
		}
	}
	else if (!InTargetRange(CombatTarget, AttackRadius) && !IsChasing())
	{
		UE_LOG(LogTemp, Warning, TEXT("Chasing if"));
		if (!IsEngaged()) ChasePlayer();
	}
	else if (InTargetRange(CombatTarget, AttackRadius) && !IsEngaged() && !IsDead())
	{
		float remain = GetWorldTimerManager().GetTimerRemaining(AttackTimer);
		if (remain > 0) return;
		UE_LOG(LogTemp, Warning, TEXT("In Range, Attack"));
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
	UE_LOG(LogTemp, Warning, TEXT("Event bound"));
	MoveCompleteHandle = EnemyController->GetPathFollowingComponent()->OnRequestFinished.AddUObject(this, &AEnemy::OnMoveCompleted);
}
void AEnemy::UnbindPatrolEvent()
{
	if (MoveCompleteHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Event unbound"));
		EnemyController->GetPathFollowingComponent()->OnRequestFinished.Remove(MoveCompleteHandle);
	}
}

void AEnemy::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	EnemyState = EEnemyState::EES_Idle;
	RemainingPatrolTargets.Remove(PatrolTarget);
	UnbindPatrolEvent();
	UE_LOG(LogTemp, Warning, TEXT("Movement Finished"));
	GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, 3.f);
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return;
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(15.f);
	EnemyController->MoveTo(MoveRequest);
}

AActor* AEnemy::ChoosePatrolTarget()
{
	if (RemainingPatrolTargets.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Targets empty, resetting"));
		RemainingPatrolTargets = PatrolTargets;
	}
	const int32 NumPatrolTargets = RemainingPatrolTargets.Num();
	const int32 Selection = FMath::RandRange(0, NumPatrolTargets - 1);
	UE_LOG(LogTemp, Warning, TEXT("Selecting target %d Total targets %d"), Selection, NumPatrolTargets);
	return RemainingPatrolTargets[Selection];
}

void AEnemy::CheckPatrolTarget()
{
	if (EnemyState == EEnemyState::EES_Patrolling) return;

	if (InTargetRange(PatrolTarget, PatrolRadius))
	{
		UE_LOG(LogTemp, Warning, TEXT("Check patrol, set timer"));
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
	EnemyState = EEnemyState::EES_Idle;
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
	RemainingPatrolTargets = PatrolTargets;
	MoveToTarget(PatrolTarget);
}

void AEnemy::PatrolTimerFinished()
{
	UE_LOG(LogTemp, Warning, TEXT("Patrol Timer Finished"));
	MoveToTarget(PatrolTarget);
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	SetHealthBarVisibility(false);
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
	UE_LOG(LogTemp, Warning, TEXT("Begin Play Set patrol"));

	EnemyController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);

	PatrolTarget = ChoosePatrolTarget();

	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}
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
