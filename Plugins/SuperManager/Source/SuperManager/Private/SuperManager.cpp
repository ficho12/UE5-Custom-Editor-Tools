// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "EditorWorldExtension.h" // Added include statement
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"


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

		// Get hold of all the menu extenders
		TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtender =
			ContentBrowserModule.GetAllPathViewContextMenuExtenders();

		//This is a more understandanble way to add to do this
		/*FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;

		CustomCBMenuDelegate.BindRaw(this, &FSuperManagerModule::CustomCBMenuExtender);

		ContentBrowserModuleMenuExtender.Add(CustomCBMenuDelegate);*/


		// Add custom delegate to all the exsinting delegates
		ContentBrowserModuleMenuExtender.Add(FContentBrowserMenuExtender_SelectedPaths::
			CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));

	}

	// To define the position for inserting menu entry
	TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
	{
		TSharedRef<FExtender> MenuExtender(new FExtender());

		if (SelectedPaths.Num() > 0)
		{
			MenuExtender->AddMenuExtension
			(
				FName("Delete"),	// Extension hook, position to insert menu entry
				EExtensionHook::After, // Insert after the hook (or before)
				TSharedPtr<FUICommandList>(),	// Custom hot keys
				FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry)	//Second binding, will define all the details for this menuy entry
			);

			FolderPathsSelected = SelectedPaths;
		}

		return MenuExtender;
	}
	// Define details for the custom menu entry
	void FSuperManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
	{
		MenuBuilder.AddMenuEntry
		(
			FText::FromString(TEXT("Delete Unused Assets")), // Title text for menu entry
			FText::FromString(TEXT("Safely delete all unused assets under folder")), // Tool tip text for menu entry
			FSlateIcon(), // Custom icon for menu entry
			FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetsButtonClicked) // Binding for the action to be executed when menu entry is clicked
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

		// Whether there are assets under the folder 
		if (AssetsPathNames.Num() == 0)
		{
			DebugHeader::ShowMessageDialog(EAppMsgType::Ok, TEXT("No assets found under selected folder"), false);
			return;
		}

		EAppReturnType::Type ConfirmResult =
			DebugHeader::ShowMessageDialog(EAppMsgType::YesNo, TEXT("A total of ") + FString::FromInt(AssetsPathNames.Num()) 
				+ TEXT(" assets need to be checked.\nWould you like to procceed?"));

		if (ConfirmResult == EAppReturnType::No) return;

		FixUpRedirectors();

		TArray<FAssetData> UnusedAssetsDataArray;

		for (const FString& AssetPathName:AssetsPathNames)
		{
			//Don't touch root folder
			if (AssetPathName.Contains(TEXT("Developers")) || AssetPathName.Contains(TEXT("Collections")))	//TODO: Change root folder definition, this can cause false positives --> Contains()
			{
				DebugHeader::ShowMessageDialog(EAppMsgType::Ok, TEXT("Found illegal path name"));
				continue;
			}

			//Check if we can do this in a better way

            if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

			if (UWorld* World = UEditorAssetLibrary::LoadAsset(AssetPathName)->GetWorld())
			{
				if (World->IsEditorWorld())
				{
					// The asset is a level, continue to the next iteration of the loop
					continue;
				}
			}

			// Check if we can do this in a better way (probably checking only AssetData only once and inside the AssetReferencers.Num()==0)
			FAssetData AssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			if (AssetData.AssetClass == "EditorUtilityBlueprint")
			{
				// The asset is an editor utility blueprint, continue to the next iteration of the loop
				continue;
			}

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
		}
		else 
		{
			DebugHeader::ShowMessageDialog(EAppMsgType::Ok, TEXT("No unused assets found under selected folder"));
		}
	}

	void FSuperManagerModule::FixUpRedirectors()
	{
		TArray<UObjectRedirector*> RedirectorsToFixArray;

		FAssetRegistryModule& AssetRegistryModule =
			FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		FARFilter Filter;
		Filter.bRecursivePaths = true;
		Filter.PackagePaths.Emplace(TEXT("/Game"));
		Filter.ClassNames.Emplace("ObjectRedirector");

		TArray<FAssetData> OutRedirectors;
		AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

		for (const FAssetData& RedirectorData : OutRedirectors)
		{
			if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
			{
				RedirectorsToFixArray.Add(RedirectorToFix);
			}
		}

		FAssetToolsModule& AssetToolsModule =
			FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

		AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
	}

#pragma endregion


	void FSuperManagerModule::ShutdownModule()
	{
		
	}





#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)