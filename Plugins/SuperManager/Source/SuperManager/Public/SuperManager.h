// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSuperManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

#pragma region ContentBrowserMenuExtention
	void InitCBMenuExtention();

	TArray<FString> FolderPathsSelected;

	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);

	void AddCBMenuEntry(class FMenuBuilder& MenuBuilder);

	void OnDeleteUnusedAssetsButtonClicked();

	void OnDeleteEmptyFoldersButtonClicked();

	void OnAdvancedDeletionClicked();

	void FixUpRedirectors();

#pragma endregion

#pragma region CustomEditorTab

	void RegistrerAdvancedDeletionTab();

	TSharedRef<SDockTab, ESPMode::ThreadSafe> OnSpawnAdvancedDeletionTab(const FSpawnTabArgs& SpawnTabArgs);

#pragma endregion
};
