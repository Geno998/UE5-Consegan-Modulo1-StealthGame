#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class AAIController;
class AActor;


// Stati possibili del nemico (gestiti dall’AI Controller)
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
    Waiting      UMETA(DisplayName = "Waiting"),     // In attesa/fermo
    Patrol       UMETA(DisplayName = "Patrol"),      // In pattuglia
    Investigate  UMETA(DisplayName = "Investigate"), // Sta indagando un rumore/posizione
    Attack       UMETA(DisplayName = "Attack")       // Inseguimento/attacco al giocatore
};

// Classe base per il personaggio nemico
// Gestisce stati, velocità e parametri di pattuglia/attacco
UCLASS()
class MODULO_1_CONSEGA_API AEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Costruttore: imposta valori iniziali del nemico
    AEnemyCharacter();

    // ---- Configurazione per la pattuglia ----
    // Lista di punti (Actor) che il nemico percorre in sequenza
    UPROPERTY(EditInstanceOnly, Category = "AI|Patrol")
    TArray<AActor*> PatrolPoints;

    // Tempo di attesa (in secondi) a ogni waypoint
    UPROPERTY(EditAnywhere, Category = "AI|Patrol")
    float WaitAtWaypoint = 2.0f;

    // Raggio d’attacco in combattimento corpo a corpo
    UPROPERTY(EditAnywhere, Category = "AI|Combat")
    float AttackRange = 200.f;

    // ---- Velocità di movimento per ciascuno stato (cm/s) ----
    UPROPERTY(EditAnywhere, Category = "AI|Movement")
    float Speed_Waiting = 0.f;      // In attesa: fermo

    UPROPERTY(EditAnywhere, Category = "AI|Movement")
    float Speed_Patrol = 200.f;     // Durante pattuglia

    UPROPERTY(EditAnywhere, Category = "AI|Movement")
    float Speed_Investigate = 300.f;// Durante investigazione

    UPROPERTY(EditAnywhere, Category = "AI|Movement")
    float Speed_Attack = 600.f;     // Durante inseguimento/attacco

    // Getter e setter dello stato attuale
    EEnemyState GetState() const { return CurrentState; }
    void        SetState(EEnemyState NewState) { CurrentState = NewState; }

    // Applica la velocità in base allo stato passato
    void ApplySpeedForState(EEnemyState NewState);

    // Verifica se il nemico può subire una "presa/uccisione silenziosa"
    bool CanBeTakedowned() const { return CurrentState != EEnemyState::Attack; }

protected:
    // Funzione chiamata all’avvio del gioco
    virtual void BeginPlay() override;

private:
    // Stato corrente del nemico
    EEnemyState CurrentState = EEnemyState::Waiting;
};
