#include "MyCustomNodesEditor.h"

#define LOCTEXT_NAMESPACE "FMyCustomNodesEditorModule"

void FMyCustomNodesEditorModule::StartupModule() {
  MyWaveTableAssetTypeActions = MakeShared<FMyWaveTableAssetTypeActions>();
  FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(MyWaveTableAssetTypeActions.ToSharedRef());
}

void FMyCustomNodesEditorModule::ShutdownModule() {
  if (!FModuleManager::Get().IsModuleLoaded("AssetTools")) return;

  FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(MyWaveTableAssetTypeActions.ToSharedRef());
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FMyCustomNodesEditorModule, MyCustomNodesEditor)