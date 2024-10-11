// Fill out your copyright notice in the Description page of Project Settings.


#include "MyWaveTableEditorToolkit.h"
#include "CoreGlobals.h"
#include "Widgets/Docking/SDockTab.h"
#include "SMyWaveTableWidget.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Utils.h"
#include "Sound/SoundWave.h"
#include "Factories/SoundFactory.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

void FMyWaveTableEditorToolkit::InitEditor(const TArray<UObject *> &InObjects) {
  MyWaveTable = Cast<UMyWaveTable>(InObjects[0]);

  const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("MyWaveTableEditorLayout")
  ->AddArea
  (
    FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
    ->Split
    (
      FTabManager::NewSplitter()
      ->SetSizeCoefficient(0.6f)
      ->SetOrientation(Orient_Horizontal)
      ->Split
      (
        FTabManager::NewStack()
        ->SetSizeCoefficient(0.8f)
        ->AddTab("MyWaveTablePlotTab", ETabState::OpenedTab)
      )
      ->Split
      (
        FTabManager::NewStack()
        ->SetSizeCoefficient(0.2f)
        ->AddTab("MyWaveTableDetailsTab", ETabState::OpenedTab)
      )
    )
  );

  InitAssetEditor(EToolkitMode::Standalone, {}, "MyWaveTableEditor", Layout, true, true, InObjects);
}

void FMyWaveTableEditorToolkit::RegisterTabSpawners(
    const TSharedRef<class FTabManager> &TabManager) {
  FAssetEditorToolkit::RegisterTabSpawners(TabManager);

  WorkspaceMenuCategory = TabManager->AddLocalWorkspaceMenuCategory(INVTEXT("My WaveTable Editor"));
 
  TabManager->RegisterTabSpawner("MyWaveTablePlotTab", FOnSpawnTab::CreateLambda([=](const FSpawnTabArgs&)
  {
          return SNew(SDockTab)
          [
                  SNew(SMyWaveTableWidget)
                  .ImportedData(this, &FMyWaveTableEditorToolkit::GetImportedData)
                  .SamplesPerFrame(this, &FMyWaveTableEditorToolkit::GetSamplesPerFrame)
          ];
  }))
  .SetDisplayName(INVTEXT("Plot"))
  .SetGroup(WorkspaceMenuCategory.ToSharedRef());

  FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
  FDetailsViewArgs DetailsViewArgs;
  DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
  DetailsViewArgs.NotifyHook = this; // !!!
  DetailsViewArgs.bAllowSearch = false;
  TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
  DetailsView->SetObjects(TArray<UObject*>{ MyWaveTable });
  TabManager->RegisterTabSpawner("MyWaveTableDetailsTab", FOnSpawnTab::CreateLambda([=](const FSpawnTabArgs&)
  {
          return SNew(SDockTab)
          [
                  DetailsView
          ];
  }))
  .SetDisplayName(INVTEXT("Details"))
  .SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FMyWaveTableEditorToolkit::UnregisterTabSpawners(
    const TSharedRef<class FTabManager> &TabManager) {
  FAssetEditorToolkit::UnregisterTabSpawners(TabManager);

  TabManager->UnregisterTabSpawner("MyWaveTablePlotTab");
  TabManager->UnregisterTabSpawner("MyWaveTableDetailsTab");
}

TArray<float>* FMyWaveTableEditorToolkit::GetImportedData() const {
  return &(MyWaveTable->ImportedData);
}

int FMyWaveTableEditorToolkit::GetSamplesPerFrame() const {
  return MyWaveTable->SamplesPerFrame;
}

void FMyWaveTableEditorToolkit::NotifyPostChange(const FPropertyChangedEvent &PropertyChangedEvent, FProperty *PropertyThatChanged) {
  auto ImportedFilePath = MyWaveTable->ImportedFilePath.FilePath;
  if (!ImportedFilePath.IsEmpty()) {
    Import(ImportedFilePath);
  };
}

void FMyWaveTableEditorToolkit::Import(const FString& ImportedFilePath) {
  USoundFactory* SoundWaveFactory = NewObject<USoundFactory>();
  if (SoundWaveFactory == nullptr) {
    AbortImport(TEXT("Could not create a SoundWaveFactory for importing"));
    return;
  }

  // TODO: how to avoid warnings about data compression when importing?
  SoundWaveFactory->bAutoCreateCue = false;
  SoundWaveFactory->SuppressImportDialogs();

  UPackage* TempPackage = CreatePackage(TEXT("/Temp/WaveTable"));
  const USoundWave* SoundWave = ImportObject<USoundWave>(
    TempPackage,
    "ImportedWaveTable",
    RF_Public | RF_Standalone,
    *ImportedFilePath,
    nullptr,
    SoundWaveFactory);
  if (SoundWave == nullptr) {
    AbortImport(TEXT("Could not import SoundWave"));
    return;
  }

  TArray<uint8> RawPCMData;
  uint32 SampleRate = 0;
  uint16 NumChannels = 0;
  SoundWave->GetImportedSoundWaveData(RawPCMData, SampleRate, NumChannels);

  if (NumChannels == 0) {
    AbortImport(TEXT("No sound wave data available"));
    return;
  }

  const int32 NumSamples = RawPCMData.Num() / NumChannels / sizeof(int16);

  if (NumSamples % MyWaveTable->SamplesPerFrame != 0) {
    AbortImport(TEXT("WaveTable bank does not contain an even number of frames"));
    return;
  }

  // TODO: is it right to manipulate MyWaveTable members like this?
  MyWaveTable->ImportedData.Empty();
  MyWaveTable->ImportedData.AddZeroed(NumSamples);
  const int16* RawDataPtr = (const int16*)(RawPCMData.GetData());
  constexpr float Max16BitAsFloat = static_cast<float>(TNumericLimits<int16>::Max());
  for (int32 i = 0; i < NumSamples; ++i) {
    MyWaveTable->ImportedData[i] = (RawDataPtr[i * NumChannels]) / Max16BitAsFloat;
  }

  UE_LOG(LogTemp, Display, TEXT("Successfully imported a WaveTable bank from %s"), *(MyWaveTable->ImportedFilePath.FilePath));
  MyWaveTable->ImportedFilePath.FilePath.Empty();
}

void FMyWaveTableEditorToolkit::AbortImport(const FString& ErrMessage) {
  UE_LOG(LogTemp, Error, TEXT("%s"), *ErrMessage);

  FNotificationInfo Info(FText::FromString(ErrMessage));
  Info.ExpireDuration = 10.0f;
  Info.Image = FCoreStyle::Get().GetBrush("Icons.ErrorWithColor"); // also see: "Icons.SuccessWithColor"
  FSlateNotificationManager::Get().AddNotification(Info);

  MyWaveTable->ImportedFilePath.FilePath.Empty();
}
