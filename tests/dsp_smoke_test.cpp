#include "arsonkupik_dsp.hpp"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {
double peak_db(const std::vector<float>& l, const std::vector<float>& r)
{
    double p = 0.0;
    for (std::size_t i = 0; i < l.size(); ++i) {
        p = std::max(p, std::abs(static_cast<double>(l[i])));
        p = std::max(p, std::abs(static_cast<double>(r[i])));
    }
    return arsonkupik::gain_to_db(p + 1.0e-12);
}
}

int main()
{
    using namespace arsonkupik;
    constexpr double sr = 48000.0;
    constexpr double kPi = 3.141592653589793238462643383279502884;
    constexpr std::size_t frames = 48000;
    std::vector<float> source_left(frames), source_right(frames);
    for (std::size_t i = 0; i < frames; ++i) {
        const double t = static_cast<double>(i) / sr;
        // Mixed stereo material: bass + centered vocal-ish tone + stereo air/detail.
        source_left[i] = static_cast<float>(0.11 * std::sin(2.0 * kPi * 60.0 * t)
                                      + 0.08 * std::sin(2.0 * kPi * 880.0 * t)
                                      + 0.04 * std::sin(2.0 * kPi * 6500.0 * t)
                                      + 0.018 * std::sin(2.0 * kPi * 2600.0 * t));
        source_right[i] = static_cast<float>(0.10 * std::sin(2.0 * kPi * 61.0 * t + 0.20)
                                       + 0.07 * std::sin(2.0 * kPi * 880.0 * t - 0.15)
                                       + 0.035 * std::sin(2.0 * kPi * 7000.0 * t)
                                       + 0.018 * std::sin(2.0 * kPi * 2600.0 * t));
    }
    const double input_peak = peak_db(source_left, source_right);
    bool failed = false;
    for (const auto& preset : factory_presets()) {
        std::vector<float> left = source_left;
        std::vector<float> right = source_right;
        float* planes[2] = { left.data(), right.data() };

        RuntimeParams params;
        params.preset_id = preset.id;
        params.enhance = preset.macro.enhance;
        params.smart_bass = preset.macro.smart_bass;
        params.smart_treble = preset.macro.smart_treble;
        params.vocal_body = preset.macro.vocal_body;
        params.stereo_magic = preset.macro.stereo_magic;
        params.output_trim_db = preset.output.output_gain_db;
        params.smart_protect = preset.macro.smart_protect;
        params.bypass = false;
        params.advanced_override = false;

        ArSonKuPikEngine engine;
        engine.prepare(sr, 2);
        engine.set_runtime_params(params);
        engine.process(planes, 2, frames);
        const auto& m = engine.meters();
        const double out_peak = peak_db(left, right);
        const double benefit = out_peak - input_peak;
        std::cout << preset.name
                  << " input_peak_db=" << input_peak
                  << " output_peak_db=" << out_peak
                  << " benefit_db=" << benefit
                  << " meter_gr_db=" << m.gain_reduction_db
                  << " corr=" << m.correlation
                  << " clipping=" << (m.clipping ? "yes" : "no")
                  << "\n";
        const bool defaultLevelOutsideTarget = preset.id == "default" && (benefit < 2.0 || benefit > 4.5);
        const bool extremeHiddenVolumeBoost = benefit > 5.0;
        if (!std::isfinite(out_peak) || out_peak > -0.20 || m.clipping
            || defaultLevelOutsideTarget || extremeHiddenVolumeBoost) {
            failed = true;
        }
    }
    return failed ? 2 : 0;
}
