// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

// Alustaa valikon ja asettaa tarvittavat parametrit
void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
    // Asettaa polun lobbyyn ja määrittää istunnon asetukset
    PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
    NumPublicConnections = NumberOfPublicConnections;
    MatchType = TypeOfMatch;

    // Lisää valikon näkyvyyden ja asettaa sen interaktiiviseksi
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    // Asettaa pelaajanohjaimen syöttötilan ja hiiren näkyvyyden
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeUIOnly InputModeData;
            InputModeData.SetWidgetToFocus(TakeWidget());
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(true);
        }
    }

    // Hakee peli-instanssin ja moninpelisessiot alijärjestelmästä
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
        MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
    }

    // Lisää tapahtumakäsittelijät moninpelisessioille
    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
        MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
        MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
        MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
        MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
    }
}

// Alustaa valikon
bool UMenu::Initialize()
{
    // Kutsuu yläluokan alustusmetodia
    if (!Super::Initialize())
    {
        return false;
    }

    // Lisää painikkeiden klikkaustapahtumakäsittelijät
    if (HostButton)
    {
        HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
    }
    if (JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
    }

    return true;
}

// Tuhoaa valikon
void UMenu::NativeDestruct()
{
    // Purkaa valikon ja kutsuu yläluokan tuhoamismetodia
    MenuTearDown();
    Super::NativeDestruct();
}

// Käsittelee istunnon luonnin valmistumistapahtumaa
void UMenu::OnCreateSession(bool bWasSuccessful)
{
    // Jos istunnon luonti onnistui, siirtyy lobbyyn, muuten näyttää virhesanoman ja aktivoi host-painikkeen
    if (bWasSuccessful)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            World->ServerTravel(PathToLobby);
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Red,
                FString(TEXT("Failed to create session!"))
            );
        }
        HostButton->SetIsEnabled(true);
    }
}

// Käsittelee istuntojen etsinnän valmistumistapahtumaa
void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
    // Jos moninpelisessiot alijärjestelmä on olemassa, tarkistaa löytyykö haluttua istuntoa, muuten aktivoi liity-painikkeen
    if (MultiplayerSessionsSubsystem == nullptr)
    {
        return;
    }

    for (auto Result : SessionResults)
    {
        FString SettingsValue;
        Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
        if (SettingsValue == MatchType)
        {
            MultiplayerSessionsSubsystem->JoinSession(Result);
            return;
        }
    }
    if (!bWasSuccessful || SessionResults.Num() == 0)
    {
        JoinButton->SetIsEnabled(true);
    }
}

// Käsittelee istuntoon liittymisen valmistumistapahtumaa
void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
    // Hakee istunto-ojaimen ja liittyy istuntoon, jos mahdollista
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            FString Address;
            SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

            APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
            if (PlayerController)
            {
                PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
            }
        }
    }
}

// Käsittelee istunnon tuhoamisen valmistumistapahtumaa
void UMenu::OnDestroySession(bool bWasSuccessful)
{
    // Ei toteutettu
}

// Käsittelee istunnon aloituksen valmistumistapahtumaa
void UMenu::OnStartSession(bool bWasSuccessful)
{
    // Ei toteutettu
}

// Käsittelee host-painikkeen klikkaustapahtumaa
void UMenu::HostButtonClicked()
{
    // Poistaa host-painikkeen käytöstä ja pyytää moninpelisessiot alijärjestelmää luomaan istunnon
    HostButton->SetIsEnabled(false);
    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
    }
}

// Käsittelee liity-painikkeen klikkaustapahtumaa
void UMenu::JoinButtonClicked()
{
    // Poistaa liity-painikkeen käytöstä ja pyytää moninpelisessiot alijärjestelmää etsimään istuntoja
    JoinButton->SetIsEnabled(false);
    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->FindSessions(10000);
    }
}

// Puree valikon ja asettaa pelaajanohjaimen takaisin pelitilaan
void UMenu::MenuTearDown()
{
    // Poistaa valikon näytöltä ja palauttaa pelaajanohjaimen pelitilaan
    RemoveFromParent();
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeGameOnly InputModeData;
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(false);
        }
    }
}
