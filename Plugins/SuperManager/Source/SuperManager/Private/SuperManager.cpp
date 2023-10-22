// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
//#include "BlutilityContentBrowserExtensions.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"


	void FSuperManagerModule::StartupModule()
	{
		InitCBMenuExtention();
	}
#pragma region ContentBrowserMenuExtention

	void FSuperManagerModule::InitCBMenuExtention()
	{
		FContentBrowserModule& ContentBrowserModule =
			FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

		TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtender =
			ContentBrowserModule.GetAllPathViewContextMenuExtenders();

		/*FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;

		CustomCBMenuDelegate.BindRaw(this, &FSuperManagerModule::CustomCBMenuExtender);

		ContentBrowserModuleMenuExtender.Add(CustomCBMenuDelegate);*/

		ContentBrowserModuleMenuExtender.Add(FContentBrowserMenuExtender_SelectedPaths::
			CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));

	}

	TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
	{
		TSharedRef<FExtender> MenuExtender(new FExtender());

		if (SelectedPaths.Num() > 0)
		{
			MenuExtender->AddMenuExtension
			(
				FName("Delete"),
				EExtensionHook::After, TSharedPtr<FUICommandList>(),
				FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry)
			);

			FolderPathsSelected = SelectedPaths;
		}

		return MenuExtender;
	}

	void FSuperManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
	{
		MenuBuilder.AddMenuEntry
		(
			FText::FromString(TEXT("Delete Unused Assets")),
			FText::FromString(TEXT("Safely delete all unused assets under folder")),
			FSlateIcon(),
			FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetsButtonClicked)
		);
	}

	void FSuperManagerModule::OnDeleteUnusedAssetsButtonClicked()
	{
		if (FolderPathsSelected.Num() > 1)
		{
			DebugHeader::ShowMessageDialog(EAppMsgType::Ok, TEXT("Please select only one folder"));
			return;
		}

		//DebugHeader::Print(TEXT("Currently selected folder: ") + FolderPathsSelected[0], FColor::Green);

		TArray<FString> AssetsPathNames =
		UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

		if (AssetsPathNames.Num() == 0)
		{
			DebugHeader::ShowMessageDialog(EAppMsgType::Ok, TEXT("No assets found under selected folder"));
			return;
		}

		EAppReturnType::Type ConfirmResult =
			DebugHeader::ShowMessageDialog(EAppMsgType::YesNo, TEXT("A total of ") + FString::FromInt(AssetsPathNames.Num()) + TEXT(" found.\nWould you like to procceed?"));

		if (ConfirmResult == EAppReturnType::No) return;

		TArray<FAssetData> UnusedAssetsDataArray;

		for (const FString& AssetPathName:AssetsPathNames)
		{
			//Don't touch root folder
			if(AssetPathName.Contains(TEXT("Developers")) || AssetPathName.Contains(TEXT("Developers"))) continue;

			if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

			TArray<FString> AssetReferencers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

			if (AssetReferencers.Num() == 0)
			{
				const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
				UnusedAssetsDataArray.Add(UnusedAssetData);
			}	
		}

		if (UnusedAssetsDataArray.Num() > 0)
		{
			ObjectTools::DeleteAssets(UnusedAssetsDataArray, true);
			return;
		}
		else 
		{
			DebugHeader::ShowMessageDialog(EAppMsgType::Ok, TEXT("No unused assets found under selected folder"));
		}
	}

#pragma endregion


	void FSuperManagerModule::ShutdownModule()
	{
		
	}





#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)