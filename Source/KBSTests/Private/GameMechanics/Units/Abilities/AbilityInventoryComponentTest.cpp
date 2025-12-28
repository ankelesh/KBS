#include "Misc/AutomationTest.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Abilities/UnitAutoAttackAbility.h"
#include "GameMechanics/Units/Abilities/UnitMovementAbility.h"
#include "GameMechanics/Units/Abilities/UnitWaitAbility.h"
#include "GameMechanics/Units/Abilities/UnitDefendAbility.h"
#include "GameMechanics/Units/Abilities/UnitFleeAbility.h"
#include "GameMechanics/Units/Abilities/WeaponSwapAbility.h"
#include "GameMechanics/Units/Abilities/AbilityFactory.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameplayTypes/AbilityTypes.h"

class FAbilityInventoryTestHelper
{
public:
	static UUnitDefinition* CreateTestUnitDefinition(UObject* Outer)
	{
		UUnitDefinition* Definition = NewObject<UUnitDefinition>(Outer);
		Definition->UnitName = TEXT("TestUnit");
		Definition->BaseStatsTemplate.MaxHealth = 100.0f;
		return Definition;
	}

	static AUnit* CreateMockUnit(UWorld* World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AUnit* Unit = World->SpawnActor<AUnit>(AUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (Unit)
		{
			UUnitDefinition* Definition = CreateTestUnitDefinition(Unit);
			Unit->SetUnitDefinition(Definition);
		}
		return Unit;
	}

	static UUnitAbilityDefinition* CreateTestAbilityDefinition(UObject* Outer, TSubclassOf<UUnitAbilityInstance> AbilityClass, const FString& Name = TEXT("TestAbility"), bool bIsPassive = false, bool bIsSpellbook = false)
	{
		UUnitAbilityDefinition* Definition = NewObject<UUnitAbilityDefinition>(Outer);
		Definition->AbilityClass = AbilityClass;
		Definition->AbilityName = Name;
		Definition->MaxCharges = 3;
		Definition->bIsPassive = bIsPassive;
		Definition->bIsSpellbookAbility = bIsSpellbook;
		return Definition;
	}
};

// Test: AddActiveAbility adds auto-attack to DefaultAttackAbility slot
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryAddActiveAbilityAutoAttackTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.AddActiveAbility_AutoAttack_AddsToDefaultSlot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryAddActiveAbilityAutoAttackTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* Definition = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitAutoAttackAbility::StaticClass(), TEXT("Attack"));
	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);

	Inventory->AddActiveAbility(Ability);

	TestEqual("DefaultAttackAbility should be set", Inventory->GetDefaultAbility(EDefaultAbilitySlot::Attack), Ability);

	Unit->Destroy();
	return true;
}

// Test: AddActiveAbility adds movement to DefaultMoveAbility slot
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryAddActiveAbilityMovementTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.AddActiveAbility_Movement_AddsToDefaultSlot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryAddActiveAbilityMovementTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* Definition = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitMovementAbility::StaticClass(), TEXT("Move"));
	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);

	Inventory->AddActiveAbility(Ability);

	TestEqual("DefaultMoveAbility should be set", Inventory->GetDefaultAbility(EDefaultAbilitySlot::Move), Ability);

	Unit->Destroy();
	return true;
}

// Test: AddActiveAbility adds wait to DefaultWaitAbility slot
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryAddActiveAbilityWaitTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.AddActiveAbility_Wait_AddsToDefaultSlot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryAddActiveAbilityWaitTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* Definition = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitWaitAbility::StaticClass(), TEXT("Wait"));
	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);

	Inventory->AddActiveAbility(Ability);

	TestEqual("DefaultWaitAbility should be set", Inventory->GetDefaultAbility(EDefaultAbilitySlot::Wait), Ability);

	Unit->Destroy();
	return true;
}

// Test: AddActiveAbility adds defend to DefaultDefendAbility slot
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryAddActiveAbilityDefendTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.AddActiveAbility_Defend_AddsToDefaultSlot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryAddActiveAbilityDefendTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* Definition = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitDefendAbility::StaticClass(), TEXT("Defend"));
	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);

	Inventory->AddActiveAbility(Ability);

	TestEqual("DefaultDefendAbility should be set", Inventory->GetDefaultAbility(EDefaultAbilitySlot::Defend), Ability);

	Unit->Destroy();
	return true;
}

// Test: AddActiveAbility adds flee to DefaultFleeAbility slot
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryAddActiveAbilityFleeTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.AddActiveAbility_Flee_AddsToDefaultSlot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryAddActiveAbilityFleeTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* Definition = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitFleeAbility::StaticClass(), TEXT("Flee"));
	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);

	Inventory->AddActiveAbility(Ability);

	TestEqual("DefaultFleeAbility should be set", Inventory->GetDefaultAbility(EDefaultAbilitySlot::Flee), Ability);

	Unit->Destroy();
	return true;
}

// Test: AddActiveAbility adds spellbook ability to spellbook array
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryAddActiveAbilitySpellbookTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.AddActiveAbility_Spellbook_AddsToSpellbookArray",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryAddActiveAbilitySpellbookTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* Definition = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitWaitAbility::StaticClass(), TEXT("Fireball"), false, true);
	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);

	Inventory->AddActiveAbility(Ability);

	TArray<UUnitAbilityInstance*> SpellbookAbilities = Inventory->GetSpellbookAbilities();
	TestEqual("Spellbook should contain 1 ability", SpellbookAbilities.Num(), 1);
	TestEqual("Spellbook should contain the added ability", SpellbookAbilities[0], Ability);

	Unit->Destroy();
	return true;
}

// Test: AddActiveAbility adds custom ability to AvailableActiveAbilities
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryAddActiveAbilityCustomTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.AddActiveAbility_Custom_AddsToAvailableAbilities",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryAddActiveAbilityCustomTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* Definition = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UWeaponSwapAbility::StaticClass(), TEXT("CustomAbility"));
	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);

	Inventory->AddActiveAbility(Ability);

	TArray<UUnitAbilityInstance*> AvailableAbilities = Inventory->GetAvailableActiveAbilities();
	TestTrue("Available abilities should contain the custom ability", AvailableAbilities.Contains(Ability));

	Unit->Destroy();
	return true;
}

// Test: AddPassiveAbility adds to PassiveAbilities
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryAddPassiveAbilityValidTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.AddPassiveAbility_Valid_AddsToPassiveAbilities",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryAddPassiveAbilityValidTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* Definition = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitWaitAbility::StaticClass(), TEXT("PassiveAbility"), true);
	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);

	Inventory->AddPassiveAbility(Ability);

	TArray<UUnitAbilityInstance*> PassiveAbilities = Inventory->GetPassiveAbilities();
	TestEqual("PassiveAbilities should contain 1 ability", PassiveAbilities.Num(), 1);
	TestEqual("PassiveAbilities should contain the added ability", PassiveAbilities[0], Ability);

	Unit->Destroy();
	return true;
}

// Test: AddPassiveAbility ignores non-passive abilities
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryAddPassiveAbilityNotPassiveTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.AddPassiveAbility_NotPassive_IgnoresAbility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryAddPassiveAbilityNotPassiveTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* Definition = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitWaitAbility::StaticClass(), TEXT("ActiveAbility"), false);
	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);

	Inventory->AddPassiveAbility(Ability);

	TArray<UUnitAbilityInstance*> PassiveAbilities = Inventory->GetPassiveAbilities();
	TestEqual("PassiveAbilities should be empty", PassiveAbilities.Num(), 0);

	Unit->Destroy();
	return true;
}

// Test: RemovePassiveAbility removes from PassiveAbilities
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryRemovePassiveAbilityTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.RemovePassiveAbility_RemovesFromArray",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryRemovePassiveAbilityTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* Definition = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitWaitAbility::StaticClass(), TEXT("PassiveAbility"), true);
	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);

	Inventory->AddPassiveAbility(Ability);
	TestEqual("PassiveAbilities should have 1 ability", Inventory->GetPassiveAbilities().Num(), 1);

	Inventory->RemovePassiveAbility(Ability);
	TestEqual("PassiveAbilities should be empty after removal", Inventory->GetPassiveAbilities().Num(), 0);

	Unit->Destroy();
	return true;
}

// Test: EquipAbility sets CurrentActiveAbility
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryEquipAbilityTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.EquipAbility_SetsCurrentActiveAbility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryEquipAbilityTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* Definition = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitWaitAbility::StaticClass(), TEXT("TestAbility"));
	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);

	Inventory->EquipAbility(Ability);

	TestEqual("CurrentActiveAbility should be set", Inventory->GetCurrentActiveAbility(), Ability);

	Unit->Destroy();
	return true;
}

// Test: EquipDefaultAbility equips attack ability
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryEquipDefaultAbilityTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.EquipDefaultAbility_EquipsAttackAbility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryEquipDefaultAbilityTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* Definition = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitAutoAttackAbility::StaticClass(), TEXT("Attack"));
	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);
	Inventory->AddActiveAbility(Ability);

	Inventory->EquipDefaultAbility();

	TestEqual("CurrentActiveAbility should be the attack ability", Inventory->GetCurrentActiveAbility(), Ability);

	Unit->Destroy();
	return true;
}

// Test: EquipDefaultAbility sets null when no attack ability
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryEquipDefaultAbilityNoAttackTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.EquipDefaultAbility_NoAttack_SetsNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryEquipDefaultAbilityNoAttackTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	Inventory->EquipDefaultAbility();

	TestNull("CurrentActiveAbility should be null when no attack ability", Inventory->GetCurrentActiveAbility());

	Unit->Destroy();
	return true;
}

// Test: GetAvailableActiveAbilities returns all default and custom abilities
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryGetAvailableActiveAbilitiesTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.GetAvailableActiveAbilities_ReturnsAllAbilities",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryGetAvailableActiveAbilitiesTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* AttackDef = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitAutoAttackAbility::StaticClass(), TEXT("Attack"));
	UUnitAbilityInstance* AttackAbility = UAbilityFactory::CreateAbilityFromDefinition(AttackDef, Unit);
	Inventory->AddActiveAbility(AttackAbility);

	UUnitAbilityDefinition* CustomDef = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UWeaponSwapAbility::StaticClass(), TEXT("Custom"));
	UUnitAbilityInstance* CustomAbility = UAbilityFactory::CreateAbilityFromDefinition(CustomDef, Unit);
	Inventory->AddActiveAbility(CustomAbility);

	TArray<UUnitAbilityInstance*> AvailableAbilities = Inventory->GetAvailableActiveAbilities();

	TestEqual("Should have 2 abilities", AvailableAbilities.Num(), 2);
	TestTrue("Should contain attack ability", AvailableAbilities.Contains(AttackAbility));
	TestTrue("Should contain custom ability", AvailableAbilities.Contains(CustomAbility));

	Unit->Destroy();
	return true;
}

// Test: GetAllDefaultAbilities returns only default abilities
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryGetAllDefaultAbilitiesTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.GetAllDefaultAbilities_ReturnsOnlyDefaultAbilities",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryGetAllDefaultAbilitiesTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* AttackDef = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitAutoAttackAbility::StaticClass(), TEXT("Attack"));
	UUnitAbilityInstance* AttackAbility = UAbilityFactory::CreateAbilityFromDefinition(AttackDef, Unit);
	Inventory->AddActiveAbility(AttackAbility);

	UUnitAbilityDefinition* MoveDef = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitMovementAbility::StaticClass(), TEXT("Move"));
	UUnitAbilityInstance* MoveAbility = UAbilityFactory::CreateAbilityFromDefinition(MoveDef, Unit);
	Inventory->AddActiveAbility(MoveAbility);

	UUnitAbilityDefinition* CustomDef = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UWeaponSwapAbility::StaticClass(), TEXT("Custom"));
	UUnitAbilityInstance* CustomAbility = UAbilityFactory::CreateAbilityFromDefinition(CustomDef, Unit);
	Inventory->AddActiveAbility(CustomAbility);

	TArray<UUnitAbilityInstance*> DefaultAbilities = Inventory->GetAllDefaultAbilities();

	TestEqual("Should have 2 default abilities", DefaultAbilities.Num(), 2);
	TestTrue("Should contain attack ability", DefaultAbilities.Contains(AttackAbility));
	TestTrue("Should contain move ability", DefaultAbilities.Contains(MoveAbility));
	TestFalse("Should not contain custom ability", DefaultAbilities.Contains(CustomAbility));

	Unit->Destroy();
	return true;
}

// Test: SetDefaultAbility and GetDefaultAbility for all slots
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventorySetGetDefaultAbilityTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.SetGetDefaultAbility_WorksForAllSlots",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventorySetGetDefaultAbilityTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* Definition = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitWaitAbility::StaticClass(), TEXT("TestAbility"));
	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);

	Inventory->SetDefaultAbility(EDefaultAbilitySlot::Attack, Ability);
	TestEqual("Attack slot should be set", Inventory->GetDefaultAbility(EDefaultAbilitySlot::Attack), Ability);

	Inventory->SetDefaultAbility(EDefaultAbilitySlot::Move, Ability);
	TestEqual("Move slot should be set", Inventory->GetDefaultAbility(EDefaultAbilitySlot::Move), Ability);

	Inventory->SetDefaultAbility(EDefaultAbilitySlot::Wait, Ability);
	TestEqual("Wait slot should be set", Inventory->GetDefaultAbility(EDefaultAbilitySlot::Wait), Ability);

	Inventory->SetDefaultAbility(EDefaultAbilitySlot::Defend, Ability);
	TestEqual("Defend slot should be set", Inventory->GetDefaultAbility(EDefaultAbilitySlot::Defend), Ability);

	Inventory->SetDefaultAbility(EDefaultAbilitySlot::Flee, Ability);
	TestEqual("Flee slot should be set", Inventory->GetDefaultAbility(EDefaultAbilitySlot::Flee), Ability);

	Unit->Destroy();
	return true;
}

// Test: GetSpellbookAbilities returns only spellbook abilities
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventoryGetSpellbookAbilitiesTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.GetSpellbookAbilities_ReturnsOnlySpellbookAbilities",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventoryGetSpellbookAbilitiesTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* SpellDef = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitWaitAbility::StaticClass(), TEXT("Fireball"), false, true);
	UUnitAbilityInstance* SpellAbility = UAbilityFactory::CreateAbilityFromDefinition(SpellDef, Unit);
	Inventory->AddActiveAbility(SpellAbility);

	UUnitAbilityDefinition* NormalDef = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitWaitAbility::StaticClass(), TEXT("Normal"));
	UUnitAbilityInstance* NormalAbility = UAbilityFactory::CreateAbilityFromDefinition(NormalDef, Unit);
	Inventory->AddActiveAbility(NormalAbility);

	TArray<UUnitAbilityInstance*> SpellbookAbilities = Inventory->GetSpellbookAbilities();

	TestEqual("Should have 1 spellbook ability", SpellbookAbilities.Num(), 1);
	TestEqual("Should contain the spell ability", SpellbookAbilities[0], SpellAbility);

	Unit->Destroy();
	return true;
}

// Test: SelectAttackAbility sets CurrentActiveAbility to attack
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityInventorySelectAttackAbilityTest,
	"KBS.Units.Abilities.AbilityInventoryComponent.SelectAttackAbility_SetsCurrentToAttack",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityInventorySelectAttackAbilityTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityInventoryTestHelper::CreateMockUnit(World);
	UAbilityInventoryComponent* Inventory = Unit->GetAbilityInventory();

	UUnitAbilityDefinition* AttackDef = FAbilityInventoryTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitAutoAttackAbility::StaticClass(), TEXT("Attack"));
	UUnitAbilityInstance* AttackAbility = UAbilityFactory::CreateAbilityFromDefinition(AttackDef, Unit);
	Inventory->AddActiveAbility(AttackAbility);

	Inventory->SelectAttackAbility();

	TestEqual("CurrentActiveAbility should be attack", Inventory->GetCurrentActiveAbility(), AttackAbility);

	Unit->Destroy();
	return true;
}
