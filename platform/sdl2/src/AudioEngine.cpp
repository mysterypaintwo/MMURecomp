#include "mmu/sdl2/AudioEngine.h"

#include "stb_vorbis_decl.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>

namespace mmu {
namespace sdl2 {
namespace {

constexpr int kAssumedSfxRate = 44100; // original used Mix_QuickLoad_RAW's device format

Sint16 clamp16(int value) {
    return static_cast<Sint16>(std::max(-32768, std::min(32767, value)));
}

Sint16 readLE16(const u8* p) {
    return static_cast<Sint16>(static_cast<u16>(p[0]) | static_cast<u16>(p[1]) << 8);
}

std::string audioBaseName(std::string_view text) {
    std::string out(text);
    for (char& c : out) {
        if (c == '\\') c = '/';
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    const std::size_t slash = out.find_last_of('/');
    if (slash != std::string::npos) out.erase(0, slash + 1);
    const std::size_t dot = out.find_last_of('.');
    if (dot != std::string::npos) out.erase(dot);
    return out;
}

std::vector<Sint16> resample(const Sint16* input, std::size_t frames,
                             int srcChannels, int srcRate,
                             int dstChannels, int dstRate) {
    if (!input || !frames || srcChannels <= 0 || dstChannels <= 0 ||
        srcRate <= 0 || dstRate <= 0)
        return {};

    if (srcRate == dstRate && srcChannels == dstChannels)
        return std::vector<Sint16>(input, input + frames * srcChannels);

    const double step = static_cast<double>(srcRate) / static_cast<double>(dstRate);
    std::vector<Sint16> output(static_cast<std::size_t>(
        static_cast<double>(frames) / step + 0.5) * dstChannels);
    for (std::size_t dst = 0; dst < output.size() / static_cast<std::size_t>(dstChannels); ++dst) {
        const double sourcePos = static_cast<double>(dst) * step;
        const std::size_t first = static_cast<std::size_t>(sourcePos);
        if (first >= frames) break;
        const std::size_t next = std::min(first + 1, frames - 1);
        const double fraction = sourcePos - static_cast<double>(first);
        for (int channel = 0; channel < dstChannels; ++channel) {
            const int srcChannel = std::min(channel, srcChannels - 1);
            const int a = input[first * srcChannels + srcChannel];
            const int b = input[next * srcChannels + srcChannel];
            output[dst * dstChannels + channel] =
                clamp16(static_cast<int>(a + (b - a) * fraction));
        }
    }
    return output;
}

} // namespace

AudioProbe probeMusic(const PackedEntry& entry) {
    AudioProbe probe;
    if (entry.data.size > static_cast<std::size_t>(0x7FFFFFFF)) return probe;
    int error = 0;
    stb_vorbis* stream = stb_vorbis_open_memory(
        entry.data.data, static_cast<int>(entry.data.size), &error, nullptr);
    if (!stream) return probe;
    const stb_vorbis_info info = stb_vorbis_get_info(stream);
    probe.sampleRate = info.sample_rate;
    probe.channels = info.channels;
    probe.frames = static_cast<std::size_t>(
        stb_vorbis_stream_length_in_samples(stream));
    stb_vorbis_close(stream);

    int decodedChannels = 0, decodedRate = 0;
    short* decoded = nullptr;
    const int samples = stb_vorbis_decode_memory(
        entry.data.data, static_cast<int>(entry.data.size),
        &decodedChannels, &decodedRate, &decoded);
    if (samples > 0 && decoded) {
        probe.ok = true;
        probe.decodedSamples = static_cast<std::size_t>(samples);
        std::free(decoded);
    }
    return probe;
}

AudioEngine::~AudioEngine() { close(); }

bool AudioEngine::open(int frequency, int channels) {
    close();
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    SDL_AudioSpec wanted{};
    wanted.freq = frequency;
    wanted.format = AUDIO_S16SYS;
    wanted.channels = static_cast<u8>(channels);
    wanted.samples = 2048;
    wanted.callback = &AudioEngine::mixCallback;
    wanted.userdata = this;

    SDL_AudioSpec obtained{};
    device_ = SDL_OpenAudioDevice(nullptr, 0, &wanted, &obtained, 0);
    if (!device_) return false;
    spec_ = obtained;
    SDL_PauseAudioDevice(device_, 0);
    return true;
}

void AudioEngine::close() {
    if (device_) {
        SDL_PauseAudioDevice(device_, 1);
        SDL_CloseAudioDevice(device_);
        device_ = 0;
    }
    voices_.clear();
    musicVoice_ = -1;
    musicName_.clear();
}

void AudioEngine::setMasterVolume(int volume) {
    masterVolume_ = std::max(0, std::min(128, volume));
}

AudioEngine::Voice AudioEngine::decodeSfx(const PackedEntry& entry,
                                          int frequency, int channels) const {
    Voice voice;
    voice.name = entry.name;
    voice.volume = entry.volume
        ? 255 * std::min<u32>(entry.volume, 128) / 128 : 255;

    std::size_t bytes = entry.audioLength
        ? std::min<std::size_t>(entry.audioLength, entry.data.size)
        : entry.data.size;
    bytes -= bytes % 2;
    const std::size_t frames = bytes / 2;
    if (!frames) return voice;

    std::vector<Sint16> monoOrStereo(frames);
    for (std::size_t i = 0; i < frames; ++i)
        monoOrStereo[i] = readLE16(entry.data.data + i * 2);

    // The original sound bank is 16-bit stereo at the mixer's default rate.
    voice.samples = resample(monoOrStereo.data(), frames / 2, 2,
                             kAssumedSfxRate, channels, frequency);
    return voice;
}

AudioEngine::Voice AudioEngine::decodeMusic(const PackedEntry& entry,
                                            int frequency, int channels) const {
    Voice voice;
    voice.name = entry.name;
    voice.loop = true;
    voice.volume = entry.volume
        ? 255 * std::min<u32>(entry.volume, 128) / 128 : 255;
    if (entry.data.size > static_cast<std::size_t>(0x7FFFFFFF)) return voice;

    int srcChannels = 0;
    int srcRate = 0;
    short* decoded = nullptr;
    const int samples = stb_vorbis_decode_memory(
        entry.data.data, static_cast<int>(entry.data.size),
        &srcChannels, &srcRate, &decoded);
    if (samples <= 0 || !decoded) return voice;

    // stb returns frames (samples per channel); the buffer is interleaved.
    voice.samples = resample(decoded, static_cast<std::size_t>(samples),
                             srcChannels, srcRate, channels, frequency);
    std::free(decoded);
    return voice;
}

bool AudioEngine::playBuffer(Voice voice) {
    if (!device_ || voice.samples.empty()) return false;
    voice.position = 0;
    voice.active = true;

    SDL_LockAudioDevice(device_);
    if (voices_.size() >= 64) {
        const auto victim = std::find_if(voices_.begin(), voices_.end(),
            [](const Voice& v) { return !v.loop && !v.active; });
        if (victim != voices_.end()) voices_.erase(victim);
        else if (voices_.size() >= 64) voices_.erase(voices_.begin());
    }
    voices_.push_back(std::move(voice));
    SDL_UnlockAudioDevice(device_);
    return true;
}

bool AudioEngine::playSfx(const PackedArchive& pak, u32 key) {
    if (!device_) return false;
    const PackedEntry* entry = pak.find(key, PackedEntryType::Audio);
    if (!entry) return false;
    return playBuffer(decodeSfx(*entry, spec_.freq, spec_.channels));
}

bool AudioEngine::playMusic(const PackedArchive& pak, u32 key) {
    if (!device_) return false;
    const PackedEntry* entry = pak.find(key, PackedEntryType::Music);
    if (!entry) return false;
    Voice voice = decodeMusic(*entry, spec_.freq, spec_.channels);
    if (voice.samples.empty()) return false;
    // Unlike SFX, music bypasses playBuffer() so it can be tracked separately;
    // activate it here or the mix callback never produces any samples.
    voice.position = 0;
    voice.active = true;

    SDL_LockAudioDevice(device_);
    if (musicVoice_ >= 0 && static_cast<std::size_t>(musicVoice_) < voices_.size())
        voices_[musicVoice_].active = false;
    voices_.push_back(std::move(voice));
    musicVoice_ = static_cast<int>(voices_.size()) - 1;
    musicName_ = entry->name;
    SDL_UnlockAudioDevice(device_);
    return true;
}

void AudioEngine::stopMusic() {
    if (!device_) return;
    SDL_LockAudioDevice(device_);
    if (musicVoice_ >= 0 && static_cast<std::size_t>(musicVoice_) < voices_.size())
        voices_[musicVoice_].active = false;
    musicVoice_ = -1;
    musicName_.clear();
    SDL_UnlockAudioDevice(device_);
}

void AudioEngine::stopNamed(std::string_view name) {
    if (!device_) return;
    const std::string needle = audioBaseName(name);
    if (needle.empty()) return;
    SDL_LockAudioDevice(device_);
    for (std::size_t i = 0; i < voices_.size(); ++i) {
        Voice& voice = voices_[i];
        if (!voice.active || audioBaseName(voice.name) != needle) continue;
        voice.active = false;
        if (static_cast<int>(i) == musicVoice_) {
            musicVoice_ = -1;
            musicName_.clear();
        }
    }
    SDL_UnlockAudioDevice(device_);
}

void AudioEngine::stopAll() {
    if (!device_) return;
    SDL_LockAudioDevice(device_);
    for (Voice& voice : voices_) voice.active = false;
    musicVoice_ = -1;
    musicName_.clear();
    SDL_UnlockAudioDevice(device_);
}

AudioStatus AudioEngine::status() const {
    AudioStatus status;
    status.opened = opened();
    status.frequency = spec_.freq;
    status.channels = spec_.channels;
    status.masterVolume = masterVolume_;
    status.music = musicName_;
    int active = 0;
    for (const Voice& voice : voices_)
        if (voice.active) ++active;
    status.activeVoices = active;
    status.voiceCount = voices_.size();
    return status;
}

void SDLCALL AudioEngine::mixCallback(void* userdata, Uint8* stream, int len) {
    AudioEngine* self = static_cast<AudioEngine*>(userdata);
    if (!self) return;
    std::memset(stream, 0, static_cast<std::size_t>(len));

    const int channels = self->spec_.channels;
    const int bytesPerFrame = channels * static_cast<int>(sizeof(Sint16));
    const int frames = len / bytesPerFrame;
    auto* output = reinterpret_cast<Sint16*>(stream);

    for (Voice& voice : self->voices_) {
        if (!voice.active) continue;
        for (int frame = 0; frame < frames; ++frame) {
            if (voice.position >= voice.samples.size()) {
                if (voice.loop) voice.position = 0;
                else { voice.active = false; break; }
            }
            for (int channel = 0; channel < channels; ++channel) {
                const int sample = voice.samples[voice.position++];
                const int mixed = output[frame * channels + channel]
                    + (sample * voice.volume * self->masterVolume_) / (255 * 128);
                output[frame * channels + channel] = clamp16(mixed);
            }
        }
    }
}

} // namespace sdl2
} // namespace mmu
