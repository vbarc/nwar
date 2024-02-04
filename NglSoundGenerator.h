#pragma once

#include <soloud.h>
#include <soloud_wav.h>

class NglSoundGenerator {
public:
    NglSoundGenerator();
    NglSoundGenerator(const NglSoundGenerator&) = delete;
    NglSoundGenerator& operator=(const NglSoundGenerator&) = delete;
    NglSoundGenerator(NglSoundGenerator&&) = delete;
    NglSoundGenerator& operator=(NglSoundGenerator&&) = delete;
    ~NglSoundGenerator();

private:
    SoLoud::Soloud mSoloud;
    SoLoud::Wav mSound;
};
