#include "MyCustomNodesEditor.h"
#include "Modules/ModuleManager.h"
#include "MetasoundEditorModule.h"

#define LOCTEXT_NAMESPACE "FMyCustomNodesEditorModule"

void FMyCustomNodesEditorModule::StartupModule() {
  MyWaveTableAssetTypeActions = MakeShared<FMyWaveTableAssetTypeActions>();
  FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(MyWaveTableAssetTypeActions.ToSharedRef());

  Metasound::Editor::IMetasoundEditorModule& MetasoundEditorModule = FModuleManager::GetModuleChecked<Metasound::Editor::IMetasoundEditorModule>("MetasoundEditor");
  MetasoundEditorModule.RegisterPinType("MyWaveTableAsset");
}

void FMyCustomNodesEditorModule::ShutdownModule() {
  if (!FModuleManager::Get().IsModuleLoaded("AssetTools")) return;

  FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(MyWaveTableAssetTypeActions.ToSharedRef());
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FMyCustomNodesEditorModule, MyCustomNodesEditor)