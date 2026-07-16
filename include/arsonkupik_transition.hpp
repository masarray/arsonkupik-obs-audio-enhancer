#pragma once

#include "arsonkupik_dsp.hpp"

#include <array>
#include <cstddef>

namespace arsonkupik {

// Shared preset/parameter transition layer used by the OBS wrapper and the
// standalone regression tests. All storage is fixed after construction so the
// audio callback does not allocate while knobs, bypass, or presets change.
class ArSonKuPikTransitionProcessor {
public:
    void prepare(double sample_rate, std::size_t channels, const EngineParams& initial);
    void process(float** planes, std::size_t channels, std::size_t frames,
                 const EngineParams& requested);

    const EngineParams& active_params() const { return active_params_; }
    bool crossfade_active() const { return crossfade_active_; }

private:
    static constexpr double kKnobSmoothingSeconds = 0.080;
    static constexpr double kPresetCrossfadeSeconds = 0.010;
    static constexpr std::size_t kTransitionChunkFrames = 2048;

    static bool params_close(const EngineParams& a, const EngineParams& b);
    static bool smooth_params(EngineParams& current, const EngineParams& target,
                              std::size_t frames, double sample_rate);

    void begin_preset_crossfade(const EngineParams& requested);
    void process_crossfade(float** planes, std::size_t channels, std::size_t frames);

    std::array<ArSonKuPikEngine, 2> engines_;
    std::size_t active_engine_ = 0;
    std::size_t staging_engine_ = 1;
    EngineParams active_params_{};
    EngineParams staging_params_{};
    double sample_rate_ = kDefaultSampleRate;
    std::size_t channels_ = 2;
    bool crossfade_active_ = false;
    std::size_t crossfade_position_ = 0;
    std::size_t crossfade_total_ = 1;
    std::array<std::array<float, kTransitionChunkFrames>, kMaxChannels> staging_audio_{};
};

} // namespace arsonkupik
