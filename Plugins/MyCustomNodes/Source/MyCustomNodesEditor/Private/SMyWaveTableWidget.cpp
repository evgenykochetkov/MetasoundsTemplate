// Fill out your copyright notice in the Description page of Project Settings.


#include "SMyWaveTableWidget.h"
#include "Editor.h"

void SMyWaveTableWidget::Construct(const FArguments &InArgs) {
  ImportedData = InArgs._ImportedData;
  SamplesPerFrame = InArgs._SamplesPerFrame;
}

int32 SMyWaveTableWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
  const TArray<float> Data = ImportedData.Get();
  const int32 PointsPerFrame = SamplesPerFrame.Get();
  const int32 NumFrames = Data.Num() / PointsPerFrame;

  const FTransform2D PointsTransform = GetPointsTransform(AllottedGeometry);
  TArray<FVector2D> Points;
  for (int32 FrameIndex = NumFrames - 1; FrameIndex >= 0; FrameIndex--) {
    Points.Empty(PointsPerFrame);
    int32 PointIndexOffset = PointsPerFrame * FrameIndex;

    float NormalizedIndex = FrameIndex / (NumFrames - 1.0); // [0, 1]
    float NormalizedIndexBipolar = NormalizedIndex * 2.0 - 1.0; // [-1, 1] 

    for (int32 PointIndex = 0; PointIndex < PointsPerFrame; ++PointIndex) {
      const float X = PointIndex / (PointsPerFrame - 1.0) + NormalizedIndexBipolar * 0.125;
      const float Y = Data[PointIndex + PointIndexOffset] * 0.15 + NormalizedIndexBipolar * 0.85;
      Points.Add(PointsTransform.TransformPoint(FVector2D(X, Y)));
    }

    auto LineColor = FMath::Lerp(PlotFrontColor, PlotBackColor, FMath::Pow(NormalizedIndex, 2.25));
    FSlateDrawElement::MakeLines(
        OutDrawElements,
        LayerId,
        AllottedGeometry.ToPaintGeometry(),
        Points,
        ESlateDrawEffect::None,
        LineColor,
        true,
        1.25);
  }
  
  return LayerId;
}

FVector2D SMyWaveTableWidget::ComputeDesiredSize(float) const {
  return FVector2D(200.0, 200.0);
}

FTransform2D SMyWaveTableWidget::GetPointsTransform(const FGeometry& AllottedGeometry) const {
  const double Margin = 0.2 * AllottedGeometry.GetLocalSize().GetMin();
  // our plot X is 0 to 1, Y is -1 to 1
  const FScale2D Scale((AllottedGeometry.GetLocalSize() - 2.0 * Margin) * FVector2D(1.0, -0.5));
  const FVector2D Translation(Margin, 0.5 * AllottedGeometry.GetLocalSize().Y);
  return FTransform2D(Scale, Translation);
}
