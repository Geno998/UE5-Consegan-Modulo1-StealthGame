#include "GameOverWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


// Costruttore del widget
void UGameOverWidget::NativeConstruct()
{
    Super::NativeConstruct();

    //collega la funzione al click del pulsante Restart 
    if (RestartButton)
    {
        RestartButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnRestartClicked);
    }

    //collega la funzione al click del pulsante Quit
    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnQuitClicked);
    }
}


// Callback del pulsante Restart
void UGameOverWidget::OnRestartClicked()
{
    if (UWorld* World = GetWorld())
    {
        // Togli la pausa dal gioco
        UGameplayStatics::SetGamePaused(World, false);

        // Ripristina il controllo al giocatore
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            FInputModeGameOnly GameOnly;
            PC->SetInputMode(GameOnly);
            PC->bShowMouseCursor = false;
            PC->bEnableClickEvents = false;
            PC->bEnableMouseOverEvents = false;
        }

        // Rimuovi il widget dalla viewport
        RemoveFromParent();

        // Ricarica il livello attuale
        const FName LevelName = FName(*World->GetMapName());
        UGameplayStatics::OpenLevel(World, LevelName);
    }
}


// Callback del pulsante Quit
// - Chiude il gioco chiamando QuitGame
void UGameOverWidget::OnQuitClicked()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            UKismetSystemLibrary::QuitGame(World, PC, EQuitPreference::Quit, true);
        }
    }
}
