#include "arsonkupik_dsp.hpp"

#include <cmath>
#include <iostream>
#include <vector>

int main()
{
    using namespace arsonkupik;
    constexpr double sr = 48000.0;
    constexpr double kPi = 3.141592653589793238462643383279502884;
    constexpr std::size_t frames = 48000;
    std::vector<float> left(frames), right(frames);
    for (std::size_t i = 0; i < frames; ++i) {
        double t = static_cast<double>(i) / sr;
        left[i] = static_cast<float>(0.11 * std::sin(2.0 * kPi * 60.0 * t)
                                  + 0.08 * std::sin(2.0 * kPi * 880.0 * t)
                                  + 0.04 * std::sin(2.0 * kPi * 6500.0 * t));
        right[i] = static_cast<float>(0.10 * std::sin(2.0 * kPi * 61.0 * t + 0.20)
                                   + 0.07 * std::sin(2.0 * kPi * 880.0 * t - 0.15)
                                   + 0.035 * std::sin(2.0 * kPi * 7000.0 * t));
    }
    float* planes[2] = { left.data(), right.data() };
    RuntimeParams params;
    params.preset_id = "default";
    params.enhance = 65;
    params.smart_bass = 55;
    params.smart_treble = 70;
    params.vocal_body = 65;
    params.stereo_magic = 85;
    params.output_trim_db = -1.0;
    params.smart_protect = true;
    params.bypass = false;

    ArSonKuPikEngine engine;
    engine.prepare(sr, 2);
    engine.set_runtime_params(params);
    engine.process(planes, 2, frames);

    const auto& m = engine.meters();
    std::cout << "preset=" << engine.current_preset().name << "\n";
    std::cout << "input_peak_db=" << m.input_peak_db << "\n";
    std::cout << "output_peak_db=" << m.output_peak_db << "\n";
    std::cout << "gain_reduction_db=" << m.gain_reduction_db << "\n";
    std::cout << "correlation=" << m.correlation << "\n";
    if (!std::isfinite(m.output_peak_db) || m.output_peak_db > 1.0) return 2;
    return 0;
}
