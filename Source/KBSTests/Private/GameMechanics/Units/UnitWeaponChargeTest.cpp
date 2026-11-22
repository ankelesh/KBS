#include "Misc/AutomationTest.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Combat/Weapon.h"
#include "GameMechanics/Units/Combat/WeaponDataAsset.h"
#include "GameMechanics/Units/Components/UnitChargeComponent.h"
#include "GameMechanics/Units/Components/Config/UnitChargeComponentConfig.h"
#include "GameplayTagsManager.h"

namespace
{
	FGameplayTag GunpowderTag() { return FGameplayTag::RequestGameplayTag(TEXT("Unit.Charge.Gunpowder")); }

	UWeapon* MakeWeapon(AUnit* Unit, const FGameplayTag& ChargeTag, int32 Cost)
	{
		FWeaponData Data;
		Data.Name = FText::FromString(TEXT("TestWeapon"));
		Data.OverridePolicy = EDescriptorOverridePolicy::InlineOnly;
		Data.ChargeTag = ChargeTag;
		Data.ChargeCost = Cost;

		UWeapon* Weapon = NewObject<UWeapon>(Unit);
		Weapon->Initialize(Unit, Data);
		return Weapon;
	}

	AUnit* MakeUnitWithGunpowder(int32 Max)
	{
		AUnit* Unit = NewObject<AUnit>(GetTransientPackage());

		FUnitChargeComponentEntry Entry;
		Entry.Tag = GunpowderTag();
		Entry.CounterMax = Max;

		FUnitChargeComponentConfig Config;
		Config.Entries.Add(Entry);

		NewObject<UUnitChargeComponent>(Unit)->InitializeFromConfig(Config);
		return Unit;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitWeaponChargeGatingTest,
	"KBS.Units.Weapon.ChargeGating",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitWeaponChargeGatingTest::RunTest(const FString& Parameters)
{
	AUnit* Unit = MakeUnitWithGunpowder(1);
	const UWeapon* Axe = MakeWeapon(Unit, FGameplayTag(), 1);
	const UWeapon* Pistol = MakeWeapon(Unit, GunpowderTag(), 1);

	TestFalse("Free weapon reports no cost", Axe->GetChargeCost().Tag.IsValid());
	TestEqual("Costly weapon reports its tag", Pistol->GetChargeCost().Tag, GunpowderTag());

	TestTrue("Free weapon usable", Unit->CanUseWeapon(*Axe));
	TestTrue("Pistol usable with ammo", Unit->CanUseWeapon(*Pistol));

	const FUnitChargeCost FreePaid = Unit->PayWeaponCost(*Axe);
	TestFalse("Paying a free weapon pays nothing", FreePaid.Tag.IsValid());
	TestEqual("Ammo untouched by free weapon", Unit->GetCharges().GetCharge(GunpowderTag()), 1);

	const FUnitChargeCost Paid = Unit->PayWeaponCost(*Pistol);
	TestEqual("Paid cost tag", Paid.Tag, GunpowderTag());
	TestEqual("Paid cost amount", Paid.Amount, 1);
	TestEqual("Ammo spent", Unit->GetCharges().GetCharge(GunpowderTag()), 0);

	TestFalse("Pistol unusable without ammo", Unit->CanUseWeapon(*Pistol));
	TestTrue("Free weapon still usable without ammo", Unit->CanUseWeapon(*Axe));

	Unit->GetCharges().RefreshCharge(GunpowderTag());
	TestTrue("Pistol usable again after reload", Unit->CanUseWeapon(*Pistol));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitWeaponMultiCostTest,
	"KBS.Units.Weapon.MultiChargeCost",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitWeaponMultiCostTest::RunTest(const FString& Parameters)
{
	AUnit* Unit = MakeUnitWithGunpowder(3);
	const UWeapon* Cannon = MakeWeapon(Unit, GunpowderTag(), 2);

	TestTrue("Usable with 3 for cost 2", Unit->CanUseWeapon(*Cannon));
	Unit->PayWeaponCost(*Cannon);
	TestEqual("1 left", Unit->GetCharges().GetCharge(GunpowderTag()), 1);
	TestFalse("Unusable with 1 for cost 2", Unit->CanUseWeapon(*Cannon));
	return true;
}
