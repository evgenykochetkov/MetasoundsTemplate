// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyWaveTable.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Misc/NotifyHook.h"

class MYCUSTOMNODESEDITOR_API FMyWaveTableEditorToolkit : public FAssetEditorToolkit, public FNotifyHook {
public:
  void InitEditor(const TArray<UObject*>& InObjects);

  void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
  void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

  FName GetToolkitFName() const override { return "MyWaveTableEditor"; }
  FText GetBaseToolkitName() const override { return INVTEXT("My WaveTable Editor"); }
  FString GetWorldCentricTabPrefix() const override { return "My WaveTable "; }
  FLinearColor GetWorldCentricTabColorScale() const override { return {}; }

  TArray<float>* GetImportedData() const;
  int GetSamplesPerFrame() const;

  //FNotifyHook
  virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;

private:
  UMyWaveTable* MyWaveTable;

  void Import(const FString& ImportedFilePath);
  void AbortImport(const FString& ErrMessage);
};
