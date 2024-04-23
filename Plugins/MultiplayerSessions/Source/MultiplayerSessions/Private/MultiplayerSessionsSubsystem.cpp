// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

// Konstruktori määrittelee delegatejen luomisen
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

    // Lisätään delegate istunnon luomisen valmistumisen käsittelyä varten
    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

    // Määritellään istunnon asetukset
    LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
    LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
    // Lisää muita istunnon asetuksia...
}

// Funktio etsii istuntoja
void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
    // Tarkistetaan, onko istunto-ojain kelvollinen
    if (!IsValidSessionInterface())
    {
        return;
    }

    // Lisätään delegate istuntojen etsinnän valmistumisen käsittelyä varten
    FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

    // Määritellään etsittävän istunnon hakuehdot
    LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
    LastSessionSearch->MaxSearchResults = MaxSearchResults;
    // Lisää muita hakuehtoja...
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

    // Lisätään delegate istuntoon liittymisen valmistumisen käsittelyä varten
    JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

    // Liitytään istuntoon
    // Lisää liittymisen logiikka...
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

    // Lisätään delegate istunnon tuhoamisen valmistumisen käsittelyä varten
    DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

    // Tuhotaan istunto
}

// Funktio tarkistaa, onko istunto-ojain kelvollinen
bool UMultiplayerSessionsSubsystem::IsValidSessionInterface()
{
}
