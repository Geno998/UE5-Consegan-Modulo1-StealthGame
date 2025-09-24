#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Enemycharacter.h"
#include "EnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class AEnemyCharacter;

UCLASS()
class MODULO_1_CONSEGA_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()
public:
    // Costruttore: inizializza il controller e i componenti di percezione
    AEnemyAIController();

    // Funzione chiamata quando il controller prende possesso di un Pawn
    virtual void OnPossess(APawn* InPawn) override;

    // Tick eseguito ogni frame per gestire i vari stati dell’IA
    virtual void Tick(float DeltaSeconds) override;

protected:
    // Componente di percezione che gestisce vista e udito
    UPROPERTY(VisibleAnywhere, Category = "AI")
    UAIPerceptionComponent* Perception = nullptr;

    // Configurazione per la vista
    UPROPERTY()
    UAISenseConfig_Sight* SightConfig = nullptr;

    // Configurazione per l’udito
    UPROPERTY()
    UAISenseConfig_Hearing* HearingConfig = nullptr;

    // Funzione chiamata quando l’IA aggiorna la percezione (vista/udito)
    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Funzione chiamata al termine di un movimento
    virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:
    // Riferimento al nemico posseduto da questo controller
    UPROPERTY()
    AEnemyCharacter* Enemy = nullptr;

    // Stato corrente dell’IA (Attesa, Pattuglia, Indagine, Attacco)
    EEnemyState State = EEnemyState::Waiting;

    // Attore bersaglio rilevato dall’IA
    TWeakObjectPtr<AActor> TargetActor;

    // Ultima posizione sospetta da investigare
    FVector InvestigateLocation = FVector::ZeroVector;

    // Indice usato per i punti di pattuglia
    int32 PatrolIndex = 0;

    // Gestore del timer per il comportamento di attesa
    FTimerHandle WaitHandle;

    // Gestore del timer per la perdita della linea visiva
    FTimerHandle LoseSightTimer;

    // Tempo di tolleranza prima di perdere definitivamente di vista il bersaglio
    float LoseSightGrace = 2.0f;

    // Flag per evitare che il Game Over venga attivato più volte
    bool bHasTriggeredGameOver = false;

private:
    // Cambia lo stato dell’IA e avvia la logica corrispondente
    void SetState(EEnemyState NewState);

    // Logica di comportamento in attesa
    void DoWaiting();

    // Logica di comportamento in pattuglia
    void DoPatrol();

    // Logica di comportamento in investigazione
    void DoInvestigate();

    // Logica di comportamento in attacco
    void DoAttack();

    // Passa al prossimo punto di pattuglia
    void GoToNextPatrolPoint();

    // Avvia un periodo di attesa (in secondi)
    void StartWait(float Seconds);

    // Fine del periodo di attesa → ritorna alla pattuglia
    void EndWait();

    // Verifica se il bersaglio è ancora visibile
    bool HasValidTargetLOS() const;

    // Verifica se il bersaglio si trova a distanza di attacco
    bool InAttackRange() const;
};
