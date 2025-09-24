#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "GameOverWidget.h"
#include "Blueprint/UserWidget.h"
#include "InputActionValue.h"
#include "Modulo_1_consegaCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

class UFootstepComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config = Game)
class AModulo_1_consegaCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UFootstepComponent* Footstep;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	// Azione: Crouch (accovacciarsi)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	// Azione: Takedown (assegnare ad esempio al tasto E)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* TakedownAction;


public:
	// Costruttore: inizializza sottocomponenti e parametri
	AModulo_1_consegaCharacter();


protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// Distanza massima (cm) per considerare valido un takedown (edge-to-edge)
	UPROPERTY(EditAnywhere, Category = "Takedown")
	float TakedownRange = 180.f;

	// Semi-angolo in gradi del cono frontale (impostare 180 per disabilitare il controllo dell’angolo)
	UPROPERTY(EditAnywhere, Category = "Takedown")
	float TakedownHalfAngleDeg = 60.f;

	// ---- UI ----

	// Classe del widget Game Over
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	// Classe del widget Vittoria
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> VictoryWidgetClass;

	// Mostra UI di vittoria (esposto a Blueprint)
	UFUNCTION(BlueprintCallable)
	void ShowVictory();

	// Mostra UI di Game Over
	void ShowGameOver();

private:
	// Inizio/Fine crouch da input
	void StartTheCrouch(const FInputActionValue& Value);
	void StopTheCrouch(const FInputActionValue& Value);

	// Prova ad eseguire un takedown sul bersaglio davanti al player
	void TryTakedown(const FInputActionValue& Value);
};


