#include "MetasoundParamHelper.h"
#include "Audio.h"
#include "MetasoundWave.h"
#include "MetasoundWaveTable.h"

#define LOCTEXT_NAMESPACE "MyCustomNodes_GetWaveTableFromWavBankNode"

namespace MyCustomNodes {
using namespace Metasound;

#pragma region Parameter Names

namespace GetWaveTableFromWavBankNodeParameterNames {
  // Input params
  METASOUND_PARAM(InParamNameBankSoundWave, "Bank", "Wavetable bank stored as a SoundWave")
  METASOUND_PARAM(InParamNameNormalizedFrameIndex, "Normalized Frame Index", "0 is the first frame, 1 is the last")
  METASOUND_PARAM(InParamNameFrameSize, "Frame Size", "Frame size in samples (in most cases it will be 2048)")

  // Output params
  METASOUND_PARAM(OutParamNameWaveTable, "WaveTable", "Extracted wavetable")
}

#pragma endregion

#pragma region Operator

class FGetWaveTableFromWavBankOperator : public TExecutableOperator<FGetWaveTableFromWavBankOperator> {
  FWaveAssetReadRef BankSoundWave;
  FFloatReadRef NormalizedFrameIndex;
  int32 FrameSize;

  FWaveTableWriteRef WaveTable;
  TArray<float> AllBankSamples;
  float PrevNormalizedFrameIndexIndex = -1.f; 
public:
  FGetWaveTableFromWavBankOperator(const FOperatorSettings &InSettings,
                                   const FWaveAssetReadRef &InBankSoundWave,
                                   const FFloatReadRef &InNormalizedFrameIndex,
                                   const int32 InFrameSize)
    : BankSoundWave(InBankSoundWave),
      NormalizedFrameIndex(InNormalizedFrameIndex),
      FrameSize(InFrameSize),
      WaveTable(TDataWriteReferenceFactory<WaveTable::FWaveTable>::CreateAny(InSettings))
  {
    WaveTable->SetNum(FrameSize);
    WaveTable->Zero();

    // Reading samples data from a SoundWave turned out to be way less straightforward than it should. 
    // See https://forums.unrealengine.com/t/how-to-reliably-access-usoundwave-pcm-data-with-or-without-fasyncaudiodecompress/444631/8
    const FWaveAsset WaveAsset = *BankSoundWave;
    FSoundWaveProxyPtr ProxyPtr = WaveAsset.GetSoundWaveProxy();
    if (ProxyPtr.IsValid() && WaveAsset.IsSoundWaveValid()) {
      check(ProxyPtr->GetRuntimeFormat() == Audio::NAME_PCM) // TODO: what is a proper way to show error?
      check(ProxyPtr->GetLoadingBehavior() == ESoundWaveLoadingBehavior::ForceInline)

      auto SoundWaveData = ProxyPtr->GetSoundWaveData();
      check(SoundWaveData->GetNumChannels() == 1)
      const auto ResourceSize = SoundWaveData->GetResourceSize();
      check(ResourceSize > 0)

      FWaveModInfo WaveInfo;
      if (WaveInfo.ReadWaveHeader(SoundWaveData->GetResourceData(), ResourceSize, 0)) {
        const auto NumSamples = SoundWaveData->GetNumFrames();
        TArrayView<const int16> ChannelArrayView(reinterpret_cast<const int16*>(WaveInfo.SampleDataStart), NumSamples);

        AllBankSamples.Empty();
        AllBankSamples.AddZeroed(NumSamples);

        constexpr float Max16BitAsFloat = TNumericLimits<int16>::Max();

        for (int32 SampleIndex = 0; SampleIndex < NumSamples; SampleIndex++) {
          AllBankSamples[SampleIndex] = ChannelArrayView[SampleIndex] / Max16BitAsFloat;
        }

        UE_LOG(LogTemp, Display, TEXT("FGetWaveTableFromWavBankOperator: successfully loaded %i samples"), AllBankSamples.Num());

        Execute();
      } else {
        UE_LOG(LogTemp, Error, TEXT("FGetWaveTableFromWavBankOperator: error reading WAV header"));
      }
    }
  }

#pragma region Operator boilerplate
  
  static const FNodeClassMetadata &GetNodeInfo() {
    auto InitNodeInfo = []() -> FNodeClassMetadata {
      FNodeClassMetadata Info;

      Info.ClassName = {TEXT("UE"), TEXT("GetWaveTableFromWavBank"), TEXT("Audio")};
      Info.MajorVersion = 1;
      Info.MinorVersion = 0;
      Info.DisplayName = LOCTEXT("MyCustomNodes_GetWaveTableFromWavBankDisplayName", "Get WaveTable From WAV Bank");
      Info.Description = LOCTEXT("MyCustomNodes_GetWaveTableFromWavBankNodeDescription",
                                 "I need to write a proper description here"); // TODO
      Info.Author = "Evgeny Kochetkov";
      Info.PromptIfMissing = PluginNodeMissingPrompt;
      Info.DefaultInterface = GetVertexInterface();
      Info.CategoryHierarchy = {LOCTEXT("MyCustomNodes_GetWaveTableFromWavBankNodeCategory", "My Custom Category")};

      return Info;
    };

    static const FNodeClassMetadata Info = InitNodeInfo();

    return Info;  
  }
  
  static const FVertexInterface &GetVertexInterface() {
    using namespace GetWaveTableFromWavBankNodeParameterNames;
    
    static const FVertexInterface Interface(
      FInputVertexInterface(
        TInputDataVertex<FWaveAsset>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameBankSoundWave)),
        TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameNormalizedFrameIndex), 0.0f),
        TInputConstructorVertex<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA_ADVANCED(InParamNameFrameSize), 2048)
      ),
      FOutputVertexInterface(
        TOutputDataVertex<WaveTable::FWaveTable>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameWaveTable))
      )
    );

    return Interface;
  }

  static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams &InParams, FBuildResults &OutResults) {
    using namespace GetWaveTableFromWavBankNodeParameterNames;

    FWaveAssetReadRef InBankSoundWave =
      InParams.InputData.GetOrConstructDataReadReference<FWaveAsset>(
        METASOUND_GET_PARAM_NAME(InParamNameBankSoundWave));
    
    FFloatReadRef InNormalizedFrameIndex =
      InParams.InputData.GetOrCreateDefaultDataReadReference<float>(
         METASOUND_GET_PARAM_NAME(InParamNameNormalizedFrameIndex),
         InParams.OperatorSettings);

    int32 InFrameSize =
      InParams.InputData.GetOrCreateDefaultValue<int32>(
         METASOUND_GET_PARAM_NAME(InParamNameFrameSize),
         InParams.OperatorSettings);

    return MakeUnique<FGetWaveTableFromWavBankOperator>(
        InParams.OperatorSettings,
        InBankSoundWave,
        InNormalizedFrameIndex,
        InFrameSize);
  }
  
  virtual void BindInputs(FInputVertexInterfaceData &InOutVertexData) override {
    using namespace GetWaveTableFromWavBankNodeParameterNames;

    InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InParamNameBankSoundWave), BankSoundWave);
    InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InParamNameNormalizedFrameIndex), NormalizedFrameIndex);
    InOutVertexData.SetValue(METASOUND_GET_PARAM_NAME(InParamNameFrameSize), FrameSize);
  }
  
  virtual void BindOutputs(FOutputVertexInterfaceData &InOutVertexData) override {
    using namespace GetWaveTableFromWavBankNodeParameterNames;

    InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutParamNameWaveTable), WaveTable);
  }

#pragma endregion Operator boilerplate

  void Execute() {
    if (AllBankSamples.Num() == 0 || FMath::IsNearlyEqual(PrevNormalizedFrameIndexIndex, *NormalizedFrameIndex)) {
      return;
    }

    uint32 MaxFrameIndex =  (AllBankSamples.Num() / FrameSize) - 1;
    int FrameIndex = *NormalizedFrameIndex * MaxFrameIndex;
    const TArrayView<float> WaveTableSamplesView = WaveTable->GetSamples();
    for (int32 i = 0; i < FrameSize; i++) {
      WaveTableSamplesView[i] = AllBankSamples[i + FrameSize * FrameIndex];
    }

    PrevNormalizedFrameIndexIndex = *NormalizedFrameIndex;
  }
};

#pragma endregion

#pragma region Node

class FGetWaveTableFromWavBankNode : public FNodeFacade {
public:
  // Constructor used by the Metasound Frontend.
  FGetWaveTableFromWavBankNode(const FNodeInitData &InitData)
      : FNodeFacade(InitData.InstanceName,
                    InitData.InstanceID,
                    Metasound::TFacadeOperatorClass<FGetWaveTableFromWavBankOperator>()) {}
};

METASOUND_REGISTER_NODE(FGetWaveTableFromWavBankNode)

#pragma endregion

}

#undef LOCTEXT_NAMESPACE
