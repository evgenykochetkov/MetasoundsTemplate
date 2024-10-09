// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MetasoundDataReference.h"
#include "MetasoundDataTypeRegistrationMacro.h"
#include "IAudioProxyInitializer.h"

#include "MyWaveTable.h"

namespace MyCustomNodes {

class MYCUSTOMNODES_API FMyWaveTableAsset {
  FMyWaveTableProxyPtr MyWaveTableProxy;

public:
  FMyWaveTableAsset() = default;
  FMyWaveTableAsset(const FMyWaveTableAsset &) = default;
  FMyWaveTableAsset &operator=(const FMyWaveTableAsset &Other) = default;

  FMyWaveTableAsset(const TSharedPtr<Audio::IProxyData> &InInitData);

  const FMyWaveTableProxyPtr &GetWaveTableProxy() const {
    return MyWaveTableProxy;
  }

  const FMyWaveTableProxy *operator->() const {
    return MyWaveTableProxy.Get();
  }

  FMyWaveTableProxy *operator->() {
    return MyWaveTableProxy.Get();
  }
};

// Declare aliases IN the namespace...
DECLARE_METASOUND_DATA_REFERENCE_ALIAS_TYPES(FMyWaveTableAsset,
                                             FMyWaveTableAssetTypeInfo,
                                             FMyWaveTableAssetReadRef,
                                             FMyWaveTableAssetWriteRef)
}

// Declare reference types OUT of the namespace...
DECLARE_METASOUND_DATA_REFERENCE_TYPES_NO_ALIASES(MyCustomNodes::FMyWaveTableAsset,
                                                  MYCUSTOMNODES_API)
