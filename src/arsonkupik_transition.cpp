#include "arsonkupik_transition.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace arsonkupik {
namespace {
constexpr double kPi = 3.1415926535897932384626433832795;
}

bool ArSonKuPikTransitionProcessor::params_close(const EngineParams& a,
                                                 const EngineParams& b)
{
    const auto near = [](double x, double y, double eps) {
        return std::abs(x - y) <= eps;
    };
    return a.preset_index == b.preset_index
        && a.advanced_override == b.advanced_override
        && a.smart_protect == b.smart_protect
        && a.bypass == b.bypass
        && near(a.enhance, b.enhance, 0.020)
        && near(a.smart_bass, b.smart_bass, 0.020)
        && near(a.smart_treble, b.smart_treble, 0.020)
        && near(a.vocal_body, b.vocal_body, 0.020)
        && near(a.stereo_magic, b.stereo_magic, 0.020)
        && near(a.output_trim_db, b.output_trim_db, 0.005);
}

bool ArSonKuPikTransitionProcessor::smooth_params(EngineParams& current,
                                                  const EngineParams& target,
                                                  std::size_t frames,
                                                  double sample_rate)
{
    const EngineParams before = current;
    const double denom = std::max(1.0, sample_rate * kKnobSmoothingSeconds);
    const double alpha = 1.0 - std::exp(-static_cast<double>(frames) / denom);
    const auto smooth = [alpha](double value, double target_value, double snap_eps) {
        const double next = value + (target_value - value) * alpha;
        return std::abs(next - target_value) <= snap_eps ? target_value : next;
    };

    current.enhance = smooth(current.enhance, target.enhance, 0.010);
    current.smart_bass = smooth(current.smart_bass, target.smart_bass, 0.010);
    current.smart_treble = smooth(current.smart_treble, target.smart_treble, 0.010);
    current.vocal_body = smooth(current.vocal_body, target.vocal_body, 0.010);
    current.stereo_magic = smooth(current.stereo_magic, target.stereo_magic, 0.010);
    current.output_trim_db = smooth(current.output_trim_db, target.output_trim_db, 0.003);
    current.smart_protect = target.smart_protect;
    current.bypass = target.bypass;
    current.advanced_override = target.advanced_override;
    return !params_close(before, current);
}

void ArSonKuPikTransitionProcessor::prepare(double sample_rate,
                                            std::size_t channels,
                                            const EngineParams& initial)
{
    sample_rate_ = sample_rate > 0.0 ? sample_rate : kDefaultSampleRate;
    channels_ = std::max<std::size_t>(1, std::min<std::size_t>(channels, kMaxChannels));

    for (auto& engine : engines_) {
        engine.set_realtime_params(initial);
        engine.prepare(sample_rate_, channels_);
    }

    active_engine_ = 0;
    staging_engine_ = 1;
    active_params_ = initial;
    staging_params_ = initial;
    crossfade_active_ = false;
    crossfade_position_ = 0;
    crossfade_total_ = 1;
}

void ArSonKuPikTransitionProcessor::begin_preset_crossfade(
    const EngineParams& requested)
{
    staging_params_ = requested;
    auto& staging = engines_[staging_engine_];
    staging.set_realtime_params(staging_params_);
    staging.reset();

    crossfade_total_ = std::max<std::size_t>(
        1, static_cast<std::size_t>(sample_rate_ * kPresetCrossfadeSeconds));
    crossfade_position_ = 0;
    crossfade_active_ = true;
}

void ArSonKuPikTransitionProcessor::process_crossfade(float** planes,
                                                      std::size_t channels,
                                                      std::size_t frames)
{
    auto& active = engines_[active_engine_];
    auto& staging = engines_[staging_engine_];

    for (std::size_t offset = 0; offset < frames; offset += kTransitionChunkFrames) {
        const std::size_t count = std::min(kTransitionChunkFrames, frames - offset);
        float* active_planes[kMaxChannels]{};
        float* staging_planes[kMaxChannels]{};

        for (std::size_t ch = 0; ch < channels; ++ch) {
            if (!planes[ch]) continue;
            active_planes[ch] = planes[ch] + offset;
            staging_planes[ch] = staging_audio_[ch].data();
            std::copy_n(active_planes[ch], count, staging_planes[ch]);
        }

        active.process(active_planes, channels, count);
        staging.process(staging_planes, channels, count);

        for (std::size_t i = 0; i < count; ++i) {
            const std::size_t absolute = crossfade_position_ + i;
            const double t = crossfade_total_ <= 1
                ? 1.0
                : std::min(1.0, static_cast<double>(absolute)
                                  / static_cast<double>(crossfade_total_ - 1));
            const double old_gain = std::cos(t * kPi * 0.5);
            const double new_gain = std::sin(t * kPi * 0.5);
            const double normalization = 1.0 / std::max(1.0, old_gain + new_gain);
            const std::size_t processed_channels = std::min<std::size_t>(channels, 2);

            for (std::size_t ch = 0; ch < processed_channels; ++ch) {
                if (!active_planes[ch] || !staging_planes[ch]) continue;
                active_planes[ch][i] = static_cast<float>((
                    static_cast<double>(active_planes[ch][i]) * old_gain
                    + static_cast<double>(staging_planes[ch][i]) * new_gain)
                    * normalization);
            }
        }

        crossfade_position_ += count;
    }

    if (crossfade_position_ >= crossfade_total_) {
        std::swap(active_engine_, staging_engine_);
        active_params_ = staging_params_;
        crossfade_active_ = false;
        crossfade_position_ = 0;
    }
}

void ArSonKuPikTransitionProcessor::process(float** planes,
                                            std::size_t channels,
                                            std::size_t frames,
                                            const EngineParams& requested)
{
    if (!planes || channels == 0 || frames == 0) return;
    channels = std::min<std::size_t>(channels, channels_);
    if (channels == 0 || !planes[0]) return;

    if (!crossfade_active_ && requested.preset_index != active_params_.preset_index) {
        begin_preset_crossfade(requested);
    }

    if (crossfade_active_) {
        // A third preset request is intentionally queued by the caller until the
        // current 10 ms transition finishes. This avoids restarting the blend
        // from a discontinuous intermediate signal. The latest target is picked
        // up on the next audio block.
        if (requested.preset_index == staging_params_.preset_index
            && smooth_params(staging_params_, requested, frames, sample_rate_)) {
            engines_[staging_engine_].set_realtime_params(staging_params_);
        }
        process_crossfade(planes, channels, frames);
        return;
    }

    if (smooth_params(active_params_, requested, frames, sample_rate_)) {
        engines_[active_engine_].set_realtime_params(active_params_);
    }
    engines_[active_engine_].process(planes, channels, frames);
}

} // namespace arsonkupik
