#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unequipped UMETA(DisplayName = "Unequiped"),
	ECS_EquippedOneHandedWeapon UMETA(DisplayName = "Equipped One Handed Weapon"),
	ECS_EquippedTwoHandedWeapon UMETA(DisplayName = "Equiped Two Handed Weapon")
};
UENUM(BlueprintType)
enum class EActionState : uint8
{
	EAS_Idle UMETA(DisplayName = "Unoccupied"),
	EAS_Using  UMETA(DisplayName = "Using"),
	EAS_Attacking  UMETA(DisplayName = "Attacking"),
	EAS_Equipping UMETA(DisplayName = "Equipping"),
};

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	EES_Idle UMETA(DisplayName = "Unoccupied"),
	EES_Dead UMETA(DisplayName = "Dead"),
	EES_Patrolling UMETA(DisplayName = "Patrolling"),
	EES_Chasing UMETA(DisplayName = "Chasing"),
	EES_Attacking  UMETA(DisplayName = "Attacking"),
	EES_Engaged UMETA(DisplayName = "Engaged"),
};

UENUM(BlueprintType)
enum class EDeathPose : uint8
{
	EDP_Death1 UMETA(DisplayName = "Death1"),
	EDP_Death2 UMETA(DisplayName = "Death2"),
	EDP_Death3 UMETA(DisplayName = "Death3")
};