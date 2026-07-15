#include "arsonkupik_dsp.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <limits>
#include <new>
#include <vector>

namespace { std::atomic<std::size_t> g_allocations{0}; }

void* operator new(std::size_t size)
{
    g_allocations.fetch_add(1, std::memory_order_relaxed);
    if (void* p = std::malloc(size)) return p;
    throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }

namespace {
constexpr double kPi = 3.141592653589793238462643383279502884;
struct StereoBuffer { std::vector<float> l; std::vector<float> r; };

bool finite_buffer(const StereoBuffer& b)
{
    for (std::size_t i = 0; i < b.l.size(); ++i) {
        if (!std::isfinite(b.l[i]) || !std::isfinite(b.r[i])) return false;
    }
    return true;
}

double max_jump(const StereoBuffer& b)
{
    double value = 0.0;
    for (std::size_t i = 1; i < b.l.size(); ++i) {
        value = std::max(value, std::abs(static_cast<double>(b.l[i]) - b.l[i - 1]));
        value = std::max(value, std::abs(static_cast<double>(b.r[i]) - b.r[i - 1]));
    }
    return value;
}

double peak(const StereoBuffer& b)
{
    double value = 0.0;
    for (std::size_t i = 0; i < b.l.size(); ++i) {
        value = std::max(value, std::abs(static_cast<double>(b.l[i])));
        value = std::max(value, std::abs(static_cast<double>(b.r[i])));
    }
    return value;
}

arsonkupik::EngineParams default_params()
{
    using namespace arsonkupik;
    const auto& p = default_preset();
    EngineParams params;
    params.preset_index = static_cast<std::uint32_t>(preset_index_from_id(p.id));
    params.enhance = p.macro.enhance;
    params.smart_bass = p.macro.smart_bass;
    params.smart_treble = p.macro.smart_treble;
    params.vocal_body = p.macro.vocal_body;
    params.stereo_magic = p.macro.stereo_magic;
    params.output_trim_db = p.output.output_gain_db;
    params.smart_protect = true;
    params.bypass = false;
    params.advanced_override = true;
    return params;
}

StereoBuffer render_automation(bool automate)
{
    using namespace arsonkupik;
    constexpr double sr = 48000.0;
    constexpr std::size_t frames = 48000;
    constexpr std::size_t block = 256;
    StereoBuffer out{std::vector<float>(frames), std::vector<float>(frames)};
    for (std::size_t i = 0; i < frames; ++i) {
        const double t = static_cast<double>(i) / sr;
        out.l[i] = static_cast<float>(0.12 * std::sin(2.0 * kPi * 73.0 * t)
                                    + 0.08 * std::sin(2.0 * kPi * 997.0 * t)
                                    + 0.025 * std::sin(2.0 * kPi * 7600.0 * t));
        out.r[i] = static_cast<float>(0.11 * std::sin(2.0 * kPi * 75.0 * t + 0.17)
                                    + 0.075 * std::sin(2.0 * kPi * 997.0 * t - 0.12)
                                    + 0.023 * std::sin(2.0 * kPi * 7100.0 * t));
    }

    EngineParams current = default_params();
    ArSonKuPikEngine engine;
    engine.set_realtime_params(current);
    engine.prepare(sr, 2);
    for (std::size_t offset = 0; offset < frames; offset += block) {
        const std::size_t count = std::min(block, frames - offset);
        if (automate) {
            const double phase = static_cast<double>(offset) / static_cast<double>(frames - 1);
            const double target = phase < 0.5 ? phase * 200.0 : (1.0 - phase) * 200.0;
            const double alpha = 1.0 - std::exp(-static_cast<double>(count) / (sr * 0.080));
            const auto smooth = [alpha](double value, double destination) {
                return value + (destination - value) * alpha;
            };
            current.smart_bass = smooth(current.smart_bass, target);
            current.smart_treble = smooth(current.smart_treble, 100.0 - target);
            current.vocal_body = smooth(current.vocal_body, target);
            current.stereo_magic = smooth(current.stereo_magic, 100.0 - target);
            engine.set_realtime_params(current);
        }
        float* planes[2] = {out.l.data() + offset, out.r.data() + offset};
        engine.process(planes, 2, count);
    }
    return out;
}

bool test_automation()
{
    const StereoBuffer reference = render_automation(false);
    const StereoBuffer automated = render_automation(true);
    const double static_jump = max_jump(reference);
    const double automated_jump = max_jump(automated);
    const double ratio = automated_jump / std::max(1.0e-12, static_jump);
    std::cout << "automation static_jump=" << static_jump
              << " automated_jump=" << automated_jump
              << " ratio=" << ratio << "\n";
    return finite_buffer(automated) && ratio <= 1.10;
}

bool test_limiter_ceiling()
{
    using namespace arsonkupik;
    constexpr double sr = 48000.0;
    constexpr std::size_t frames = 8192;
    StereoBuffer out{std::vector<float>(frames), std::vector<float>(frames)};
    for (std::size_t i = 0; i < frames; ++i) {
        const double t = static_cast<double>(i) / sr;
        const double transient = (i % 733U) < 5U ? 1.25 : 0.0;
        out.l[i] = static_cast<float>(0.88 * std::sin(2.0 * kPi * 83.0 * t) + transient);
        out.r[i] = static_cast<float>(0.86 * std::sin(2.0 * kPi * 89.0 * t + 0.2) - transient * 0.7);
    }
    EngineParams params = default_params();
    params.output_trim_db = 6.0;
    params.smart_bass = 100.0;
    params.enhance = 100.0;
    ArSonKuPikEngine engine;
    engine.set_realtime_params(params);
    engine.prepare(sr, 2);
    float* planes[2] = {out.l.data(), out.r.data()};
    engine.process(planes, 2, frames);
    const double out_peak = peak(out);
    std::cout << "limiter peak=" << out_peak << " peak_db=" << gain_to_db(out_peak) << "\n";
    return finite_buffer(out) && out_peak <= 0.985;
}

bool test_correlation_meter()
{
    using namespace arsonkupik;
    constexpr double sr = 48000.0;
    constexpr std::size_t frames = 24000;
    auto render = [&](bool inverted) {
        StereoBuffer out{std::vector<float>(frames), std::vector<float>(frames)};
        for (std::size_t i = 0; i < frames; ++i) {
            const float x = static_cast<float>(0.12 * std::sin(2.0 * kPi * 997.0 * static_cast<double>(i) / sr));
            out.l[i] = x;
            out.r[i] = inverted ? -x : x;
        }
        EngineParams params = default_params();
        params.stereo_magic = 50.0;
        ArSonKuPikEngine engine;
        engine.set_realtime_params(params);
        engine.prepare(sr, 2);
        float* planes[2] = {out.l.data(), out.r.data()};
        engine.process(planes, 2, frames);
        return engine.meters().correlation;
    };
    const double positive = render(false);
    const double negative = render(true);
    std::cout << "correlation positive=" << positive << " negative=" << negative << "\n";
    return positive > 0.95 && negative < -0.95;
}

bool test_realtime_allocation_free()
{
    using namespace arsonkupik;
    constexpr double sr = 48000.0;
    constexpr std::size_t block = 256;
    std::vector<float> l(block, 0.05f);
    std::vector<float> r(block, -0.04f);
    EngineParams params = default_params();
    ArSonKuPikEngine engine;
    engine.set_realtime_params(params);
    engine.prepare(sr, 2);

    g_allocations.store(0, std::memory_order_relaxed);
    for (int i = 0; i < 200; ++i) {
        params.smart_bass = 50.0 + 45.0 * std::sin(static_cast<double>(i) * 0.05);
        params.smart_treble = 50.0 + 45.0 * std::cos(static_cast<double>(i) * 0.04);
        params.vocal_body = 50.0 + 40.0 * std::sin(static_cast<double>(i) * 0.03);
        params.stereo_magic = 50.0 + 45.0 * std::cos(static_cast<double>(i) * 0.02);
        engine.set_realtime_params(params);
        float* planes[2] = {l.data(), r.data()};
        engine.process(planes, 2, block);
    }
    const std::size_t allocations = g_allocations.load(std::memory_order_relaxed);
    std::cout << "realtime allocations=" << allocations << "\n";
    return allocations == 0;
}

bool test_bass_linked_pumping()
{
    using namespace arsonkupik;
    constexpr double sr = 48000.0;
    constexpr std::size_t frames = 96000;
    StereoBuffer out{std::vector<float>(frames), std::vector<float>(frames)};
    for (std::size_t i = 0; i < frames; ++i) {
        const double t = static_cast<double>(i) / sr;
        const std::size_t cycle = i % 12000U;
        const double kick_env = cycle < 1200U ? std::exp(-static_cast<double>(cycle) / 330.0) : 0.0;
        out.l[i] = static_cast<float>(0.68 * kick_env * std::sin(2.0 * kPi * 58.0 * t));
        out.r[i] = static_cast<float>(0.10 * std::sin(2.0 * kPi * 1000.0 * t));
    }
    EngineParams params = default_params();
    params.smart_bass = 90.0;
    params.enhance = 82.0;
    ArSonKuPikEngine engine;
    engine.set_realtime_params(params);
    engine.prepare(sr, 2);
    float* planes[2] = {out.l.data(), out.r.data()};
    engine.process(planes, 2, frames);

    double min_rms = std::numeric_limits<double>::max();
    double max_rms = 0.0;
    for (std::size_t cycle_start = 12000; cycle_start + 12000 <= frames; cycle_start += 12000) {
        const std::size_t begin = cycle_start + 2200;
        const std::size_t end = cycle_start + 10500;
        double sum = 0.0;
        for (std::size_t i = begin; i < end; ++i) sum += static_cast<double>(out.r[i]) * out.r[i];
        const double rms = std::sqrt(sum / static_cast<double>(end - begin));
        min_rms = std::min(min_rms, rms);
        max_rms = std::max(max_rms, rms);
    }
    const double variation_db = gain_to_db(max_rms / std::max(1.0e-12, min_rms));
    std::cout << "pumping variation_db=" << variation_db << "\n";
    return finite_buffer(out) && variation_db <= 0.35;
}
} // namespace

int main()
{
    bool ok = true;
    ok = test_automation() && ok;
    ok = test_limiter_ceiling() && ok;
    ok = test_correlation_meter() && ok;
    ok = test_realtime_allocation_free() && ok;
    ok = test_bass_linked_pumping() && ok;
    return ok ? 0 : 3;
}
