#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "MyWaveTableActions.h"

class FMyCustomNodesEditorModule : public IModuleInterface {
public:
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

private:
  TSharedPtr<FMyWaveTableAssetTypeActions> MyWaveTableAssetTypeActions;
};
