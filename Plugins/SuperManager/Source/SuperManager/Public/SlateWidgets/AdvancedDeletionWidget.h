// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SAdvanceDeletionTab : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SAdvanceDeletionTab){}
	SLATE_ARGUMENT(FString,TestString)
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs);
};


