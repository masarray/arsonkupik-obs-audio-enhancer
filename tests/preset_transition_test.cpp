#include "arsonkupik_transition.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <new>

namespace { std::atomic<std::size_t> g_allocations{0}; }

void* operator new(std::size_t size)
{
    g_allocations.fetch_add(1, std::memory_order_relaxed);
    if (void* p = std::malloc(size)) return p;
    throw std::bad_alloc();
}
void* operator new[](std::size_t size)
{
    g_allocations.fetch_add(1, std::memory_order_relaxed);
    if (void* p = std::malloc(size)) return p;
    throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

namespace {
constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr std::size_t kMaxBlock = 1024;

struct JumpTracker {
    double maximum = 0.0;
    double peak = 0.0;
    float previous_l = 0.0f;
    float previous_r = 0.0f;
    bool has_previous = false;
    bool finite = true;

    void add(float l, float r)
    {
        if (!std::isfinite(l) || !std::isfinite(r)) finite = false;
        peak = std::max(peak, std::max(std::abs(static_cast<double>(l)),
                                      std::abs(static_cast<double>(r))));
        if (has_previous) {
            maximum = std::max(maximum,
                std::abs(static_cast<double>(l) - previous_l));
            maximum = std::max(maximum,
                std::abs(static_cast<double>(r) - previous_r));
        }
        previous_l = l;
        previous_r = r;
        has_previous = true;
    }
};

std::pair<float, float> source_sample(std::size_t index, double sample_rate)
{
    const double t = static_cast<double>(index) / sample_rate;
    const double slow = 0.16 * std::sin(2.0 * kPi * 67.0 * t);
    const double center = 0.095 * std::sin(2.0 * kPi * 997.0 * t);
    const double detail_l = 0.030 * std::sin(2.0 * kPi * 6100.0 * t);
    const double detail_r = 0.027 * std::sin(2.0 * kPi * 6900.0 * t + 0.21);
    return {
        static_cast<float>(slow + center + detail_l),
        static_cast<float>(0.92 * slow + 0.91 * center + detail_r)
    };
}

arsonkupik::EngineParams params_for_preset(std::size_t index, bool bypass)
{
    using namespace arsonkupik;
    const auto& preset = preset_at_index(index);
    EngineParams p;
    p.preset_index = static_cast<std::uint32_t>(index);
    p.enhance = preset.macro.enhance;
    p.smart_bass = preset.macro.smart_bass;
    p.smart_treble = preset.macro.smart_treble;
    p.vocal_body = preset.macro.vocal_body;
    p.stereo_magic = preset.macro.stereo_magic;
    p.output_trim_db = preset.output.output_gain_db;
    p.smart_protect = preset.macro.smart_protect;
    p.bypass = bypass;
    p.advanced_override = false;
    return p;
}

double static_reference_jump(double sample_rate)
{
    using namespace arsonkupik;
    constexpr std::size_t block = 256;
    const std::size_t total_frames = static_cast<std::size_t>(sample_rate * 0.22);
    const std::size_t warmup_frames = static_cast<std::size_t>(sample_rate * 0.05);
    double maximum = 0.0;

    std::array<float, block> left{};
    std::array<float, block> right{};

    for (std::size_t preset_index = 0; preset_index < factory_presets().size(); ++preset_index) {
        ArSonKuPikEngine engine;
        const EngineParams params = params_for_preset(preset_index, false);
        engine.set_realtime_params(params);
        engine.prepare(sample_rate, 2);

        JumpTracker tracker;
        for (std::size_t offset = 0; offset < total_frames; offset += block) {
            const std::size_t count = std::min(block, total_frames - offset);
            for (std::size_t i = 0; i < count; ++i) {
                const auto sample = source_sample(offset + i, sample_rate);
                left[i] = sample.first;
                right[i] = sample.second;
            }
            float* planes[2] = {left.data(), right.data()};
            engine.process(planes, 2, count);
            for (std::size_t i = 0; i < count; ++i) {
                if (offset + i >= warmup_frames) tracker.add(left[i], right[i]);
            }
        }
        maximum = std::max(maximum, tracker.maximum);
    }
    return maximum;
}

bool run_case(double sample_rate, std::size_t block_size, double interval_ms,
              double reference_jump)
{
    using namespace arsonkupik;
    const std::size_t total_frames = static_cast<std::size_t>(sample_rate * 0.42);
    const std::size_t warmup_frames = static_cast<std::size_t>(sample_rate * 0.05);
    const std::size_t interval_frames = std::max<std::size_t>(1,
        static_cast<std::size_t>(sample_rate * interval_ms / 1000.0));

    std::array<float, kMaxBlock> left{};
    std::array<float, kMaxBlock> right{};
    ArSonKuPikTransitionProcessor processor;
    processor.prepare(sample_rate, 2, params_for_preset(0, false));

    JumpTracker tracker;
    g_allocations.store(0, std::memory_order_relaxed);

    for (std::size_t offset = 0; offset < total_frames; offset += block_size) {
        const std::size_t count = std::min(block_size, total_frames - offset);
        const std::size_t event = offset / interval_frames;
        const std::size_t preset_index = event % factory_presets().size();
        const bool bypass = (event % 7U) == 3U || (event % 11U) == 8U;
        const EngineParams requested = params_for_preset(preset_index, bypass);

        for (std::size_t i = 0; i < count; ++i) {
            const auto sample = source_sample(offset + i, sample_rate);
            left[i] = sample.first;
            right[i] = sample.second;
        }

        float* planes[2] = {left.data(), right.data()};
        processor.process(planes, 2, count, requested);
        for (std::size_t i = 0; i < count; ++i) {
            if (offset + i >= warmup_frames) tracker.add(left[i], right[i]);
        }
    }

    const std::size_t allocations = g_allocations.load(std::memory_order_relaxed);
    const double jump_ratio = tracker.maximum / std::max(1.0e-12, reference_jump);
    const bool ok = tracker.finite
                 && tracker.peak <= 1.001
                 && jump_ratio <= 1.10
                 && allocations == 0;

    std::cout << "transition sr=" << sample_rate
              << " block=" << block_size
              << " interval_ms=" << interval_ms
              << " jump=" << tracker.maximum
              << " reference_jump=" << reference_jump
              << " ratio=" << jump_ratio
              << " peak=" << tracker.peak
              << " allocations=" << allocations
              << " result=" << (ok ? "pass" : "fail") << "\n";
    return ok;
}
} // namespace

int main()
{
    using namespace arsonkupik;
    (void)factory_presets(); // Initialize all static preset storage before counting.

    bool ok = true;
    constexpr std::array<double, 3> sample_rates = {44100.0, 48000.0, 96000.0};
    constexpr std::array<std::size_t, 5> block_sizes = {64, 128, 256, 480, 1024};
    constexpr std::array<double, 3> intervals_ms = {2.0, 10.0, 20.0};

    for (std::size_t sr_index = 0; sr_index < sample_rates.size(); ++sr_index) {
        const double sr = sample_rates[sr_index];
        const double reference = static_reference_jump(sr);
        for (std::size_t block_index = 0; block_index < block_sizes.size(); ++block_index) {
            const double interval = intervals_ms[(sr_index + block_index) % intervals_ms.size()];
            ok = run_case(sr, block_sizes[block_index], interval, reference) && ok;
        }
    }

    const double reference_48k = static_reference_jump(48000.0);
    for (double interval : intervals_ms) {
        ok = run_case(48000.0, 128, interval, reference_48k) && ok;
    }

    return ok ? 0 : 4;
}
