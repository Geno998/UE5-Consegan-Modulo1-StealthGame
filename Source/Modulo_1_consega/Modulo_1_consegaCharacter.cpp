
#include "Modulo_1_consegaCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "FootstepComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "EnemyCharacter.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AModulo_1_consegaCharacter

AModulo_1_consegaCharacter::AModulo_1_consegaCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->CrouchedHalfHeight = 65.0f;

	Footstep = CreateDefaultSubobject<UFootstepComponent>(TEXT("Footstep"));

	GameOverWidgetClass = UGameOverWidget::StaticClass();
}

void AModulo_1_consegaCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AModulo_1_consegaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// If you call Super here, do it once at the top
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		// Moving
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AModulo_1_consegaCharacter::Move);
		}

		// Looking
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AModulo_1_consegaCharacter::Look);
		}

		// Crouch
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AModulo_1_consegaCharacter::StartTheCrouch);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AModulo_1_consegaCharacter::StopTheCrouch);
		}

		// Takedown
		if (TakedownAction)
		{
			EnhancedInputComponent->BindAction(TakedownAction, ETriggerEvent::Started, this, &AModulo_1_consegaCharacter::TryTakedown);
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AModulo_1_consegaCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AModulo_1_consegaCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// Inizio crouch: abbassa il personaggio e riduce la velocità
void AModulo_1_consegaCharacter::StartTheCrouch(const FInputActionValue& Value)
{
	Crouch();

	GetCharacterMovement()->MaxWalkSpeed = 150.f;
}

// Fine crouch: rialza il personaggio e ripristina la velocità
void AModulo_1_consegaCharacter::StopTheCrouch(const FInputActionValue& Value)
{
	UnCrouch();

	GetCharacterMovement()->MaxWalkSpeed = 500.f;
}

// Mostra schermata di Game Over (blocca input e mette in pausa)
void AModulo_1_consegaCharacter::ShowGameOver()
{
	if (UGameplayStatics::IsGamePaused(GetWorld())) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !GameOverWidgetClass) return;

	UUserWidget* Widget = CreateWidget<UUserWidget>(PC, GameOverWidgetClass);
	if (!Widget) return;

	Widget->AddToViewport(100);

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(Widget->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(Mode);
	PC->bShowMouseCursor = true;

	UGameplayStatics::SetGamePaused(GetWorld(), true);
}


// Mostra schermata di Game Over (blocca input e mette in pausa)
void AModulo_1_consegaCharacter::ShowVictory()
{
	if (UGameplayStatics::IsGamePaused(GetWorld())) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !VictoryWidgetClass) return;

	UUserWidget* Widget = CreateWidget<UUserWidget>(PC, VictoryWidgetClass);
	if (!Widget) return;

	Widget->AddToViewport(100);

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(Widget->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(Mode);
	PC->bShowMouseCursor = true;
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;

	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

// Takedown stealth:
// - Trova nemici vicini entro raggio
// - Controlla angolo frontale e stato del nemico
// - Se valido, elimina immediatamente il bersaglio
void AModulo_1_consegaCharacter::TryTakedown(const FInputActionValue& /*Value*/)
{
	// Recupera il mondo di gioco
	UWorld* World = GetWorld();
	if (!World) return;

	// Posizione e direzione in avanti del giocatore
	const FVector MyLoc = GetActorLocation();
	const FVector Forward = GetActorForwardVector();

	// Definizione della ricerca
	// Creiamo un array di "tipi di oggetti" da cercare: qui solo Pawn
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	// Lista di attori da ignorare (non vogliamo colpire noi stessi)
	TArray<AActor*> Ignore;
	Ignore.Add(this);

	// Lista che conterrà i risultati (nemici trovati)
	TArray<AActor*> Hits;

	// Raggio di ricerca preso dalla variabile TakedownRange
	const float Radius = TakedownRange;

	// Ricerca degli attori entro il raggio
	const bool bFound = UKismetSystemLibrary::SphereOverlapActors(
		World, MyLoc, Radius, ObjTypes, AEnemyCharacter::StaticClass(), Ignore, Hits);

	// Se non è stato trovato nessun attore, esci
	if (!bFound) return;

	// Preparazione per selezionare il miglior bersaglio
	AEnemyCharacter* BestTarget = nullptr;
	float BestDot = -1.f; // memorizza l’allineamento migliore (Dot product più alto)

	// Calcoliamo il coseno dell’angolo di mezzo cono
	// Serve per confrontare l’angolo tra "Forward" e direzione verso il nemico.
	const float CosHalf = FMath::Cos(
		FMath::DegreesToRadians(
			FMath::Clamp(TakedownHalfAngleDeg, 0.f, 179.f)
		)
	);

	// Ciclo sui nemici trovati
	for (AActor* A : Hits)
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(A);
		if (!Enemy) continue; // se non è un EnemyCharacter, salta

		// Nemico vulnerabile solo se non in stato "Attack"
		if (!Enemy->CanBeTakedowned()) continue;

		// Calcola direzione normalizzata dal player al nemico
		const FVector ToEnemy = (Enemy->GetActorLocation() - MyLoc).GetSafeNormal();

		// misura quanto il nemico è "davanti" al giocatore
		const float Dot = FVector::DotProduct(Forward, ToEnemy);

		// Deve essere dentro il cono frontale e meglio allineato del candidato attuale
		if (Dot >= CosHalf && Dot > BestDot)
		{
			BestDot = Dot;
			BestTarget = Enemy;
		}
	}

	// Se non abbiamo trovato nessun nemico valido, esci
	if (!BestTarget) return;

	// Esecuzione del takedown
	// Qui semplicemente distruggiamo l’attore nemico,
	// ma si potrebbe sostituire con un’animazione o una sequenza più complessa.
	BestTarget->Destroy();

	// Debug visivo (solo in editor / build non shipping)
#if !(UE_BUILD_SHIPPING)
	// Disegna una sfera verde che mostra il raggio di ricerca
	DrawDebugSphere(World, MyLoc, Radius, 16, FColor::Green, false, 1.0f);

	// Disegna una linea verde dal player al nemico colpito
	DrawDebugLine(World, MyLoc,
		BestTarget ? BestTarget->GetActorLocation() : MyLoc,
		FColor::Green, false, 1.0f, 0, 2.f);
#endif
}