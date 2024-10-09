// Fill out your copyright notice in the Description page of Project Settings.


#include "MyWaveTable.h"

UMyWaveTable::UMyWaveTable() {
}

TSharedPtr<Audio::IProxyData> UMyWaveTable::CreateProxyData(const Audio::FProxyDataInitParams &InitParams) {
  // TODO: check `if (!RenderableCopyOfImportedData)` and invalidate it later? 
  UE_LOG(LogTemp, Warning, TEXT("UMyWaveTable::CreateProxyData"))

  RenderableCopyOfImportedData = MakeShared<TArray<float>>(ImportedData);

  TSharedPtr<FMyWaveTableProxy> Proxy = MakeShared<FMyWaveTableProxy>(RenderableCopyOfImportedData, SamplesPerFrame);
  return Proxy;
}
