// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaunchPad.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UDecalComponent;

UCLASS()
class FPSGAME_API ALaunchPad : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALaunchPad();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* OverlapComp;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditInstanceOnly, Category = "Properties")
	float LaunchPitchAngle;

	UPROPERTY(EditInstanceOnly, Category = "Properties")
	float LaunchStrength = 1000;

	UPROPERTY(EditDefaultsOnly, Category = "Properties")
	UParticleSystem* ActivateLaunchPadEffect;
};
