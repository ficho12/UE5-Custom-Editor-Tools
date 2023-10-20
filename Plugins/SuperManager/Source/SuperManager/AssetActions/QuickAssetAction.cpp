// Fill out your copyright notice in the Description page of Project Settings.

#include "QuickAssetAction.h"
//#include "AssetActions/QuickAssetAction.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "EditorUtilityLibrary.h"


//void UQuickAssetAction::TestFunc()
//{
//	Print(TEXT("Working"), FColor::Cyan);
//	PrintLog(TEXT("Working"));
//}

void UQuickAssetAction::DuplicateAssets(int32 NumOfDuplicates)
{
	if(NumOfDuplicates <= 0)
	{
		Print(TEXT("Number of duplicates must be greater than 0"), FColor::Red);
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
		Print(TEXT("Successfully duplicated " + FString::FromInt(Counter) + " files"), FColor::Green);
	}
}
