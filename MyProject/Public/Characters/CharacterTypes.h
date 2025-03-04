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
};