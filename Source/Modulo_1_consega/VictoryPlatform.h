#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VictoryPlatform.generated.h"

class UBoxComponent;

// Attore piattaforma di vittoria
// Mostra la schermata di vittoria quando il giocatore ci sale sopra.
// Funziona tramite un BoxComponent che rileva l’overlap.

UCLASS()
class MODULO_1_CONSEGA_API AVictoryPlatform : public AActor
{
    GENERATED_BODY()

public:

    // Costruttore: inizializza i componenti
    AVictoryPlatform();

protected:

    // Box di collisione per rilevare l’ingresso del giocatore
    UPROPERTY(VisibleAnywhere)
    UBoxComponent* Box;

    // Callback chiamata quando un attore entra nel box
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    bool bTriggered = false;
};