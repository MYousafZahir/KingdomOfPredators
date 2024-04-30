#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "DataCommunicator.h"
#include <tuple>
#include <vector>

//#include "Engine/EngineTypes.h"

#include "Data.generated.h"

//using namespace std;

UCLASS(ClassGroup = (Custom), Blueprintable, meta = (BlueprintSpawnableComponent))
class KINGDOMOFPREDATORS_API UData : public UActorComponent, public IDataCommunicator
{
    GENERATED_BODY()

private:
    static const int directionalArrSize = 45;

    // hash map for ID to affinity
    TMap<int, int> IDMap;
    void initializeAttributes();
    

    // variables for direction
    const int centeredAngle = 360 - (directionalArrSize / 2);
    std::tuple<float, FVector> directionTuple[directionalArrSize];
    
    TArray<AActor*> raycastIgnoreList;

    FCollisionObjectQueryParams OnlyPawns = FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn);
    FCollisionObjectQueryParams OnlyGrassAndCorpses = FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldDynamic);

    // ECC_Pawn and OverlapAllDynamic
    FCollisionObjectQueryParams PawnsAndCorpses = FCollisionObjectQueryParams(ECC_TO_BITFIELD(ECollisionChannel::ECC_Pawn) | ECC_TO_BITFIELD(ECollisionChannel::ECC_WorldDynamic));


    UFUNCTION()
    int gameObjectToAffinity(AActor* gameObject);

    UFUNCTION()
    FVector newForwardDirection(AActor* actor);

    void DecayMemory();

    float lastHit = 0.0f;

    FTimerHandle DeathTimer;
    void Death();

    float energy = 10.0f;

    // stork reference
    AActor* StorkActor;

public:
    UData();

    // ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int ID = 0;

    int memory[360];
    int* directionalArr = &memory[0];

    // Attributes, all editable in the editor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
    int attr_health = 100;
    int attr_healthMax = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
    int attr_teeth = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
    int attr_gut = 100;

    // Affinities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affinities")
    int aff_health = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affinities")
    int aff_teeth = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affinities")
    int aff_gut = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affinities")
    int aff_grass = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affinities")
    int aff_corpse = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affinities")
    int affForMate = 180;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int highestHit = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int highestHitI = 0;



    DECLARE_MULTICAST_DELEGATE_TwoParams(FDataSignature, const FString&, AActor*);
    FDataSignature DataDelegate;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void SendData(const FString& Data, AActor* TargetActor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void ReceiveData(const FString& Data, AActor* SenderActor);

    UFUNCTION()
    void OnDataReceived(const FString& Data, AActor* SenderActor);

    UFUNCTION(BlueprintCallable)
    void WriteStringOverCharacter(FString statename);
    
    UFUNCTION(BlueprintCallable)
    int getIDMapVal(int IDtoGet);


    UFUNCTION(BlueprintCallable)
    void wipeData();


    void ProcessHit(int damage);

    void UpdateHealth();




protected:
    virtual void BeginPlay() override;

    void Parser(FString toParse);

    void newForwardDirectionProcess(AActor* Owner, int arrOffSet, int i, TArray<FHitResult>* Hits);

    int CalcAttraction(AActor* target);

    UPROPERTY(VisibleAnywhere)
    UCapsuleComponent* CapsuleComponent;

public:

    // Owner Stuff
    AActor* Owner = GetOwner();

    // Mated with
    TArray<int> MatedWith;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    /*void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);*/

    UFUNCTION(BlueprintCallable)
    void AddEnergy(float energyToAdd);

    UFUNCTION(BlueprintCallable)
    float GetEnergy();

    void initializeAttributes(int attr_health1, int attr_health2, int attr_teeth1, int attr_teeth2, int attr_gut1, int attr_gut2, int aff_grass1, int aff_grass2, int aff_corpse1, int aff_corpse2);


    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool seeDebugLines = false;

};