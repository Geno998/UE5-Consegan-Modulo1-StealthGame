#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

class UButton;


// Widget Game Over
// Mostra un’interfaccia con pulsanti per ricominciare o uscire dal gioco
UCLASS()
class MODULO_1_CONSEGA_API UGameOverWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Costruttore nativo del widget: chiamato quando il widget viene creato
    virtual void NativeConstruct() override;

protected:
    // Pulsante per ricominciare la partita (deve avere lo stesso nome nel BP Designer)
    UPROPERTY(meta = (BindWidget))
    UButton* RestartButton;

    // Pulsante per uscire dal gioco (deve avere lo stesso nome nel BP Designer)
    UPROPERTY(meta = (BindWidget))
    UButton* QuitButton;

    // Callback quando viene premuto il pulsante Restart
    UFUNCTION()
    void OnRestartClicked();

    // Callback quando viene premuto il pulsante Quit
    UFUNCTION()
    void OnQuitClicked();
};
