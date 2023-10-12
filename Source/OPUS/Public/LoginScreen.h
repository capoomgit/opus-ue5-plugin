#pragma once
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

class LoginScreen 
{
public:
	TSharedRef<SDockTab> ShowLoginScreen();
	

private:
	bool IsLoggedIn;
	TSharedPtr<SEditableText> KeyField;

	FReply LoginButtonClicked();
	void OnTextCommittedInKeyField(const FText& Text, ETextCommit::Type CommitMethod);
};