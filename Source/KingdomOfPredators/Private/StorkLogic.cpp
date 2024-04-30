// Fill out your copyright notice in the Description page of Project Settings.

#include "StorkLogic.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPtr.h"
#include <Data.h>
#include <Kismet/GameplayStatics.h>

// Sets default values for this component's properties
UStorkLogic::UStorkLogic()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
}


// Called when the game starts
void UStorkLogic::BeginPlay()
{
	Super::BeginPlay();


	UObject* LoadedObj = StaticLoadObject(UBlueprint::StaticClass(), nullptr, TEXT("/Game/creature_components/BP_Character.BP_Character"));
	CharacterBlueprint = Cast<UBlueprint>(LoadedObj);
	CharBP = Cast<UBlueprintGeneratedClass>(CharacterBlueprint->GeneratedClass);

	UObject* LoadedGrass = StaticLoadObject(UBlueprint::StaticClass(), nullptr, TEXT("/Game/Blueprints/Grass.Grass"));
	GrassBlueprint = Cast<UBlueprint>(LoadedGrass);
	GrassBP = Cast<UBlueprintGeneratedClass>(GrassBlueprint->GeneratedClass);

	UObject* LoadedCorpse = StaticLoadObject(UBlueprint::StaticClass(), nullptr, TEXT("/Game/creature_components/BP_Corpse.BP_Corpse"));
	CorpseBlueprint = Cast<UBlueprint>(LoadedCorpse);
	CorpseBP = Cast<UBlueprintGeneratedClass>(CorpseBlueprint->GeneratedClass);

	grassCooldown = 10.0f;
}


// Called every frame
void UStorkLogic::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	MakeGrass(DeltaTime);

	calculateNetAttributes();
	// ...


}

void UStorkLogic::Mate(AActor* Parent1, AActor* Parent2) {
	// parent1 and parent2 are BP_characters, get their ID's from Data component
	int ID1 = Parent1->FindComponentByClass<UData>()->ID;
	int ID2 = Parent2->FindComponentByClass<UData>()->ID;

	TPair<int, int> mates;

	if (ID1 < ID2) {
		mates = TPair<int, int>(ID1, ID2);
	} else {
		mates = TPair<int, int>(ID2, ID1);
	}

	if (matingCooldown.Contains(mates)) {
		return;
	}

	if (wantToMate.Contains(mates)) {
		wantToMate.Remove(mates);
		SetMatingCooldown(ID1, ID2);

		// spawn new BP_character
		spawnCharacter(Parent1, Parent2);


	} else {
		wantToMate.Add(TPair<int, int>(ID1, ID2));
	}
}

void UStorkLogic::SetMatingCooldown(int ID1, int ID2) {
	TPair<int, int> mates;

	if (ID1 < ID2) {
		mates = TPair<int, int>(ID1, ID2);
	}
	else {
		mates = TPair<int, int>(ID2, ID1);
	}

	matingCooldown.Add(mates);
}



void UStorkLogic::spawnCharacter(AActor* Parent1, AActor* Parent2) {
	if (!CharBP) {
		return;
	}

	float X = FMath::RandRange(-400.0f, 2000.0f);
	float Y = FMath::RandRange(-2200.0f, 200.0f);


	// Parent1 and Parent2 are BP_characters, get their data components
	UData* Parent1Data = Parent1->FindComponentByClass<UData>();
	UData* Parent2Data = Parent2->FindComponentByClass<UData>();

	// health, teeth, gut, grass, corpse, all should be inherited from parents
	int health = (Parent1Data->attr_health + Parent2Data->attr_health) / 2;
	int teeth = (Parent1Data->attr_teeth + Parent2Data->attr_teeth) / 2;
	int gut = (Parent1Data->attr_gut + Parent2Data->attr_gut) / 2;
	int grass = (Parent1Data->aff_grass + Parent2Data->aff_grass) / 2;
	int corpse = (Parent1Data->aff_corpse + Parent2Data->aff_corpse) / 2;




	FVector Location(X, Y, 0.0f);
	FRotator Rotation(0.0f, 0.0f, 0.0f);

	FActorSpawnParameters SpawnParameters;
	
	// spawn CharacterBlueprint
	APawn* SpawnedActor = GetWorld()->SpawnActor<APawn>(CharBP, Location, Rotation, SpawnParameters);

	if (SpawnedActor) {
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Spawned successfully"));
		SpawnedActor->SpawnDefaultController();

		//SpawnedActor->SpawnDefaultController();
		if (SpawnedActor->GetController()) {
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Controller spawned successfully"));

			SpawnedActor->FindComponentByClass<UData>()->initializeAttributes(health, health, teeth, teeth, gut, gut, grass, grass, corpse, corpse);
		} else {
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to spawn controller"));
		}
	}
	else {
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to spawn actor"));
	}
}


void UStorkLogic::MakeGrass(float time) {
	if (grassCooldown > 0) {
		grassCooldown -= time;
		return;
	}
	else {
		grassCooldown = 10.0f;

		float X = FMath::RandRange(-400.0f, 2000.0f);
		float Y = FMath::RandRange(-2200.0f, 200.0f);

		FVector Location(X, Y, -21.0f);
		FRotator Rotation(0.0f, 0.0f, 0.0f);

		FActorSpawnParameters SpawnParameters;

		// spawn CharacterBlueprint
		APawn* SpawnedActor = GetWorld()->SpawnActor<APawn>(GrassBP, Location, Rotation, SpawnParameters);
	}
}

void UStorkLogic::calculateNetAttributes() {
	// get all BP_characters in the world
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), CharBP, FoundActors);

	n_attr_health = 0.0f;
	n_attr_teeth = 0.0f;
	n_attr_gut = 0.0f;
	n_aff_health = 0.0f;
	n_aff_teeth = 0.0f;
	n_aff_gut = 0.0f;
	n_aff_grass = 0.0f;
	n_aff_corpse = 0.0f;


	// iterate through all BP_characters
	for (AActor* Actor : FoundActors) {
		UData* Data = Actor->FindComponentByClass<UData>();

		n_attr_health += Data->attr_health;
		n_attr_teeth += Data->attr_teeth;
		n_attr_gut += Data->attr_gut;
		n_aff_health += Data->aff_health;
		n_aff_teeth += Data->aff_teeth;
		n_aff_gut += Data->aff_gut;
		n_aff_grass += Data->aff_grass;
		n_aff_corpse += Data->aff_corpse;

	}

	n_attr_health /= FoundActors.Num();
	n_attr_teeth /= FoundActors.Num();
	n_attr_gut /= FoundActors.Num();
	n_aff_health /= FoundActors.Num();
	n_aff_teeth /= FoundActors.Num();
	n_aff_gut /= FoundActors.Num();
	n_aff_grass /= FoundActors.Num();
	n_aff_corpse /= FoundActors.Num();


}


void UStorkLogic::MakeCorpse(float X, float Y) {
	GetWorld()->GetTimerManager().SetTimer(CorpseTimer, [this, X, Y]() {
		SpawnCorpse(X, Y);
		}, 0.2f, false);
}

void UStorkLogic::SpawnCorpse(float X, float Y) {
	FVector Location(X, Y, 4.0f);
	FRotator Rotation(0.0f, 0.0f, 0.0f);

	FActorSpawnParameters SpawnParameters;

	// spawn CharacterBlueprint
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(CorpseBP, Location, Rotation, SpawnParameters);
}	