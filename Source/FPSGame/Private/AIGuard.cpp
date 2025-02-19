// Fill out your copyright notice in the Description page of Project Settings.


#include "AIGuard.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "FPSGameMode.h"
#include "Engine/World.h"
#include "AINavigationVertex.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AAIGuard::AAIGuard()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GuardCollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(FName("GuardCollisionComponent"));
	GuardCollisionComponent->SetupAttachment(RootComponent);

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

	PawnSensingComp->OnSeePawn.AddDynamic(this, &AAIGuard::OnPawnSeen);
	PawnSensingComp->OnHearNoise.AddDynamic(this, &AAIGuard::OnPawnHeard);

	GuardState = EAIState::Idle;
}

// Called when the game starts or when spawned
void AAIGuard::BeginPlay()
{
	Super::BeginPlay();
	OriginalRotation = GetActorRotation();
	CurrentPatrolPoint = FirstPatrolPoint;
}

// Called every frame
void AAIGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GuardState == EAIState::Idle) {
		MoveToNavVertex(CurrentPatrolPoint);
	}
}

void AAIGuard::ResetOrientation()
{
	if (GuardState == EAIState::Alerted) { return; }
	SetActorRotation(OriginalRotation);
	SetGuardState(EAIState::Idle);
}

void AAIGuard::OnPawnSeen(APawn* SeenPawn) {
	if (!SeenPawn) { return; }
	DrawDebugSphere(GetWorld(), SeenPawn->GetActorLocation(), 32.f, 12, FColor::Yellow, false, 10.f);

	AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
	if (GM) {
		if (SeenPawn->InputEnabled()) {
			GM->CompleteMission(SeenPawn, false);
		}
	}

	SetGuardState(EAIState::Alerted);
}

void AAIGuard::OnPawnHeard(APawn* HeardPawn, const FVector& Location, float Volume) {
	if (GuardState == EAIState::Alerted) { return; }

	AAIController* GuardController = Cast<AAIController>(GetController());
	if (GuardController) {
		GuardController->StopMovement();
	}

	DrawDebugSphere(GetWorld(), Location, 32.f, 12, FColor::Red, false, 10.f);

	FVector Direction = Location - GetActorLocation();
	Direction.Normalize();

	FRotator NewLookAt = FRotationMatrix::MakeFromX(Direction).Rotator();
	NewLookAt.Pitch = 0;
	NewLookAt.Roll = 0;

	SetActorRotation(NewLookAt);

	GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrientation);
	
	GetWorldTimerManager().SetTimer(TimerHandle_ResetOrientation, this, &AAIGuard::ResetOrientation, 3.f, false);

	SetGuardState(EAIState::Suspicious);
}

void AAIGuard::OnRep_GuardState()
{
	OnStateChanged(GuardState);
}

void AAIGuard::SetGuardState(EAIState NewState) {
	if (GuardState == NewState) {
		return;
	}
	GuardState = NewState;
	OnRep_GuardState();
}

/* DO NOT USE THIS CODE TO INFORM LATER MOVEMENT CODE. This code is exploratory in nature. Instead refer to lecture 31 video code for this project or to the code in Coop Game*/
void AAIGuard::MoveToNavVertex(AAINavigationVertex* DestinationVertex) {
	if (!DestinationVertex) { return; }

	AAIController* GuardController = Cast<AAIController>(GetController());
	if (!GuardController) { return; }
	if (GetDistanceTo(DestinationVertex) < MoveAcceptanceRadius) {
		if (DestinationVertex == FirstPatrolPoint) {
			CurrentPatrolPoint = SecondPatrolPoint;
		}
		else {
			CurrentPatrolPoint = FirstPatrolPoint;
		}
	}
	else {
		GuardController->MoveToActor(DestinationVertex, MoveAcceptanceRadius);
	}
}

void AAIGuard::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAIGuard, GuardState);
}