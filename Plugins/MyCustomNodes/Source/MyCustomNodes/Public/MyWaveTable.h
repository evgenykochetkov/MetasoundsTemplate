// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MyWaveTable.generated.h"


UCLASS(BlueprintType)
class MYCUSTOMNODES_API UMyWaveTable : public UObject {
  GENERATED_BODY()

public:
  UMyWaveTable();

  UPROPERTY(VisibleAnywhere, Category="Settings")
  int SamplesPerFrame = 2048;

  UPROPERTY()
  TArray<float> ImportedData;

#if WITH_EDITORONLY_DATA
  UPROPERTY(EditAnywhere, SkipSerialization, Category="Import", DisplayName="Select a .WAV File", meta=(FilePathFilter=".WAV File|*.wav"))
  FFilePath ImportedFilePath;
#endif
};
