// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

class MYCUSTOMNODESEDITOR_API SMyWaveTableWidget : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SMyWaveTableWidget)
            : _ImportedData(nullptr)
            , _SamplesPerFrame(2048)
    {}
    SLATE_ATTRIBUTE(TArray<float>*, ImportedData)
    SLATE_ATTRIBUTE(int, SamplesPerFrame)
  SLATE_END_ARGS()

  void Construct(const FArguments& InArgs);

  int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
  FVector2D ComputeDesiredSize(float) const override;

private:
  TAttribute<TArray<float>*> ImportedData;
  TAttribute<int> SamplesPerFrame;

  FTransform2D GetPointsTransform(const FGeometry& AllottedGeometry) const;

  const FLinearColor PlotFrontColor = FLinearColor(1.0, 0.5, 0.0);
  const FLinearColor PlotBackColor = FLinearColor(0.7, 0.7, 0.7);
};
