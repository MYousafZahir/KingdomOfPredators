// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StorkLogic.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KINGDOMOFPREDATORS_API UStorkLogic : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStorkLogic();

	TArray<TPair<int, int>> wantToMate;
	TArray<TPair<int, int>> matingCooldown;

	TSubclassOf<AActor> ActorBlueprint;

	UBlueprintGeneratedClass* CharBP;
	UBlueprintGeneratedClass* GrassBP;
	UBlueprintGeneratedClass* CorpseBP;

	void spawnCharacter(AActor* Parent1, AActor* Parent2);

	UPROPERTY()
	UBlueprint* CharacterBlueprint;

	UPROPERTY()
	UBlueprint* GrassBlueprint;


	UPROPERTY()
	UBlueprint* CorpseBlueprint;

	FTimerHandle CorpseTimer;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void Mate(AActor* Parent1, AActor* Parent2);


	UFUNCTION()
	void calculateNetAttributes();

	void SetMatingCooldown(int ID1, int ID2);

	UFUNCTION(BlueprintCallable)
	void MakeCorpse(float X, float Y);

	void SpawnCorpse(float X, float Y);

	float grassCooldown = 0.0f;

	UFUNCTION(BlueprintCallable)
	void MakeGrass(float time);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float n_attr_health = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float n_attr_teeth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float n_attr_gut = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float n_aff_health = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float n_aff_teeth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float n_aff_gut = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float n_aff_grass = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float n_aff_corpse = 0.0f;



		
};
