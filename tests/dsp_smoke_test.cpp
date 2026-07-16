#include "arsonkupik_dsp.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {
constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr double kSampleRate = 48000.0;
constexpr std::size_t kFrames = 48000;
constexpr std::size_t kBlock = 256;
constexpr std::size_t kWarmup = 9600;

struct StereoBuffer {
    std::vector<float> left;
    std::vector<float> right;
};

struct Metrics {
    double peak_db = -120.0;
    double rms_db = -120.0;
    double crest_db = 0.0;
};

Metrics measure(const StereoBuffer& buffer, std::size_t begin)
{
    double peak = 0.0;
    double sum = 0.0;
    std::size_t count = 0;
    for (std::size_t i = begin; i < buffer.left.size(); ++i) {
        const double l = buffer.left[i];
        const double r = buffer.right[i];
        peak = std::max(peak, std::max(std::abs(l), std::abs(r)));
        sum += l * l + r * r;
        count += 2;
    }
    const double rms = std::sqrt(sum / std::max<std::size_t>(1, count));
    Metrics m;
    m.peak_db = arsonkupik::gain_to_db(peak + 1.0e-12);
    m.rms_db = arsonkupik::gain_to_db(rms + 1.0e-12);
    m.crest_db = m.peak_db - m.rms_db;
    return m;
}

bool finite_buffer(const StereoBuffer& buffer)
{
    for (std::size_t i = 0; i < buffer.left.size(); ++i) {
        if (!std::isfinite(buffer.left[i]) || !std::isfinite(buffer.right[i])) return false;
    }
    return true;
}

StereoBuffer make_reference_source()
{
    StereoBuffer source{std::vector<float>(kFrames), std::vector<float>(kFrames)};
    for (std::size_t i = 0; i < kFrames; ++i) {
        const double t = static_cast<double>(i) / kSampleRate;
        const std::size_t transient_pos = i % 12000U;
        const double transient_env = transient_pos < 700U
            ? std::exp(-static_cast<double>(transient_pos) / 115.0)
            : 0.0;
        const double transient = 0.32 * transient_env
                               * std::sin(2.0 * kPi * 92.0 * t);

        source.left[i] = static_cast<float>(
            0.11 * std::sin(2.0 * kPi * 60.0 * t)
          + 0.08 * std::sin(2.0 * kPi * 880.0 * t)
          + 0.04 * std::sin(2.0 * kPi * 6500.0 * t)
          + 0.018 * std::sin(2.0 * kPi * 2600.0 * t)
          + transient);
        source.right[i] = static_cast<float>(
            0.10 * std::sin(2.0 * kPi * 61.0 * t + 0.20)
          + 0.07 * std::sin(2.0 * kPi * 880.0 * t - 0.15)
          + 0.035 * std::sin(2.0 * kPi * 7000.0 * t)
          + 0.018 * std::sin(2.0 * kPi * 2600.0 * t)
          - transient * 0.72);
    }
    return source;
}

StereoBuffer scale_to_peak(const StereoBuffer& source, double target_peak_db)
{
    const Metrics source_metrics = measure(source, 0);
    const double gain = arsonkupik::db_to_gain(target_peak_db - source_metrics.peak_db);
    StereoBuffer scaled = source;
    for (std::size_t i = 0; i < scaled.left.size(); ++i) {
        scaled.left[i] = static_cast<float>(scaled.left[i] * gain);
        scaled.right[i] = static_cast<float>(scaled.right[i] * gain);
    }
    return scaled;
}

arsonkupik::RuntimeParams runtime_for(const arsonkupik::Preset& preset)
{
    arsonkupik::RuntimeParams params;
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
    return params;
}

double percentile95(std::vector<double> values)
{
    if (values.empty()) return 0.0;
    std::sort(values.begin(), values.end());
    const std::size_t index = std::min(values.size() - 1,
        static_cast<std::size_t>(std::ceil(values.size() * 0.95)) - 1);
    return values[index];
}

bool level_gate(double input_peak_db, double rms_benefit_db,
                double peak_benefit_db, double crest_loss_db,
                double gr95_db)
{
    if (input_peak_db <= -12.0) {
        return rms_benefit_db >= 2.70 && rms_benefit_db <= 3.65
            && peak_benefit_db >= 3.15 && peak_benefit_db <= 4.95
            && crest_loss_db <= 2.25
            && gr95_db <= 4.50;
    }
    if (input_peak_db <= -6.0) {
        return rms_benefit_db >= 1.20 && rms_benefit_db <= 3.65
            && crest_loss_db <= 4.00
            && gr95_db <= 6.00;
    }
    if (input_peak_db <= -3.0) {
        return rms_benefit_db >= 0.20 && rms_benefit_db <= 3.40
            && crest_loss_db <= 5.00
            && gr95_db <= 7.00;
    }
    return rms_benefit_db >= -0.50 && rms_benefit_db <= 2.80
        && crest_loss_db <= 6.00
        && gr95_db <= 8.00;
}
} // namespace

int main()
{
    using namespace arsonkupik;
    constexpr std::array<double, 5> input_peak_levels = {-18.0, -12.0, -6.0, -3.0, -1.0};
    const StereoBuffer reference = make_reference_source();
    bool failed = false;

    std::cout << "multi_level_loudness_matrix peak_levels_db=[-18,-12,-6,-3,-1]"
              << " moderate_rms_target_db=[2.70,3.65] output_peak_ceiling_db=-0.20\n";

    for (const auto& preset : factory_presets()) {
        for (double target_peak_db : input_peak_levels) {
            StereoBuffer input = scale_to_peak(reference, target_peak_db);
            StereoBuffer output = input;
            const Metrics input_metrics = measure(input, kWarmup);

            ArSonKuPikEngine engine;
            engine.set_runtime_params(runtime_for(preset));
            engine.prepare(kSampleRate, 2);

            std::vector<double> gr_samples;
            bool meter_clipping = false;
            for (std::size_t offset = 0; offset < kFrames; offset += kBlock) {
                const std::size_t count = std::min(kBlock, kFrames - offset);
                float* planes[2] = {output.left.data() + offset, output.right.data() + offset};
                engine.process(planes, 2, count);
                if (offset >= kWarmup) {
                    gr_samples.push_back(engine.meters().gain_reduction_db);
                    meter_clipping = meter_clipping || engine.meters().clipping;
                }
            }

            const Metrics output_metrics = measure(output, kWarmup);
            const double rms_benefit = output_metrics.rms_db - input_metrics.rms_db;
            const double peak_benefit = output_metrics.peak_db - input_metrics.peak_db;
            const double crest_loss = input_metrics.crest_db - output_metrics.crest_db;
            const double gr95 = percentile95(gr_samples);
            const bool level_ok = level_gate(target_peak_db, rms_benefit,
                                             peak_benefit, crest_loss, gr95);
            const bool safe = finite_buffer(output)
                           && output_metrics.peak_db <= -0.20
                           && !meter_clipping;
            const bool ok = safe && level_ok;

            std::cout << preset.name
                      << " input_target_peak_db=" << target_peak_db
                      << " input_peak_db=" << input_metrics.peak_db
                      << " output_peak_db=" << output_metrics.peak_db
                      << " peak_benefit_db=" << peak_benefit
                      << " input_rms_db=" << input_metrics.rms_db
                      << " output_rms_db=" << output_metrics.rms_db
                      << " rms_benefit_db=" << rms_benefit
                      << " crest_loss_db=" << crest_loss
                      << " gr95_db=" << gr95
                      << " clipping=" << (meter_clipping ? "yes" : "no")
                      << " result=" << (ok ? "pass" : "fail")
                      << "\n";

            failed = failed || !ok;
        }
    }

    return failed ? 2 : 0;
}
