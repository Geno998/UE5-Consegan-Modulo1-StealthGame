#include "VictoryPlatform.h"
#include "Components/BoxComponent.h"
#include "Modulo_1_consegaCharacter.h"


// Costruttore: configura il Box di collisione che rileva il giocatore
AVictoryPlatform::AVictoryPlatform()
{
    // Questa piattaforma non ha logica di Tick
    PrimaryActorTick.bCanEverTick = false;

    // Crea e imposta il Box come RootComponent
    Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
    RootComponent = Box;

    // Dimensioni del box (X,Y,Z)
    Box->SetBoxExtent(FVector(100.f, 100.f, 50.f));

    // Configurazione collisioni:
    Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);   // solo query, nessuna fisica
    Box->SetCollisionObjectType(ECC_WorldStatic);             // attore statico
    Box->SetCollisionResponseToAllChannels(ECR_Ignore);       // ignora tutto
    Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);// solo i Pawn generano overlap
    Box->SetGenerateOverlapEvents(true);                      // abilita eventi di overlap

    // Collega la funzione OnOverlapBegin all’evento di overlap
    Box->OnComponentBeginOverlap.AddDynamic(this, &AVictoryPlatform::OnOverlapBegin);
}


// Callback di overlap: viene chiamata quando un Pawn entra nel Box
//Se è il giocatore, mostra la schermata di vittoria (una sola volta)
void AVictoryPlatform::OnOverlapBegin(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    // Se la vittoria è già stata attivata, non fare nulla
    if (bTriggered) return;

    // Controlla se l’attore che ha fatto overlap è il personaggio del giocatore
    if (AModulo_1_consegaCharacter* Player = Cast<AModulo_1_consegaCharacter>(OtherActor))
    {
        bTriggered = true;      // blocca futuri trigger
        Player->ShowVictory();  // mostra UI di vittoria
    }
}
