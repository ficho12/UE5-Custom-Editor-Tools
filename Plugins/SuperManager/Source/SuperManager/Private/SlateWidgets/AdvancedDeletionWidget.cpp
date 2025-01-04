// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvancedDeletionWidget.h"

void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	UE_LOG(LogTemp, Log, TEXT("OnSpawnAdvancedDeletionTab called"));

	bCanSupportFocus = true;

	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));

	TitleTextFont.Size = 30;

	ChildSlot
	[
		//Main Vertical Box
		SNew(SVerticalBox)

		//First vertical slot for the title text
		+ SVerticalBox::Slot()
		.AutoHeight()
			[
			SNew(STextBlock)
			.Text(FText::FromString("Advanced Deletion"))
			.Font(TitleTextFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FLinearColor::White)
		]
	];
}
