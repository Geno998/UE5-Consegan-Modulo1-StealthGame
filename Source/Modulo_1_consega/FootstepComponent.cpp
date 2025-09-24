#include "FootstepComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"


// Costruttore: abilita il Tick del componente
UFootstepComponent::UFootstepComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}


// Inizializzazione: chiamata quando il gioco parte
void UFootstepComponent::BeginPlay()
{
    Super::BeginPlay();
}


// Tick del componente (ogni frame):
// - Controlla se il personaggio si muove
// - Emette suoni e segnali di rumore a intervalli regolari
void UFootstepComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Ottieni il personaggio proprietario del componente
    ACharacter* Char = Cast<ACharacter>(GetOwner());
    if (!Char) return;

    // Recupera il componente movimento
    UCharacterMovementComponent* Move = Char->GetCharacterMovement();
    if (!Move || Move->IsCrouching()) return; // Se non valido o accovacciato, nessun passo

    // Calcola la velocità orizzontale del personaggio
    const float Speed = Char->GetVelocity().Size2D();
    if (Speed < 150.f) // Soglia minima per emettere passi
    {
        TimeSinceLastStep = 0.f;
        return;
    }

    // Se non sta toccando il terreno, non emette passi
    if (!Move->IsMovingOnGround())
    {
        TimeSinceLastStep = 0.f;
        return;
    }

    // Accumula il tempo dall’ultimo passo
    TimeSinceLastStep += DeltaTime;
    if (TimeSinceLastStep >= StepInterval)
    {
        // Reset del timer
        TimeSinceLastStep = 0.f;

        // Posizione attuale del personaggio
        FVector Loc = Char->GetActorLocation();

        // Se c’è un suono di passo configurato, riproducilo
        if (DefaultFootstep)
        {
            // Riproduci il suono alla posizione del personaggio
            UGameplayStatics::PlaySoundAtLocation(this, DefaultFootstep, Loc);

            // Genera un evento di rumore percepibile dall’IA (udito)
            UAISense_Hearing::ReportNoiseEvent(GetWorld(), Loc, 1.0f, Char, 500.f, FName("Footstep"));
        }
    }
}
