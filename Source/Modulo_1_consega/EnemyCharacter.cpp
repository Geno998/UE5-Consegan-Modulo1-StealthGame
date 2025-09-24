#include "EnemyCharacter.h"
#include "EnemyAIController.h"
#include "UObject/ConstructorHelpers.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"  

// Costruttore: imposta AI, movimento e aspetto del nemico
AEnemyCharacter::AEnemyCharacter()
{
    // Fa sì che l’AI Controller venga assegnato automaticamente
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AEnemyAIController::StaticClass();

    // Configurazione rotazione e movimento
    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = Speed_Patrol;

    // --- (Opzionale) Assegnazione mesh via codice ---
    // In genere si fa da Blueprint, ma può essere fatto anche qui
    // ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshRef(TEXT("/Game/Characters/Enemy/Mesh.Enemy"));
    // if (MeshRef.Succeeded()) { GetMesh()->SetSkeletalMesh(MeshRef.Object); }

    // --- Assegna l’Anim Blueprint al nemico ---
    static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPClass(TEXT("/Game/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny_C"));
    if (AnimBPClass.Succeeded())
    {
        GetMesh()->SetAnimInstanceClass(AnimBPClass.Class);
    }

    // --- Posizionamento relativo della mesh (tipico per rig Mannequin) ---
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
}

// Funzione chiamata quando il gioco inizia o l’attore viene spawnato
// Imposta lo stato iniziale in base ai PatrolPoints disponibili
void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Se non ci sono waypoint, il nemico resta in attesa, altrimenti inizia in pattuglia
    CurrentState = PatrolPoints.Num() > 0 ? EEnemyState::Patrol : EEnemyState::Waiting;
    ApplySpeedForState(CurrentState);
}

// Applica la velocità di movimento in base allo stato dell’IA
void AEnemyCharacter::ApplySpeedForState(EEnemyState NewState)
{
    UCharacterMovementComponent* Move = GetCharacterMovement();
    if (!Move) return;

    float Target = Speed_Patrol;
    switch (NewState)
    {
    case EEnemyState::Waiting:     Target = Speed_Waiting;     break; // fermo
    case EEnemyState::Patrol:      Target = Speed_Patrol;      break; // pattuglia
    case EEnemyState::Investigate: Target = Speed_Investigate; break; // indaga
    case EEnemyState::Attack:      Target = Speed_Attack;      break; // inseguimento/attacco
    default: break;
    }

    // Aggiorna la velocità massima
    Move->MaxWalkSpeed = Target;
}