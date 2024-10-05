// Fill out your copyright notice in the Description page of Project Settings.


#include "MyWaveTableActions.h"
#include "MyWaveTableEditorToolkit.h"
#include "MyWaveTable.h"

UClass * FMyWaveTableAssetTypeActions::GetSupportedClass() const {
  return UMyWaveTable::StaticClass();
}

FText FMyWaveTableAssetTypeActions::GetName() const {
  return INVTEXT("My WaveTable");
}

FColor FMyWaveTableAssetTypeActions::GetTypeColor() const {
  return FColor(255, 127, 0);
}

uint32 FMyWaveTableAssetTypeActions::GetCategories() {
  return EAssetTypeCategories::Sounds;
}

void FMyWaveTableAssetTypeActions::OpenAssetEditor(const TArray<UObject *> &InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor) {
  MakeShared<FMyWaveTableEditorToolkit>()->InitEditor(InObjects);
}