// Copyright Mercury Massif


#include "AbilitySystem/Abilities/AuraSummonAbility.h"

#include "Kismet/KismetSystemLibrary.h"

TArray<FVector> UAuraSummonAbility::GetSpawnLocations()
{
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	const float DeltaSpread = SpawnSpread / NumMinions;

	// const FVector RightOfSpread = Forward.RotateAngleAxis(SpawnSpread / 2.f, FVector::UpVector);
	// UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), Location,
	//                                      Location + RightOfSpread * MaxSpawnDistance, 4.f, FLinearColor::Green, 4.f);
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpawnSpread / 2.f, FVector::UpVector);
	// UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), Location,
	//                                      Location + LeftOfSpread * MaxSpawnDistance, 4.f, FLinearColor::Red, 4.f);

	TArray<FVector> SpawnLocations;
	for (auto i = 0; i < NumMinions; i++)
	{
		const auto Direction = LeftOfSpread.RotateAngleAxis(i * DeltaSpread, FVector::UpVector);
		auto ChosenSpawnLocation = Location + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);

		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, ChosenSpawnLocation + FVector(0, 0, 400), ChosenSpawnLocation - FVector(0, 0, 400), ECC_Visibility);
		ChosenSpawnLocation = Hit.ImpactPoint;
		SpawnLocations.AddUnique(ChosenSpawnLocation);
	}

	return SpawnLocations;
}

TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass()
{
	const auto Index = FMath::RandRange(0, MinionClasses.Num() - 1);
	return MinionClasses[Index];
}
