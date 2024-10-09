// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "IAudioProxyInitializer.h"

#include "MyWaveTable.generated.h"


UCLASS(BlueprintType)
class MYCUSTOMNODES_API UMyWaveTable : public UObject, public IAudioProxyDataFactory {
  GENERATED_BODY()

public:
  UMyWaveTable();

  UPROPERTY(VisibleAnywhere, Category="Settings")
  int SamplesPerFrame = 2048;

  UPROPERTY()
  TArray<float> ImportedData;

  virtual TSharedPtr<Audio::IProxyData> CreateProxyData(const Audio::FProxyDataInitParams& InitParams) override;
  
#if WITH_EDITORONLY_DATA
  UPROPERTY(EditAnywhere, SkipSerialization, Category="Import", DisplayName="Select a .WAV File", meta=(FilePathFilter=".WAV File|*.wav"))
  FFilePath ImportedFilePath;
#endif

private:
  TSharedPtr<TArray<float>> RenderableCopyOfImportedData;
};

class MYCUSTOMNODES_API FMyWaveTableProxy : public Audio::TProxyData<FMyWaveTableProxy> {
public:
  IMPL_AUDIOPROXY_CLASS(FMyWaveTableProxy);

  explicit FMyWaveTableProxy(const TSharedPtr<TArray<float>> &AllSamples,
                             const int SamplesPerFrame)
    : AllSamples(AllSamples),
      SamplesPerFrame(SamplesPerFrame) {
  }

  TSharedPtr<TArray<float>> GetAllSamples() {
    return AllSamples;
  }

  int GetSamplesPerFrame() {
    return SamplesPerFrame;
  }

private:
  TSharedPtr<TArray<float>> AllSamples;
  int SamplesPerFrame;
};

using FMyWaveTableProxyPtr = TSharedPtr<FMyWaveTableProxy, ESPMode::ThreadSafe>;
