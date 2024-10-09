// Fill out your copyright notice in the Description page of Project Settings.


#include "MyWaveTableAsset.h"

#include "MetasoundDataTypeRegistrationMacro.h"

// look for `RegisterPinType` call in MyCustomNodesEditor.cpp
REGISTER_METASOUND_DATATYPE(MyCustomNodes::FMyWaveTableAsset, "MyWaveTableAsset", Metasound::ELiteralType::UObjectProxy, UMyWaveTable);

namespace MyCustomNodes
{
using namespace Metasound;

FMyWaveTableAsset::FMyWaveTableAsset(const TSharedPtr<Audio::IProxyData>& InInitData)
{
  if (InInitData.IsValid())
  {
    if (InInitData->CheckTypeCast<FMyWaveTableProxy>())
    {
      // should we be getting handed a SharedPtr here?
      MyWaveTableProxy = MakeShared<FMyWaveTableProxy, ESPMode::ThreadSafe>(InInitData->GetAs<FMyWaveTableProxy>());
    }
  }
}

}
