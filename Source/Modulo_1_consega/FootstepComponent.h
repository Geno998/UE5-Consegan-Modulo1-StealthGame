#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FootstepComponent.generated.h"

class USoundBase;

// Componente che gestisce i passi del personaggio:
// - Riproduce suoni a intervalli regolari durante il movimento
// - Genera stimoli uditivi percepibili dall’IA
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class MODULO_1_CONSEGA_API UFootstepComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Costruttore: inizializza i valori di default
    UFootstepComponent();

    // Suono di default per il passo
    UPROPERTY(EditAnywhere, Category = "Footstep|Audio")
    USoundBase* DefaultFootstep = nullptr;

    // Intervallo (secondi) tra un passo e l’altro durante il movimento
    UPROPERTY(EditAnywhere, Category = "Footstep|Timing")
    float StepInterval = 0.45f;

    // Velocità minima orizzontale (cm/s) per emettere suoni di passi
    UPROPERTY(EditAnywhere, Category = "Footstep|Timing")
    float MinSpeedToStep = 150.f;

    // Se true, nessun suono o rumore viene emesso mentre il personaggio è accovacciato
    UPROPERTY(EditAnywhere, Category = "Footstep|Logic")
    bool bSilentWhenCrouched = true;

    // Intensità base dello stimolo uditivo (viene scalata fino a MaxLoudness in base alla velocità)
    UPROPERTY(EditAnywhere, Category = "Footstep|AI")
    float BaseLoudness = 0.4f;

    // Intensità massima dello stimolo uditivo
    UPROPERTY(EditAnywhere, Category = "Footstep|AI")
    float MaxLoudness = 1.0f;

    // Tag associato all’evento di rumore (es. "Footstep")
    UPROPERTY(EditAnywhere, Category = "Footstep|AI")
    FName HearingTag = TEXT("Footstep");

protected:
    // Funzione chiamata all’avvio del gioco
    virtual void BeginPlay() override;

public:
    // Tick del componente: controlla velocità e intervallo dei passi
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Contatore del tempo trascorso dall’ultimo passo
    float TimeSinceLastStep = 0.f;
};
