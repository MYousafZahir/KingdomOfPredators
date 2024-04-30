// Fill out your copyright notice in the Description page of Project Settings.


#include "Data.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

#include "EngineUtils.h" 
#include "GameFramework/Actor.h"  
#include <StorkLogic.h>

// Sets default values for this component's properties

//UData::FDataSignature UData::DataDelegate;



UData::UData()
{
    // set ID as random number from 1 to 20000000
   

    MatedWith.Empty();

    PrimaryComponentTick.bCanEverTick = true;

    for (int i = 0; i < 360; i++) {
        memory[i] = 0;
    }


    DataDelegate.AddUObject(this, &UData::OnDataReceived);


    // Get owners capsule component

    //if (Owner) {
    //    CapsuleComponent = Owner->FindComponentByClass<UCapsuleComponent>();
    //    if (CapsuleComponent) {


    //        //CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &UData::OnOverlapBegin);
    //        /*GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Check0");*/
    //    }
    //}


}


// Called when the game starts
void UData::BeginPlay()
{
    Super::BeginPlay();

    initializeAttributes();
    

    // print ID
    ID = FMath::RandRange(1, 20000000);
    FString IDStr = FString::FromInt(ID);
    //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "ID is: " + IDStr);

    // energy
    energy = 100.0f;

    // ...

    // initialize directionTuple
    for (int i = 0; i < directionalArrSize; i++) {
        float val1 = FMath::DegreesToRadians(centeredAngle + i);
        FVector val2(FMath::Cos(val1), FMath::Sin(val1), 0.0f);

        directionTuple[i] = std::make_tuple(val1, val2);
    }

    // setup stork reference
    UWorld* world = Owner->GetWorld();

    // find stork actor
    for (TActorIterator<AActor> ActorItr(world); ActorItr; ++ActorItr)
    {
        if (ActorItr->GetName().Mid(0, 8) == "BP_Stork") {
			StorkActor = *ActorItr;
            //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Stork found");
			break;
		}
	}


}


// Called every frame
void UData::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    FVector OwnerLocation = Owner->GetActorLocation();
    FRotator OwnerRotation = Owner->GetActorRotation().Clamp();

    int arrOffSet = OwnerRotation.Yaw;
    directionalArr = &memory[arrOffSet];

    if(lastHit > 0) lastHit -= DeltaTime;

    if (energy > 0) {
        energy -= DeltaTime;
        if (energy <= 0) ProcessHit(attr_health);
    }

    UpdateHealth();


    // Raycast parameters
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(Owner);
    FHitResult HitResult;


    for (int32 i = 0; i < directionalArrSize; i++) {
        // this is used to know the direction of the raycast relative to the owner
        FVector Direction = OwnerRotation.RotateVector(std::get<1>(directionTuple[i]));

        TArray<FHitResult> Hits;

        // Perform the raycast
        if (GetWorld()->LineTraceMultiByObjectType(Hits, OwnerLocation, OwnerLocation + Direction * 1000.0f, PawnsAndCorpses, TraceParams))
        {
            int sumOfHits = 0;
            for (auto Hit : Hits) {
                if (Hit.GetActor()->GetName().Mid(0, 12) == "BP_Character") {
                    if (!IDMap.Contains(Hit.GetActor()->FindComponentByClass<UData>()->ID)) {
                        IDMap.Add(Hit.GetActor()->FindComponentByClass<UData>()->ID, CalcAttraction(Hit.GetActor()));

                    }

                    if (!(Hit.GetActor()->FindComponentByClass<UData>()->IDMap.Contains(ID))) {
						Hit.GetActor()->FindComponentByClass<UData>()->IDMap.Add(ID, CalcAttraction(Owner));

					}
                }


                sumOfHits += gameObjectToAffinity(Hit.GetActor());

            }

            memory[(arrOffSet + i) % 360] = sumOfHits;

            // update highest if necessary
            if (sumOfHits > highestHit) {
				highestHit = sumOfHits;
				highestHitI = (arrOffSet + i) % 360;

				//newForwardDirection(Owner);
			}

            //newForwardDirectionProcess(Owner, arrOffSet, i, &Hits);

            // Get Play_UI 
            if(seeDebugLines)DrawDebugLine(GetWorld(), OwnerLocation, OwnerLocation + Direction * 1000.0f, FColor::Red, false, 0.1f, 0, 1.0f);

        }
        else {
            if (seeDebugLines)DrawDebugLine(GetWorld(), OwnerLocation, OwnerLocation + Direction * 1000.0f, FColor::Green, false, 0.1f, 0, 1.0f);


        }
    }

    DecayMemory();

}


int UData::CalcAttraction(AActor* target) {
    int attraction = 0;

    attraction += abs(target->FindComponentByClass<UData>()->attr_health - aff_health);
    attraction += abs(target->FindComponentByClass<UData>()->attr_teeth - aff_teeth);
    attraction += abs(target->FindComponentByClass<UData>()->attr_gut - aff_gut);

    return attraction;

}

void UData::SendData_Implementation(const FString& Data, AActor* TargetActor) {

    if (TargetActor) {
        UData* DataComponent = TargetActor->FindComponentByClass<UData>();
        if (DataComponent) {
            DataComponent->OnDataReceived(Data, GetOwner());
        }
    }

}

void UData::OnDataReceived(const FString& Data, AActor* SenderActor) {
    ReceiveData_Implementation(Data, SenderActor);
}

void UData::ReceiveData_Implementation(const FString& Data, AActor* SenderActor)
{
    Parser(Data);
}


void UData::Parser(FString toParse) {
    FString title = "";
    FString data = "";

    for (int i = 0; i < toParse.Len(); i++) {
        if (toParse[i] == '|') {
            title = toParse.Mid(0, i);
            data = toParse.Mid(i + 1, toParse.Len());
            break;
        }

    }

    if (title == "hit") {
        ProcessHit(FCString::Atoi(*data));
    }
}

int UData::gameObjectToAffinity(AActor* gameObject) {
    FString name = gameObject->GetName();

    if (name.Mid(0, 5) == "Grass") {
        return aff_grass;
    }
    else if (name.Mid(0, 9) == "BP_Corpse") {
        return aff_corpse;
    }
    else if (name.Mid(0, 12) == "BP_Character") {
        return IDMap[gameObject->FindComponentByClass<UData>()->ID];
    }

    return 0;
}


void UData::newForwardDirectionProcess(AActor* O, int arrOffSet, int i, TArray<FHitResult>* Hits) {
    int index = (arrOffSet + i) % 360;
    memory[index] = 0;


    for (auto Hit : *Hits)
        if (Hit.GetActor() != O) memory[index] += gameObjectToAffinity(Hit.GetActor());

    if (memory[index] > highestHit) {
        highestHit = memory[index];
        highestHitI = index;

        newForwardDirection(O);
    }

}


// function that returns new forward direction for owner
FVector UData::newForwardDirection(AActor* actor) {
    // owner rotatio is new rotation
    FRotator newRotation = actor->GetActorRotation();
    newRotation.Yaw = highestHitI;

    FVector output = newRotation.Vector();
    return output;
}


void UData::initializeAttributes() {
    int sumOfAttributes = 260;

    // randomly distribute sumOfAttributes to attributes
    attr_health = FMath::RandRange(0, sumOfAttributes);
    sumOfAttributes -= attr_health;

    attr_teeth = FMath::RandRange(0, sumOfAttributes);
    sumOfAttributes -= attr_teeth;

    attr_gut = sumOfAttributes;



    aff_health = FMath::RandRange(0, 100);
    aff_teeth = FMath::RandRange(0, 100);
    aff_gut = FMath::RandRange(0, 100);


    aff_grass = FMath::RandRange(0, 100);
    aff_corpse = FMath::RandRange(0, 100);
}

void UData::initializeAttributes(int attr_health1, int attr_health2, int attr_teeth1, int attr_teeth2, int attr_gut1, int attr_gut2, int aff_grass1, int aff_grass2, int aff_corpse1, int aff_corpse2) {
    attr_health = ((attr_health1 + attr_health2)/2) + FMath::RandRange(-10, 10);
    attr_teeth = ((attr_teeth1 + attr_teeth2) / 2) + FMath::RandRange(-10, 10);
    attr_gut = ((attr_gut1 + attr_gut2) / 2) + FMath::RandRange(-10, 10);

    aff_grass = ((aff_grass1 + aff_grass2) / 2) + FMath::RandRange(-10, 10);
    aff_corpse = ((aff_corpse1 + aff_corpse2) / 2) + FMath::RandRange(-10, 10);

}


void UData::DecayMemory() {
    int newHighest = 0;
    for (int i = 0; i < 360; i++) {
        if (memory[i] <= 100 && memory[i] > 0) {
            memory[i]-= 0.3f;
        }
        else {
            memory[i]  *= 0.9;
        }

        if (memory[i] > newHighest) newHighest = memory[i];
    }

    highestHit = newHighest;
}


int UData::getIDMapVal(int IDtoGet) {
    if(IDMap.Contains(IDtoGet)) return IDMap[IDtoGet];

    return -100;

}

void UData::WriteStringOverCharacter(FString statename) {
    //FString stateName = "State:";
    Owner->FindComponentByClass<UTextRenderComponent>()->SetText(FText::FromString(statename));
}

void UData::wipeData() {
    highestHit = 0;
	highestHitI = 0;

	for (int i = 0; i < 360; i++) {
		memory[i] = 0;
	}
}


void UData::ProcessHit(int damage) {
    if (lastHit > 0.0f) return;

    attr_health -= damage;

    if (attr_health <= 0) {
        float newX = FMath::RandRange(0.0f, 800.0f);
        float newY = FMath::RandRange(-600.0f, -1400.0f);


        StorkActor->FindComponentByClass<UStorkLogic>()->MakeCorpse(Owner->GetActorLocation().X, Owner->GetActorLocation().Y);
        Owner->SetActorLocation(FVector(newX, newY, -600.0f));
        
        Owner->GetWorldTimerManager().SetTimer(DeathTimer, this, &UData::Death, 3.0f, false);
    }

    // damage to string
    //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "hit for " + FString::FromInt(damage));
    lastHit = 0.5f;
}

void UData::Death() {
	Owner->Destroy();
}

void UData::AddEnergy(float energyToAdd) {
    if (energy + energyToAdd <= 100) {
        energy += energyToAdd;
    }
    else {
        energy = 100;
    }
}

float UData::GetEnergy() {
	return energy;
}

void UData::UpdateHealth() {
    if (energy >= 80) {
        if (attr_health < attr_healthMax) {
            attr_health += 0.15;
        }
    }
}
