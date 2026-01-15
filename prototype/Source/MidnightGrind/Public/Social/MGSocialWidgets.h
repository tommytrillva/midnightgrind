// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGSocialSubsystem.h"
#include "MGSocialWidgets.generated.h"

class UTextBlock;
class UImage;
class UButton;
class UScrollBox;
class UVerticalBox;

/**
 * Friend list entry widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGFriendEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendSelected, const FMGFriendData&, Friend);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendAction, const FMGFriendData&, Friend);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFriendSelected OnSelected;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFriendAction OnJoinRequested;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFriendAction OnInviteRequested;

	/** Set friend data */
	UFUNCTION(BlueprintCallable, Category = "Friend")
	void SetFriendData(const FMGFriendData& Friend);

	/** Get friend data */
	UFUNCTION(BlueprintPure, Category = "Friend")
	FMGFriendData GetFriendData() const { return FriendData; }

	/** Set selected state */
	UFUNCTION(BlueprintCallable, Category = "Friend")
	void SetSelected(bool bSelected);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGFriendData FriendData;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsSelected = false;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Handle click */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void HandleClick();

	/** Handle join button */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void HandleJoinClick();

	/** Handle invite button */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void HandleInviteClick();
};

/**
 * Friends list panel widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGFriendsListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Refresh friends list */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	void RefreshList();

	/** Filter by status */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	void SetStatusFilter(bool bOnlineOnly);

	/** Search friends */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	void SearchFriends(const FString& SearchTerm);

protected:
	/** Entry widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGFriendEntryWidget> EntryWidgetClass;

	/** Show only online friends */
	UPROPERTY(BlueprintReadOnly, Category = "Filter")
	bool bOnlineOnly = false;

	/** Current search term */
	UPROPERTY(BlueprintReadOnly, Category = "Filter")
	FString SearchTerm;

	/** Entry widgets */
	UPROPERTY()
	TArray<UMGFriendEntryWidget*> EntryWidgets;

	/** Social subsystem */
	UPROPERTY()
	UMGSocialSubsystem* SocialSubsystem;

	/** Handle friends list update */
	UFUNCTION()
	void OnFriendsUpdated(const TArray<FMGFriendData>& Friends);

	/** Update list display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateListDisplay(const TArray<FMGFriendData>& Friends);

	/** Create entry widget */
	UMGFriendEntryWidget* CreateEntryWidget();
};

/**
 * Friend request widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGFriendRequestWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestHandled, const FString&, RequestID);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRequestHandled OnAccepted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRequestHandled OnDeclined;

	/** Set request data */
	UFUNCTION(BlueprintCallable, Category = "Request")
	void SetRequestData(const FMGFriendRequest& Request);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGFriendRequest RequestData;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Accept request */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void AcceptRequest();

	/** Decline request */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void DeclineRequest();
};

/**
 * Crew member entry widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGCrewMemberWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemberAction, const FString&, PlayerID);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMemberAction OnKickRequested;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMemberAction OnPromoteRequested;

	/** Set member data */
	UFUNCTION(BlueprintCallable, Category = "Crew")
	void SetMemberData(const FMGCrewMember& Member, EMGCrewRank ViewerRank);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGCrewMember MemberData;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	EMGCrewRank ViewerRank;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bCanKick = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bCanPromote = false;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Kick member */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void KickMember();

	/** Promote member */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void PromoteMember();
};

/**
 * Crew info panel widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGCrewPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Set crew data */
	UFUNCTION(BlueprintCallable, Category = "Crew")
	void SetCrewData(const FMGCrewData& Crew, EMGCrewRank PlayerRank);

	/** Refresh crew data */
	UFUNCTION(BlueprintCallable, Category = "Crew")
	void RefreshCrew();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGCrewData CrewData;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	EMGCrewRank PlayerRank;

	/** Member widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGCrewMemberWidget> MemberWidgetClass;

	/** Member widgets */
	UPROPERTY()
	TArray<UMGCrewMemberWidget*> MemberWidgets;

	/** Social subsystem */
	UPROPERTY()
	UMGSocialSubsystem* SocialSubsystem;

	/** Handle crew update */
	UFUNCTION()
	void OnCrewUpdated(const FMGCrewData& Crew);

	/** Update crew display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateCrewDisplay();

	/** Update members display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateMembersDisplay();

	/** Create member widget */
	UMGCrewMemberWidget* CreateMemberWidget();
};

/**
 * Create crew dialog widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGCreateCrewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCrewCreated);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCancelled);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCrewCreated OnCrewCreated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCancelled OnCancelled;

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Input")
	FString CrewName;

	UPROPERTY(BlueprintReadWrite, Category = "Input")
	FString CrewTag;

	UPROPERTY(BlueprintReadWrite, Category = "Input")
	FString Description;

	UPROPERTY(BlueprintReadWrite, Category = "Input")
	FLinearColor CrewColor = FLinearColor::White;

	/** Validate input */
	UFUNCTION(BlueprintPure, Category = "Validation")
	bool IsInputValid() const;

	/** Get validation error */
	UFUNCTION(BlueprintPure, Category = "Validation")
	FText GetValidationError() const;

	/** Create crew */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void CreateCrew();

	/** Cancel */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void Cancel();
};

/**
 * Recent players widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGRecentPlayersWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Refresh list */
	UFUNCTION(BlueprintCallable, Category = "Recent")
	void RefreshList();

protected:
	/** Entry widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UUserWidget> EntryWidgetClass;

	/** Social subsystem */
	UPROPERTY()
	UMGSocialSubsystem* SocialSubsystem;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay(const TArray<FMGRecentPlayer>& Players);

	/** Add friend from recent */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void AddFriendFromRecent(const FString& PlayerID);

	/** Block player from recent */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void BlockPlayerFromRecent(const FString& PlayerID);
};

/**
 * Game invite notification widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGGameInviteWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Show invite */
	UFUNCTION(BlueprintCallable, Category = "Invite")
	void ShowInvite(const FMGFriendData& FromFriend, const FString& SessionID);

	/** Hide invite */
	UFUNCTION(BlueprintCallable, Category = "Invite")
	void HideInvite();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGFriendData InviterData;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FString SessionID;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsShowing = false;

	/** Auto-hide timer */
	FTimerHandle AutoHideTimer;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Accept invite */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void AcceptInvite();

	/** Decline invite */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void DeclineInvite();

	/** On auto-hide timer */
	void OnAutoHide();
};

/**
 * Social hub main screen widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGSocialHubWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Show tab */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void ShowTab(int32 TabIndex);

protected:
	/** Current tab */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 CurrentTab = 0;

	/** Friends list widget */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Widgets")
	UMGFriendsListWidget* FriendsListWidget;

	/** Crew panel widget */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UMGCrewPanelWidget* CrewPanelWidget;

	/** Recent players widget */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UMGRecentPlayersWidget* RecentPlayersWidget;

	/** Social subsystem */
	UPROPERTY()
	UMGSocialSubsystem* SocialSubsystem;

	/** Update tab display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateTabDisplay();

	/** Handle friend request received */
	UFUNCTION()
	void OnFriendRequestReceived(const FMGFriendRequest& Request);

	/** Handle game invite received */
	UFUNCTION()
	void OnGameInviteReceived(const FMGFriendData& FromFriend, const FString& SessionID);
};
