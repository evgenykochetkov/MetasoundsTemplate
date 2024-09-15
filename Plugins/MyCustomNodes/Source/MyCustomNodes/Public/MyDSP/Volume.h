#pragma once

#include "HAL/Platform.h"

namespace MyDSP {
class FVolume {
public:
  void Init();

  void SetAmplitude(const float InAmount);
  void ProcessAudioBuffer(const float *InBuffer,
                          float *OutBuffer,
                          const int32 InNumSamples);

private:
  float Amplitude = 0.0f;
};
} // namespace MyDSP
