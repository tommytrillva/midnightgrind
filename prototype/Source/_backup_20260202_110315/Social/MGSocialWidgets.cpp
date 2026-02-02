// Copyright Midnight Grind. All Rights Reserved.

#include "Social/MGSocialWidgets.h"
#include "Social/MGSocialSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// ==========================================
// UMGFriendEntryWidget
// ==========================================

void UMGFriendEntryWidget::SetFriendData(const FMGFriendData& Friend)
{
	FriendData = Friend;
	UpdateDisplay();
}

void UMGFriendEntryWidget::SetSelected(bool bSelected)
{
	bIsSelected = bSelected;
	UpdateDisplay();
}

void UMGFriendEntryWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGFriendEntryWidget::HandleClick()
{
	OnSelected.Broadcast(FriendData);
}

void UMGFriendEntryWidget::HandleJoinClick()
{
	if (FriendData.bCanJoin)
	{
		OnJoinRequested.Broadcast(FriendData);
	}
}

void UMGFriendEntryWidget::HandleInviteClick()
{
	OnInviteRequested.Broadcast(FriendData);
}

// ==========================================
// UMGFriendsListWidget
// ==========================================

void UMGFriendsListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		SocialSubsystem = GI->GetSubsystem<UMGSocialSubsystem>();
		if (SocialSubsystem)
		{
			SocialSubsystem->OnFriendListUpdated.AddDynamic(this, &UMGFriendsListWidget::OnFriendsUpdated);
			RefreshList();
		}
	}
}

void UMGFriendsListWidget::NativeDestruct()
{
	if (SocialSubsystem)
	{
		SocialSubsystem->OnFriendListUpdated.RemoveDynamic(this, &UMGFriendsListWidget::OnFriendsUpdated);
	}
	Super::NativeDestruct();
}

void UMGFriendsListWidget::RefreshList()
{
	if (SocialSubsystem)
	{
		SocialSubsystem->RefreshFriendsList();
	}
}

void UMGFriendsListWidget::SetStatusFilter(bool bOnline)
{
	bOnlineOnly = bOnline;
	RefreshList();
}

void UMGFriendsListWidget::SearchFriends(const FString& Term)
{
	SearchTerm = Term;
	if (SocialSubsystem)
	{
		OnFriendsUpdated(SocialSubsystem->GetFriendsList());
	}
}

void UMGFriendsListWidget::OnFriendsUpdated(const TArray<FMGFriendData>& Friends)
{
	TArray<FMGFriendData> FilteredFriends;

	for (const FMGFriendData& Friend : Friends)
	{
		// Apply online filter
		if (bOnlineOnly && Friend.Status == EMGFriendStatus::Offline)
		{
			continue;
		}

		// Apply search filter
		if (!SearchTerm.IsEmpty() && !Friend.DisplayName.Contains(SearchTerm, ESearchCase::IgnoreCase))
		{
			continue;
		}

		FilteredFriends.Add(Friend);
	}

	// Sort: favorites first, then by status (online first), then alphabetically
	FilteredFriends.Sort([](const FMGFriendData& A, const FMGFriendData& B)
	{
		if (A.bIsFavorite != B.bIsFavorite) return A.bIsFavorite;
		if ((A.Status != EMGFriendStatus::Offline) != (B.Status != EMGFriendStatus::Offline))
			return A.Status != EMGFriendStatus::Offline;
		return A.DisplayName < B.DisplayName;
	});

	UpdateListDisplay(FilteredFriends);
}

void UMGFriendsListWidget::UpdateListDisplay_Implementation(const TArray<FMGFriendData>& Friends)
{
	// Ensure enough widgets
	while (EntryWidgets.Num() < Friends.Num())
	{
		UMGFriendEntryWidget* Widget = CreateEntryWidget();
		if (Widget)
		{
			EntryWidgets.Add(Widget);
		}
	}

	// Update visible widgets
	for (int32 i = 0; i < Friends.Num(); i++)
	{
		if (i < EntryWidgets.Num() && EntryWidgets[i])
		{
			EntryWidgets[i]->SetFriendData(Friends[i]);
			EntryWidgets[i]->SetVisibility(ESlateVisibility::Visible);
		}
	}

	// Hide unused
	for (int32 i = Friends.Num(); i < EntryWidgets.Num(); i++)
	{
		if (EntryWidgets[i])
		{
			EntryWidgets[i]->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

UMGFriendEntryWidget* UMGFriendsListWidget::CreateEntryWidget()
{
	if (!EntryWidgetClass)
	{
		return nullptr;
	}
	return CreateWidget<UMGFriendEntryWidget>(this, EntryWidgetClass);
}

// ==========================================
// UMGFriendRequestWidget
// ==========================================

void UMGFriendRequestWidget::SetRequestData(const FMGFriendRequest& Request)
{
	RequestData = Request;
	UpdateDisplay();
}

void UMGFriendRequestWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGFriendRequestWidget::AcceptRequest()
{
	OnAccepted.Broadcast(RequestData.RequestID);
}

void UMGFriendRequestWidget::DeclineRequest()
{
	OnDeclined.Broadcast(RequestData.RequestID);
}

// ==========================================
// UMGCrewMemberWidget
// ==========================================

void UMGCrewMemberWidget::SetMemberData(const FMGCrewMember& Member, EMGCrewRank InViewerRank)
{
	MemberData = Member;
	ViewerRank = InViewerRank;

	// Determine permissions
	bCanKick = false;
	bCanPromote = false;

	if (ViewerRank == EMGCrewRank::Leader)
	{
		bCanKick = Member.Rank != EMGCrewRank::Leader;
		bCanPromote = Member.Rank == EMGCrewRank::Member;
	}
	else if (ViewerRank == EMGCrewRank::Officer)
	{
		bCanKick = Member.Rank == EMGCrewRank::Member;
	}

	UpdateDisplay();
}

void UMGCrewMemberWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGCrewMemberWidget::KickMember()
{
	if (bCanKick)
	{
		OnKickRequested.Broadcast(MemberData.PlayerID);
	}
}

void UMGCrewMemberWidget::PromoteMember()
{
	if (bCanPromote)
	{
		OnPromoteRequested.Broadcast(MemberData.PlayerID);
	}
}

// ==========================================
// UMGCrewPanelWidget
// ==========================================

void UMGCrewPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		SocialSubsystem = GI->GetSubsystem<UMGSocialSubsystem>();
		if (SocialSubsystem)
		{
			SocialSubsystem->OnCrewDataUpdated.AddDynamic(this, &UMGCrewPanelWidget::OnCrewUpdated);

			if (SocialSubsystem->IsInCrew())
			{
				SetCrewData(SocialSubsystem->GetCurrentCrew(), SocialSubsystem->GetCrewRank());
			}
		}
	}
}

void UMGCrewPanelWidget::NativeDestruct()
{
	if (SocialSubsystem)
	{
		SocialSubsystem->OnCrewDataUpdated.RemoveDynamic(this, &UMGCrewPanelWidget::OnCrewUpdated);
	}
	Super::NativeDestruct();
}

void UMGCrewPanelWidget::SetCrewData(const FMGCrewData& Crew, EMGCrewRank InPlayerRank)
{
	CrewData = Crew;
	PlayerRank = InPlayerRank;
	UpdateCrewDisplay();
	UpdateMembersDisplay();
}

void UMGCrewPanelWidget::RefreshCrew()
{
	if (SocialSubsystem && SocialSubsystem->IsInCrew())
	{
		SetCrewData(SocialSubsystem->GetCurrentCrew(), SocialSubsystem->GetCrewRank());
	}
}

void UMGCrewPanelWidget::OnCrewUpdated(const FMGCrewData& Crew)
{
	if (SocialSubsystem)
	{
		SetCrewData(Crew, SocialSubsystem->GetCrewRank());
	}
}

void UMGCrewPanelWidget::UpdateCrewDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGCrewPanelWidget::UpdateMembersDisplay_Implementation()
{
	// Ensure enough widgets
	while (MemberWidgets.Num() < CrewData.Members.Num())
	{
		UMGCrewMemberWidget* Widget = CreateMemberWidget();
		if (Widget)
		{
			MemberWidgets.Add(Widget);
		}
	}

	// Update visible widgets
	for (int32 i = 0; i < CrewData.Members.Num(); i++)
	{
		if (i < MemberWidgets.Num() && MemberWidgets[i])
		{
			MemberWidgets[i]->SetMemberData(CrewData.Members[i], PlayerRank);
			MemberWidgets[i]->SetVisibility(ESlateVisibility::Visible);
		}
	}

	// Hide unused
	for (int32 i = CrewData.Members.Num(); i < MemberWidgets.Num(); i++)
	{
		if (MemberWidgets[i])
		{
			MemberWidgets[i]->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

UMGCrewMemberWidget* UMGCrewPanelWidget::CreateMemberWidget()
{
	if (!MemberWidgetClass)
	{
		return nullptr;
	}
	return CreateWidget<UMGCrewMemberWidget>(this, MemberWidgetClass);
}

// ==========================================
// UMGCreateCrewWidget
// ==========================================

bool UMGCreateCrewWidget::IsInputValid() const
{
	if (CrewName.Len() < 3 || CrewName.Len() > 24)
	{
		return false;
	}
	if (CrewTag.Len() < 2 || CrewTag.Len() > 4)
	{
		return false;
	}
	return true;
}

FText UMGCreateCrewWidget::GetValidationError() const
{
	if (CrewName.Len() < 3)
	{
		return FText::FromString(TEXT("Crew name must be at least 3 characters"));
	}
	if (CrewName.Len() > 24)
	{
		return FText::FromString(TEXT("Crew name must be 24 characters or less"));
	}
	if (CrewTag.Len() < 2)
	{
		return FText::FromString(TEXT("Crew tag must be at least 2 characters"));
	}
	if (CrewTag.Len() > 4)
	{
		return FText::FromString(TEXT("Crew tag must be 4 characters or less"));
	}
	return FText::GetEmpty();
}

void UMGCreateCrewWidget::CreateCrew()
{
	if (!IsInputValid())
	{
		return;
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UMGSocialSubsystem* Social = GI->GetSubsystem<UMGSocialSubsystem>())
		{
			Social->CreateCrew(CrewName, CrewTag, Description);
			OnCrewCreated.Broadcast();
		}
	}
}

void UMGCreateCrewWidget::Cancel()
{
	OnCancelled.Broadcast();
}

// ==========================================
// UMGRecentPlayersWidget
// ==========================================

void UMGRecentPlayersWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		SocialSubsystem = GI->GetSubsystem<UMGSocialSubsystem>();
		RefreshList();
	}
}

void UMGRecentPlayersWidget::RefreshList()
{
	if (SocialSubsystem)
	{
		UpdateDisplay(SocialSubsystem->GetRecentPlayers());
	}
}

void UMGRecentPlayersWidget::UpdateDisplay_Implementation(const TArray<FMGRecentPlayer>& Players)
{
	// Blueprint implementation
}

void UMGRecentPlayersWidget::AddFriendFromRecent(const FString& PlayerID)
{
	if (SocialSubsystem)
	{
		SocialSubsystem->SendFriendRequest(PlayerID);
	}
}

void UMGRecentPlayersWidget::BlockPlayerFromRecent(const FString& PlayerID)
{
	if (SocialSubsystem)
	{
		SocialSubsystem->BlockPlayer(PlayerID);
		RefreshList();
	}
}

// ==========================================
// UMGGameInviteWidget
// ==========================================

void UMGGameInviteWidget::ShowInvite(const FMGFriendData& FromFriend, const FString& InSessionID)
{
	InviterData = FromFriend;
	SessionID = InSessionID;
	bIsShowing = true;

	UpdateDisplay();
	SetVisibility(ESlateVisibility::Visible);

	// Auto-hide after 30 seconds
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			AutoHideTimer,
			this,
			&UMGGameInviteWidget::OnAutoHide,
			30.0f,
			false
		);
	}
}

void UMGGameInviteWidget::HideInvite()
{
	bIsShowing = false;
	SetVisibility(ESlateVisibility::Hidden);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoHideTimer);
	}
}

void UMGGameInviteWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGGameInviteWidget::AcceptInvite()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UMGSocialSubsystem* Social = GI->GetSubsystem<UMGSocialSubsystem>())
		{
			Social->AcceptGameInvite(SessionID);
		}
	}
	HideInvite();
}

void UMGGameInviteWidget::DeclineInvite()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UMGSocialSubsystem* Social = GI->GetSubsystem<UMGSocialSubsystem>())
		{
			Social->DeclineGameInvite(SessionID);
		}
	}
	HideInvite();
}

void UMGGameInviteWidget::OnAutoHide()
{
	HideInvite();
}

// ==========================================
// UMGSocialHubWidget
// ==========================================

void UMGSocialHubWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		SocialSubsystem = GI->GetSubsystem<UMGSocialSubsystem>();
		if (SocialSubsystem)
		{
			SocialSubsystem->OnFriendRequestReceived.AddDynamic(this, &UMGSocialHubWidget::OnFriendRequestReceived);
			SocialSubsystem->OnGameInviteReceived.AddDynamic(this, &UMGSocialHubWidget::OnGameInviteReceived);
		}
	}

	UpdateTabDisplay();
}

void UMGSocialHubWidget::NativeDestruct()
{
	if (SocialSubsystem)
	{
		SocialSubsystem->OnFriendRequestReceived.RemoveDynamic(this, &UMGSocialHubWidget::OnFriendRequestReceived);
		SocialSubsystem->OnGameInviteReceived.RemoveDynamic(this, &UMGSocialHubWidget::OnGameInviteReceived);
	}
	Super::NativeDestruct();
}

void UMGSocialHubWidget::ShowTab(int32 TabIndex)
{
	CurrentTab = TabIndex;
	UpdateTabDisplay();
}

void UMGSocialHubWidget::UpdateTabDisplay_Implementation()
{
	// Blueprint implementation - show/hide tab content
}

void UMGSocialHubWidget::OnFriendRequestReceived(const FMGFriendRequest& Request)
{
	// Blueprint implementation - show notification
}

void UMGSocialHubWidget::OnGameInviteReceived(const FMGFriendData& FromFriend, const FString& SessionID)
{
	// Blueprint implementation - show invite popup
}
