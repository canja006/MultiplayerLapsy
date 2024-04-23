// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

// Konstruktori m��rittelee delegatejen luomisen
UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() :
    CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
    FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
    JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
    DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
    StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
}

// Funktio luo uuden istunnon
void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
    // Tarkistetaan, onko istunto kelvollinen
    if (!IsValidSessionInterface())
    {
        return;
    }

    // Tarkistetaan, onko olemassa jo istunto
    auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (ExistingSession != nullptr)
    {
        // Tallennetaan tiedot, jotta istunto voidaan luoda uudelleen istunnon tuhoutuessa
        bCreateSessionOnDestroy = true;
        LastNumPublicConnections = NumPublicConnections;
        LastMatchType = MatchType;

        // Tuhotaan olemassaoleva istunto
        DestroySession();
    }

    // Lis�t��n delegate istunnon luomisen valmistumisen k�sittely� varten
    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

    // M��ritell��n istunnon asetukset
    LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
    LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
    // Lis�� muita istunnon asetuksia...
}

// Funktio etsii istuntoja
void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
    // Tarkistetaan, onko istunto-ojain kelvollinen
    if (!IsValidSessionInterface())
    {
        return;
    }

    // Lis�t��n delegate istuntojen etsinn�n valmistumisen k�sittely� varten
    FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

    // M��ritell��n etsitt�v�n istunnon hakuehdot
    LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
    LastSessionSearch->MaxSearchResults = MaxSearchResults;
    // Lis�� muita hakuehtoja...
}

// Funktio liittyy olemassaolevaan istuntoon
void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
    // Tarkistetaan, onko istunto-ojain kelvollinen
    if (!SessionInterface.IsValid())
    {
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
        return;
    }

    // Lis�t��n delegate istuntoon liittymisen valmistumisen k�sittely� varten
    JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

    // Liityt��n istuntoon
    // Lis�� liittymisen logiikka...
}

// Funktio tuhoaa istunnon
void UMultiplayerSessionsSubsystem::DestroySession()
{
    // Tarkistetaan, onko istunto-ojain kelvollinen
    if (!SessionInterface.IsValid())
    {
        MultiplayerOnDestroySessionComplete.Broadcast(false);
        return;
    }

    // Lis�t��n delegate istunnon tuhoamisen valmistumisen k�sittely� varten
    DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

    // Tuhotaan istunto
}

// Funktio tarkistaa, onko istunto-ojain kelvollinen
bool UMultiplayerSessionsSubsystem::IsValidSessionInterface()
{
}
