// Copyright Epic Games, Inc. All Rights Reserved.

#include "OPUSStyle.h"
#include "OPUS.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FOPUSStyle::StyleInstance = nullptr;

void FOPUSStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FOPUSStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FOPUSStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("OPUSStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FOPUSStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("OPUSStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("OPUS")->GetBaseDir() / TEXT("Resources"));

	Style->Set("OPUS.PluginAction", new IMAGE_BRUSH_SVG(TEXT("opusapi_icon_diamond"), Icon20x20));

	Style->Set("OPUS.APILogo", new IMAGE_BRUSH_SVG(TEXT("opusapi_horizontal_white"), FVector2D(381, 100)));

	Style->Set("OPUS.SmallLogo", new IMAGE_BRUSH(TEXT("opusapi_icon"), FVector2D(112, 83)));

	return Style;
}

void FOPUSStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FOPUSStyle::Get()
{
	return *StyleInstance;
}
