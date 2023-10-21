// Fill out your copyright notice in the Description page of Project Settings.

#include "QuickAssetAction.h"
//#include "AssetActions/QuickAssetAction.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "EditorUtilityLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"


//void UQuickAssetAction::TestFunc()
//{
//	Print(TEXT("Working"), FColor::Cyan);
//	PrintLog(TEXT("Working"));
//}

void UQuickAssetAction::DuplicateAssets(int32 NumOfDuplicates)
{
	if(NumOfDuplicates <= 0)
	{
		ShowMessageDialog(EAppMsgType::Ok,TEXT("Please enter a VALID number"));
		return;
	}

	TArray<FAssetData> SelectedAssetsData =  UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	
	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		for (int32 i = 0; i < NumOfDuplicates; i++)
		{
			const FString SourceAssetPath = SelectedAssetData.ObjectPath.ToString();
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i+1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(),NewDuplicatedAssetName);

			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				Counter++;
			}
		}
	}

	if (Counter>0)
	{
		ShowNotifyInfo(TEXT("Successfully duplicated " + FString::FromInt(Counter) + " files"));
		//Print(TEXT("Successfully duplicated " + FString::FromInt(Counter) + " files"), FColor::Green);
	}
}

void UQuickAssetAction::AddPrefixes()
{
	TArray<UObject*>SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for (UObject* SelectedObject : SelectedObjects)
	{
		if(!SelectedObject) continue;

		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());

		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			Print(TEXT("Failed to find prefix for class ") + SelectedObject->GetClass()->GetName(), FColor::Cyan);
			continue;
		}

		FString OldName = SelectedObject->GetName();

		if (OldName.StartsWith(*PrefixFound))
		{
			Print(TEXT(" already has prefix added"), FColor::Red);
			continue;
		}
		
		if (SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}

		const FString NewNameWithPrefix = *PrefixFound + OldName;
		
		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);
		Counter++;
	}
	if(Counter > 0)
	{
		ShowNotifyInfo(TEXT("Successfully renamed " + FString::FromInt(Counter) + " assets"));
	}
}

void UQuickAssetAction::AddPrefixesToAssetArray(TArray<UObject*>SelectedObjects)
{
	//TArray<UObject*>SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for (UObject* SelectedObject : SelectedObjects)
	{
		if (!SelectedObject) continue;

		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());

		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			Print(TEXT("Failed to find prefix for class ") + SelectedObject->GetClass()->GetName(), FColor::Cyan);
			continue;
		}

		FString OldName = SelectedObject->GetName();

		if (OldName.StartsWith(*PrefixFound))
		{
			Print(TEXT(" already has prefix added"), FColor::Red);
			continue;
		}

		if (SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}

		const FString NewNameWithPrefix = *PrefixFound + OldName;

		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);
		Counter++;
	}
	if (Counter > 0)
	{
		ShowNotifyInfo(TEXT("Successfully renamed " + FString::FromInt(Counter) + " assets"));
	}
}

void UQuickAssetAction::RemoveUnusedAssets()
{
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetsData;

	FixUpRedirectors();

	for (const FAssetData& SeletedAssetData:SelectedAssetsData)
	{
		TArray<FString> AssetReferencers =
		UEditorAssetLibrary::FindPackageReferencersForAsset(SeletedAssetData.ObjectPath.ToString());

		if (AssetReferencers.Num() == 0)
		{
			UnusedAssetsData.Add(SeletedAssetData);
		}
	}

	if (UnusedAssetsData.Num() == 0)
	{
		ShowMessageDialog(EAppMsgType::Ok, TEXT("No unused assets found among selected assets"), false);
		return;
	}

	int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData, true);

	if (NumOfAssetsDeleted == 0) return;
	
	ShowNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfAssetsDeleted) + " assets"));
}

void UQuickAssetAction::BatchRename(FString NewName)
{
	TArray<UObject*>SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 1;

	if (NewName.IsEmpty())
	{
		ShowMessageDialog(EAppMsgType::Ok, TEXT("Please enter a non-empty name"));
		return;
	}

	for (UObject* SelectedObject : SelectedObjects)
	{
		const FString OldName = SelectedObject->GetName();
		//const FString NewName = OldName.Replace(TEXT(" "), TEXT("_"));

		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewName + "_" + FString::FromInt(Counter));
		Counter++;
	}

	AddPrefixesToAssetArray(SelectedObjects);
}

void UQuickAssetAction::FixUpRedirectors()
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
