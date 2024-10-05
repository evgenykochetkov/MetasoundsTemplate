// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"


/**
 * We need to implement a class inheriting from IAssetTypeActions
 * to register our asset type with the engine.
 *
 * By overriding the interface's methods, we can set up
 * how our asset looks and behaves in the editor's content browser.
 *
 * We can choose the name, category, color,
 * actions for the context-menu when the asset is right-clicked
 * and more.
 */
class MYCUSTOMNODESEDITOR_API FMyWaveTableAssetTypeActions : public FAssetTypeActions_Base {
public:
  UClass* GetSupportedClass() const override;
  FText GetName() const override;
  FColor GetTypeColor() const override;
  uint32 GetCategories() override;

  void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor) override;
};
