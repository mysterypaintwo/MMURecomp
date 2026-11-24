#pragma once

#include "mmu/core/PackedArchive.h"

#include <SDL.h>

#include <string>
#include <string_view>
#include <vector>

namespace mmu {
namespace sdl2 {

struct AudioProbe {
    bool ok = false;
    int sampleRate = 0;
    int channels = 0;
    std::size_t frames = 0;
    std::size_t decodedSamples = 0;
};

// Cheap header probe plus a full decode, used by --selftest to verify that
// baked music blobs are valid Ogg Vorbis without opening an audio device.
AudioProbe probeMusic(const PackedEntry& entry);

struct AudioStatus {
    bool opened = false;
    int frequency = 0;
    int channels = 0;
    int activeVoices = 0;
    std::size_t voiceCount = 0;
    int masterVolume = 0;
    std::string music;
};

// Platform-side audio playback. Core stays SDL-free: this engine only consumes
// PackedEntry blobs. SFX are the original raw 16-bit PCM; music is Ogg Vorbis
// decoded with the vendored stb_vorbis. Both are mixed in one SDL callback.
class AudioEngine {
public:
    AudioEngine() = default;
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    bool open(int frequency = 44100, int channels = 2);
    void close();
    bool opened() const { return device_ != 0; }

    void setMasterVolume(int volume); // 0..128
    int masterVolume() const { return masterVolume_; }

    bool playSfx(const PackedArchive& pak, u32 key);
    bool playMusic(const PackedArchive& pak, u32 key);
    void stopAll();
    void stopMusic();
    // Stops active voices whose entry name matches, ignoring directory and
    // extension. Used by the cutscene StopSound command.
    void stopNamed(std::string_view name);

    AudioStatus status() const;

private:
    struct Voice {
        std::vector<Sint16> samples; // interleaved, already in device format
        std::size_t position = 0;
        int volume = 255;
        bool loop = false;
        bool active = false;
        std::string name;
    };

    static void SDLCALL mixCallback(void* userdata, Uint8* stream, int len);
    bool playBuffer(Voice voice);
    Voice decodeSfx(const PackedEntry& entry, int frequency, int channels) const;
    Voice decodeMusic(const PackedEntry& entry, int frequency, int channels) const;

    SDL_AudioDeviceID device_ = 0;
    SDL_AudioSpec spec_{};
    int masterVolume_ = 96;
    std::vector<Voice> voices_;
    int musicVoice_ = -1;
    std::string musicName_;
};

} // namespace sdl2
} // namespace mmu
