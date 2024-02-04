#include "NglSoundGenerator.h"

NglSoundGenerator::NglSoundGenerator() {
    mSoloud.init();
    mSound.load("sound.mp3");
    int soundHandle = mSoloud.play(mSound, 0.15f);
    mSoloud.setLooping(soundHandle, true);
}

NglSoundGenerator::~NglSoundGenerator() {
    mSoloud.deinit();
}
