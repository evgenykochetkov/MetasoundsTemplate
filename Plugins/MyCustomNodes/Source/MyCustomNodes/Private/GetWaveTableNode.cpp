#include "MetasoundParamHelper.h"
#include "MetasoundEnumRegistrationMacro.h"
#include "MetasoundWaveTable.h"
#include "MyWaveTableAsset.h"

#define LOCTEXT_NAMESPACE "MyCustomNodes_GetWaveTableNode"

namespace MyCustomNodes {
using namespace Metasound;

#pragma region Parameter Names

namespace GetWaveTableNodeParameterNames {
  // Input params
  METASOUND_PARAM(InParamNameMyWaveTable, "WaveTable", "My Custom WaveTable Asset")
  METASOUND_PARAM(InParamNameNormalizedFrameIndex, "Normalized Frame Index", "0 is the first frame, 1 is the last")

  // Output params
  METASOUND_PARAM(OutParamNameWaveTable, "WaveTable", "Extracted wavetable")
}

#pragma endregion

#pragma region Operator

class FGetWaveTableOperator : public TExecutableOperator<FGetWaveTableOperator> {
  FMyWaveTableAssetReadRef MyWaveTableAsset;
  FFloatReadRef NormalizedFrameIndex;

  FWaveTableWriteRef WaveTable;
  // TArray<float> AllBankSamples; // TODO: do we need to cache the data?
  float PrevNormalizedFrameIndexIndex = -1.f; 
public:
  FGetWaveTableOperator(const FOperatorSettings &InSettings,
                        const FMyWaveTableAssetReadRef &InMyWaveTableAsset,
                        const FFloatReadRef &InNormalizedFrameIndex)
    : MyWaveTableAsset(InMyWaveTableAsset),
      NormalizedFrameIndex(InNormalizedFrameIndex),
      WaveTable(TDataWriteReferenceFactory<WaveTable::FWaveTable>::CreateAny(InSettings))
  {
    auto Asset = *MyWaveTableAsset;
    auto ProxyPtr = Asset.GetWaveTableProxy();
    if (!ProxyPtr.IsValid()) {
      UE_LOG(LogTemp, Error, TEXT("FGetWaveTableOperator: constructor: ProxyPtr is invalid"))
      return;
    }

    WaveTable->SetNum(ProxyPtr->GetSamplesPerFrame());
    WaveTable->Zero();

    // TODO: do we need to cache the data?
    // auto AssetSamples = ProxyPtr->GetAllSamples();
    // AllBankSamples = *AssetSamples.Get();

    Execute();
  }

#pragma region Operator boilerplate
  
  static const FNodeClassMetadata &GetNodeInfo() {
    auto InitNodeInfo = []() -> FNodeClassMetadata {
      FNodeClassMetadata Info;

      Info.ClassName = {TEXT("UE"), TEXT("GetWaveTable"), TEXT("Audio")};
      Info.MajorVersion = 1;
      Info.MinorVersion = 0;
      Info.DisplayName = LOCTEXT("MyCustomNodes_GetWaveTableDisplayName", "Get WaveTable From Bank");
      Info.Description = LOCTEXT("MyCustomNodes_GetWaveTableNodeDescription",
                                 "Get a Single WaveTable From MyWaveTable Bank");
      Info.Author = "Evgeny Kochetkov";
      Info.PromptIfMissing = PluginNodeMissingPrompt;
      Info.DefaultInterface = GetVertexInterface();
      Info.CategoryHierarchy = {LOCTEXT("MyCustomNodes_GetWaveTableNodeCategory", "My Custom Category")}; // TODO: change to "Wave Tables"?

      return Info;
    };

    static const FNodeClassMetadata Info = InitNodeInfo();

    return Info;  
  }
  
  static const FVertexInterface &GetVertexInterface() {
    using namespace GetWaveTableNodeParameterNames;
    
    static const FVertexInterface Interface(
      FInputVertexInterface(
        TInputDataVertex<FMyWaveTableAsset>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameMyWaveTable)),
        TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameNormalizedFrameIndex), 0.0f)
      ),
      FOutputVertexInterface(
        TOutputDataVertex<WaveTable::FWaveTable>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameWaveTable))
      )
    );

    return Interface;
  }

  static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams &InParams, FBuildResults &OutResults) {
    using namespace GetWaveTableNodeParameterNames;

    FMyWaveTableAssetReadRef InMyWaveTableAsset =
      InParams.InputData.GetOrCreateDefaultDataReadReference<FMyWaveTableAsset>(
        METASOUND_GET_PARAM_NAME(InParamNameMyWaveTable),
        InParams.OperatorSettings);

    FFloatReadRef InNormalizedFrameIndex =
      InParams.InputData.GetOrCreateDefaultDataReadReference<float>(
         METASOUND_GET_PARAM_NAME(InParamNameNormalizedFrameIndex),
         InParams.OperatorSettings);

    return MakeUnique<FGetWaveTableOperator>(
        InParams.OperatorSettings,
        InMyWaveTableAsset,
        InNormalizedFrameIndex);
  }
  
  virtual void BindInputs(FInputVertexInterfaceData &InOutVertexData) override {
    using namespace GetWaveTableNodeParameterNames;
    
    InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InParamNameMyWaveTable), MyWaveTableAsset);
    InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InParamNameNormalizedFrameIndex), NormalizedFrameIndex);
  }
  
  virtual void BindOutputs(FOutputVertexInterfaceData &InOutVertexData) override {
    using namespace GetWaveTableNodeParameterNames;

    InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutParamNameWaveTable), WaveTable);
  }

#pragma endregion Operator boilerplate

  void Execute() {
    if (FMath::IsNearlyEqual(PrevNormalizedFrameIndexIndex, *NormalizedFrameIndex)) {
      return;
    }

    auto Asset = *MyWaveTableAsset;
    auto ProxyPtr = Asset.GetWaveTableProxy();
    if (!ProxyPtr.IsValid()) {
      UE_LOG(LogTemp, Error, TEXT("FGetWaveTableOperator: Execute: ProxyPtr is invalid"))
      return;
    }

    auto FrameSize = ProxyPtr->GetSamplesPerFrame();
    auto AllBankSamples = ProxyPtr->GetAllSamples();

    if (!AllBankSamples.IsValid()) {
      UE_LOG(LogTemp, Error, TEXT("FGetWaveTableOperator: Execute: AllBankSamples is invalid"))
      return;
    }

    uint32 MaxFrameIndex = (AllBankSamples->Num() / FrameSize) - 1;
    float ScaledFrameIndex = *NormalizedFrameIndex * MaxFrameIndex;
    uint32 FrameIndexA = FMath::FloorToInt32(ScaledFrameIndex);
    uint32 FrameIndexB = FMath::CeilToInt32(ScaledFrameIndex);
    float BlendFactorA = ScaledFrameIndex - FrameIndexA;
    float BlendFactorB = 1.f - BlendFactorA;

    const TArrayView<float> WaveTableSamplesView = WaveTable->GetSamples();
    for (int32 i = 0; i < FrameSize; i++) {
      float SampleA = AllBankSamples->GetData()[i + FrameSize * FrameIndexA];
      float SampleB = AllBankSamples->GetData()[i + FrameSize * FrameIndexB];
      WaveTableSamplesView[i] = SampleA * BlendFactorA + SampleB * BlendFactorB;
    }
    WaveTable->SetFinalValue(WaveTableSamplesView[0]); // to be consistent with FWaveTable::SetData

    PrevNormalizedFrameIndexIndex = *NormalizedFrameIndex;
  }
};

#pragma endregion

#pragma region Node

using FGetWaveTableNode = TNodeFacade<FGetWaveTableOperator>;

METASOUND_REGISTER_NODE(FGetWaveTableNode)

#pragma endregion

}

#undef LOCTEXT_NAMESPACE
