#include "EnemyAIController.h"
#include "EnemyCharacter.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense.h"

#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

#include "Modulo_1_consegaCharacter.h"


// Costruttore: configura Tick, Perception e sensi (vista/udito)
AEnemyAIController::AEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Crea il componente di percezione
    Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));

    // --- Vista: raggio, angolo e affiliazioni rilevabili ---
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2500.f;
    SightConfig->LoseSightRadius = 3000.f;
    SightConfig->PeripheralVisionAngleDegrees = 80.f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    // --- Udito: raggio e affiliazioni rilevabili ---
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 500.f;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

    // Registra i sensi nella Perception e setta la vista come dominante
    Perception->ConfigureSense(*SightConfig);
    Perception->ConfigureSense(*HearingConfig);
    Perception->SetDominantSense(SightConfig->GetSenseImplementation());

    // Aggancia il callback quando la percezione cambia
    Perception->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::OnPerceptionUpdated);

    // Richiede un aggiornamento iniziale degli stimoli
    Perception->RequestStimuliListenerUpdate();
}


// Chiamato quando il controller prende possesso del Pawn nemico
// Imposta lo stato iniziale (Patrol se ci sono waypoint, altrimenti Waiting)
void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    Enemy = Cast<AEnemyCharacter>(InPawn);
    if (Enemy)
    {
        const EEnemyState Initial = (Enemy->PatrolPoints.Num() > 0)
            ? EEnemyState::Patrol
            : EEnemyState::Waiting;

        SetState(Initial);
    }
}


// Tick per frame: esegue la logica in base allo stato corrente
void AEnemyAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    switch (State)
    {
    case EEnemyState::Waiting:     DoWaiting();     break;   // Attesa
    case EEnemyState::Patrol:      DoPatrol();      break;   // Pattuglia
    case EEnemyState::Investigate: DoInvestigate(); break;   // Indagine
    case EEnemyState::Attack:      DoAttack();      break;   // Attacco/inseguimento
    default: break;
    }
}


// Callback percezione: reazione a vista/udito del giocatore
// - Vista: se visto, passa ad Attack; se perso, dopo un grace time vai a Investigate
// - Udito: se non in Attack, vai a Investigate sulla posizione del rumore
void AEnemyAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stim)
{
    if (!Enemy) return;

    AModulo_1_consegaCharacter* Player = Cast<AModulo_1_consegaCharacter>(Actor);
    if (!Player) return;

    const FAISenseID SightID = UAISense::GetSenseID(UAISense_Sight::StaticClass());
    const FAISenseID HearingID = UAISense::GetSenseID(UAISense_Hearing::StaticClass());

    if (Stim.Type == SightID)
    {
        if (Stim.WasSuccessfullySensed())
        {
            // Vista positiva: memorizza il bersaglio e attacca
            TargetActor = Player;
            SetState(EEnemyState::Attack);
            GetWorld()->GetTimerManager().ClearTimer(LoseSightTimer);
        }
        else
        {
            // Vista persa: dopo LoseSightGrace passa a Investigate sull’ultima posizione vista
            const FVector LastSeen = Stim.StimulusLocation;
            GetWorld()->GetTimerManager().SetTimer(
                LoseSightTimer,
                [this, LastSeen]()
                {
                    TargetActor.Reset();
                    if (State == EEnemyState::Attack)
                    {
                        InvestigateLocation = LastSeen;
                        SetState(EEnemyState::Investigate);
                    }
                },
                LoseSightGrace, false);
        }
    }
    else if (Stim.Type == HearingID)
    {
        // Rumore udito: se non stai già attaccando, investiga
        if (State != EEnemyState::Attack)
        {
            InvestigateLocation = Stim.StimulusLocation;
            SetState(EEnemyState::Investigate);
        }
    }
}


// Chiamato a fine movimento: decide la prossima azione in base allo stato
// - Patrol: attende al waypoint per WaitAtWaypoint secondi
// - Investigate: attende un breve periodo prima di riprendere la pattuglia
void AEnemyAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    if (!Enemy) return;

    if (State == EEnemyState::Patrol)
    {
        StartWait(Enemy->WaitAtWaypoint);
    }
    else if (State == EEnemyState::Investigate)
    {
        StartWait(5.0f);
    }
}

// Cambia stato in modo sicuro: cancella timer/movimento, aggiorna nemico e avvia nuova logica
void AEnemyAIController::SetState(EEnemyState NewState)
{
    // Evita ri-settare lo stesso stato, tranne per Investigate (ri-avviabile)
    if (State == NewState && NewState != EEnemyState::Investigate)
        return;

    // Pulisce eventuali attese in corso
    GetWorld()->GetTimerManager().ClearTimer(WaitHandle);

    // Interrompe l’eventuale path in corso
    if (UPathFollowingComponent* PFC = GetPathFollowingComponent())
    {
        PFC->AbortMove(*this, FPathFollowingResultFlags::ForcedScript);
    }
    else
    {
        StopMovement();
    }

    // Cancella il focus sull’attore precedente
    ClearFocus(EAIFocusPriority::Gameplay);

    // Aggiorna lo stato
    State = NewState;

    // Allinea lo stato/velocità del Character nemico
    if (Enemy)
    {
        Enemy->SetState(State);
        Enemy->ApplySpeedForState(State);
    }

    // Avvia l’azione collegata allo stato
    switch (State)
    {
    case EEnemyState::Waiting:
        // Attendi per un periodo (di default quello del nemico, altrimenti un fallback)
        StartWait(Enemy ? Enemy->WaitAtWaypoint : 5.0f);
        break;

    case EEnemyState::Patrol:
        // Inizia/continua la pattuglia
        GoToNextPatrolPoint();
        break;

    case EEnemyState::Investigate:
        // Raggiungi la posizione sospetta e poi OnMoveCompleted gestirà l’attesa
        MoveToLocation(InvestigateLocation, 120.f, false, true, true, true, nullptr, true);
        break;

    case EEnemyState::Attack:
        // Nessun movimento immediato qui: la logica è nel Tick (DoAttack)
        break;

    default: break;
    }
}


// Stato: Attesa (non fa nulla; EndWait deciderà la transizione)
void AEnemyAIController::DoWaiting()
{
    // In attesa; EndWait() porterà in Patrol se necessario
}


// Stato: Pattuglia (se fermo, ordina il movimento al prossimo punto)
void AEnemyAIController::DoPatrol()
{
    if (GetMoveStatus() != EPathFollowingStatus::Moving)
    {
        GoToNextPatrolPoint();
    }
}

// Stato: Indagine (la logica di arrivo è in OnMoveCompleted)
void AEnemyAIController::DoInvestigate()
{
    // All’arrivo, OnMoveCompleted() attende e poi torna in pattuglia
}


// Stato: Attacco/Inseguimento
// - Se il bersaglio non è valido, passa a Investigate
// - Se in vista e a portata, ferma e "colpisce" (qui gestisce Game Over)
// - Altrimenti muoviti verso il bersaglio; se fallisce, investiga l’ultima posizione
void AEnemyAIController::DoAttack()
{
    // Se il bersaglio non è più valido (nullptr, distrutto, ecc.), smetti di attaccare e vai a investigare l’ultima posizione nota.
    if (!TargetActor.IsValid())
    {
        SetState(EEnemyState::Investigate);
        return;
    }

    // Recupera il Pawn controllato da questo AIController
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;


    // Calcolo delle dimensioni capsule e delle distanze reali
    //    (per evitare falsi "contatti" dovuti ai centri attore)
    float MyRad = 0.f, MyHalf = 0.f, TarRad = 0.f, TarHalf = 0.f;

    // Dimensioni capsula del nemico (questo AI)
    if (UCapsuleComponent* MyCap = MyPawn->FindComponentByClass<UCapsuleComponent>())
        MyCap->GetScaledCapsuleSize(MyRad, MyHalf);

    // Dimensioni capsula del bersaglio (se è un Pawn con capsula)
    if (APawn* TPawn = Cast<APawn>(TargetActor.Get()))
    {
        if (UCapsuleComponent* TCap = TPawn->FindComponentByClass<UCapsuleComponent>())
            TCap->GetScaledCapsuleSize(TarRad, TarHalf);
    }

    // Posizioni attuali
    const FVector MyLoc = MyPawn->GetActorLocation();
    const FVector TarLoc = TargetActor->GetActorLocation();

    // Distanza centro-centro tra i due attori (semplice)
    const float CenterDist = FVector::Dist(MyLoc, TarLoc);

    // Distanza bordo-bordo: togli i raggi delle capsule dalla distanza centro-centro
    // (clamp a 0 per evitare valori negativi se le capsule si intersecano)
    const float EdgeDist = FMath::Max(0.f, CenterDist - (MyRad + TarRad));

    // Portata d’attacco nominale: se esiste un Enemy associato, usa la sua; altrimenti fallback
    const float MeleeRange = (Enemy ? Enemy->AttackRange : 200.f);

    // Raggio di accettazione per MoveTo verso il bersaglio:
    // si parte dalla portata d’attacco ma si corregge con le capsule per evitare oscillazioni
    const float AcceptRadius = FMath::Max(5.f, MeleeRange + TarRad - MyRad);

    // Verifica della linea di vista (LOS) verso il bersaglio
    const bool bHasLOS = LineOfSightTo(TargetActor.Get());

    // Verifica se la distanza bordo-bordo è entro la portata d’attacco
    const bool bCloseEnough = (EdgeDist <= MeleeRange);

    // Log diagnostico: utile per il tuning in editor
    UE_LOG(LogTemp, Verbose, TEXT("[AI] Dist: center=%.1f edge=%.1f  accept=%.1f  myR=%.1f tarR=%.1f  LOS=%d"),
        CenterDist, EdgeDist, AcceptRadius, MyRad, TarRad, bHasLOS ? 1 : 0);


    // Se abbiamo LOS e siamo abbastanza vicini: interrompi l’inseguimento e applica l’effetto di "presa" (qui: Game Over sul giocatore)

    if (bHasLOS && bCloseEnough)
    {
        // Ferma il path-following
        StopMovement();

        // Imposta il focus sul bersaglio (usato da AI per orientamento/animazioni)
        SetFocus(TargetActor.Get());

        // Se il bersaglio è il personaggio del giocatore, gestisci il Game Over
        if (AModulo_1_consegaCharacter* PlayerCharacter = Cast<AModulo_1_consegaCharacter>(TargetActor.Get()))
        {
            // Evita di invocare Game Over più volte
            if (!bHasTriggeredGameOver)
            {
                bHasTriggeredGameOver = true;

                // Mostra UI e pausa gioco
                PlayerCharacter->ShowGameOver();

                // (Facoltativo) Elimina il nemico dopo l’attacco per “chiudere” l’azione
                if (Enemy)
                {
                    Enemy->Destroy();
                }
            }
        }
        return; // Fine: abbiamo eseguito l’azione d’attacco
    }


    // Altrimenti continua/avvia l’inseguimento verso il bersaglio usando l’AcceptRadius calcolato (per evitare micro-oscillazioni)

    const EPathFollowingRequestResult::Type Res =
        MoveToActor(TargetActor.Get(), AcceptRadius, false, true, true, nullptr, true);

    // Se il path fallisce (es. navmesh bloccata o target irraggiungibile),
    // memorizza la posizione attuale del bersaglio e passa a Investigate
    if (Res == EPathFollowingRequestResult::Failed)
    {
        InvestigateLocation = TarLoc;
        TargetActor.Reset();
        SetState(EEnemyState::Investigate);
    }
}

// Seleziona e raggiunge il prossimo punto di pattuglia (ciclo)
void AEnemyAIController::GoToNextPatrolPoint()
{
    if (!Enemy || Enemy->PatrolPoints.Num() == 0) return;

    PatrolIndex = (PatrolIndex + 1) % Enemy->PatrolPoints.Num();
    if (AActor* Next = Enemy->PatrolPoints[PatrolIndex])
    {
        MoveToActor(Next, 80.f, true);
    }
}

// Avvia un timer di attesa; allo scadere verrà chiamato EndWait
void AEnemyAIController::StartWait(float Seconds)
{
    GetWorld()->GetTimerManager().ClearTimer(WaitHandle);
    GetWorld()->GetTimerManager().SetTimer(WaitHandle, this, &AEnemyAIController::EndWait, Seconds, false);
}


// Fine attesa: se in Waiting o Investigate, torna in Patrol
void AEnemyAIController::EndWait()
{
    if (State == EEnemyState::Waiting || State == EEnemyState::Investigate)
    {
        SetState(EEnemyState::Patrol);
    }
}


// Utility: verifica se il bersaglio è valido e in linea di vista
bool AEnemyAIController::HasValidTargetLOS() const
{
    return TargetActor.IsValid() && LineOfSightTo(TargetActor.Get());
}


// Utility: verifica distanza di attacco rispetto al bersaglio
bool AEnemyAIController::InAttackRange() const
{
    if (!TargetActor.IsValid() || !GetPawn()) return false;
    return FVector::Dist(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation()) <= 200.f;
}