#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace arsonkupik {

constexpr double kDefaultSampleRate = 48000.0;
constexpr int kMaxChannels = 8;
constexpr std::size_t kMaxEqStages = 96;

double db_to_gain(double db);
double gain_to_db(double gain);
double clamp(double value, double lo, double hi);
double clamp01(double value);

enum class EqType {
    LowCut,
    LowShelf,
    Bell,
    Notch,
    HighShelf,
    HighCut,
    AllPass
};

struct EqBand {
    // Factory EQ identifiers are static string literals used for documentation
    // and debugging only. A pointer avoids std::string allocation when presets
    // are copied on the realtime audio thread.
    const char* id = "";
    EqType type = EqType::Bell;
    double frequency = 1000.0;
    double gain_db = 0.0;
    double q = 0.70710678;
    int slope_db_per_oct = 12;
    bool enabled = true;
};

struct CompressorConfig {
    bool enabled = true;
    double threshold_db = -24.4;
    double ratio = 1.7;
    double knee_db = 24.0;
    double attack_sec = 0.032;
    double release_sec = 0.21;
    double makeup_gain_db = 0.88;
    double parallel_mix = 92.0; // percent
};

struct ColorConfig {
    bool enabled = true;
    double drive = 3.12;
    double body_freq = 166.0;
    double body = 12.8;
    double smart_bass = 55.0;
    double warmth_freq = 500.0;
    double warmth = 12.8;
    double harmonics_freq = 2180.0;
    double harmonics = 34.0;
    double air_freq = 12650.0;
    double air = 48.8;
    double ai_high_repair = 46.0;
    double velvet_treble = 66.0;
    double god_particles = 92.0;
    double vocal_tickle = 67.0;
    double vocal_presence = 55.0;
    double mid_projection = 65.0;
    double mix = 30.5;
    double stereo_mid = 70.0;
    std::string mode = "mastering";
};

struct WidthConfig {
    bool enabled = true;
    double mix = 73.0;
    double width = 153.0;
    double low_width = 100.0;
    double low_mid_width = 101.0;
    double mid_width = 128.0;
    double high_width = 200.0;
    double source_protect = 56.0;
    bool mono_bass = true;
    double mono_bass_freq = 150.0;
    double side_tone = 4.28;
};

struct OutputConfig {
    double input_gain_db = 0.0;
    double output_gain_db = -0.55;
    bool limiter_enabled = true;
    double limiter_ceiling_db = -1.05;
    double limiter_drive = 0.76;
    bool punch_protect = true;
    bool bypass = false;
};

struct MacroConfig {
    double masari_feel = 100.0;
    double enhance = 65.0;
    double smart_bass = 55.0;
    double smart_treble = 70.0;
    double vocal_body = 65.0;
    double stereo_magic = 85.0;
    bool smart_protect = true;
};

struct Preset {
    std::string id;
    std::string name;
    std::string description;
    std::vector<EqBand> eq;
    CompressorConfig compressor;
    ColorConfig color;
    WidthConfig width;
    OutputConfig output;
    MacroConfig macro;
};

const std::vector<Preset>& factory_presets();
const Preset* find_preset(const std::string& id);
const Preset& default_preset();

inline std::size_t preset_index_from_id(const std::string& id)
{
    const auto& presets = factory_presets();
    for (std::size_t i = 0; i < presets.size(); ++i) {
        if (presets[i].id == id) return i;
    }
    return 0U;
}

inline const Preset& preset_at_index(std::size_t index)
{
    const auto& presets = factory_presets();
    return presets[index < presets.size() ? index : 0U];
}

// Public convenience structure used by tests and non-realtime callers.
struct RuntimeParams {
    std::string preset_id = "default";
    double enhance = 65.0;
    double smart_bass = 55.0;
    double smart_treble = 70.0;
    double vocal_body = 65.0;
    double stereo_magic = 85.0;
    double output_trim_db = -0.55;
    bool smart_protect = true;
    bool bypass = false;
    bool advanced_override = false;
};

// Allocation-free POD parameter block for the OBS audio thread.
struct EngineParams {
    std::uint32_t preset_index = 0;
    double enhance = 65.0;
    double smart_bass = 55.0;
    double smart_treble = 70.0;
    double vocal_body = 65.0;
    double stereo_magic = 85.0;
    double output_trim_db = -0.55;
    bool smart_protect = true;
    bool bypass = false;
    bool advanced_override = false;
};

class Biquad {
public:
    Biquad() = default;
    void reset();
    void set_identity();
    void set(EqType type, double sample_rate, double frequency, double q, double gain_db = 0.0);
    float process(float x);
private:
    double b0_ = 1.0, b1_ = 0.0, b2_ = 0.0, a1_ = 0.0, a2_ = 0.0;
    double z1_ = 0.0, z2_ = 0.0;
};

class OnePoleSmoother {
public:
    void prepare(double sample_rate, double seconds, double initial = 0.0);
    double process(double target);
    void reset(double value = 0.0);
    double value() const { return state_; }
private:
    double alpha_ = 0.001;
    double state_ = 0.0;
};

struct MeterState {
    double input_peak_db = -120.0;
    double output_peak_db = -120.0;
    double gain_reduction_db = 0.0;
    double correlation = 1.0;
    bool clipping = false;
};

class DynamicsProcessor {
public:
    void prepare(double sample_rate, const CompressorConfig& cfg);
    void set_config(const CompressorConfig& cfg);
    void reset();
    std::pair<float,float> process(float l, float r);
    double gain_reduction_db() const { return last_gr_db_; }
private:
    void update_coefficients();

    double sample_rate_ = kDefaultSampleRate;
    CompressorConfig cfg_{};
    double env_ = 0.0;
    double last_gr_db_ = 0.0;
    double attack_coeff_ = 0.0;
    double release_coeff_ = 0.0;
    double dry_mix_ = 0.0;
    double wet_mix_ = 1.0;
    double wet_gain_current_ = 1.0;
    double wet_gain_step_ = 0.0;
    std::uint32_t control_countdown_ = 0;
};

class LimiterProcessor {
public:
    void prepare(double sample_rate, const OutputConfig& cfg);
    void set_config(const OutputConfig& cfg);
    void reset();
    std::pair<float,float> process(float l, float r);
    double gain_reduction_db() const { return last_gr_db_; }
private:
    void update_coefficients();

    double sample_rate_ = kDefaultSampleRate;
    OutputConfig cfg_{};
    double gain_ = 1.0;
    double release_coeff_ = 0.0;
    double drive_gain_ = 1.0;
    double ceiling_gain_ = 1.0;
    double last_gr_db_ = 0.0;
    std::uint32_t meter_countdown_ = 0;
};

class ArSonKuPikEngine {
public:
    ArSonKuPikEngine();
    void prepare(double sample_rate, std::size_t channels);
    void set_runtime_params(const RuntimeParams& params);
    void set_realtime_params(const EngineParams& params);
    void reset();
    void process(float** planes, std::size_t channels, std::size_t frames);
    const MeterState& meters() const { return meters_; }
    const Preset& current_preset() const { return *preset_; }

private:
    void load_working_from_preset();
    void rebuild_from_preset();
    void rebuild_eq();
    void rebuild_color_filters();
    void rebuild_width_filters();
    void apply_macros();
    void update_cached_gains();
    std::pair<float,float> process_color(float l, float r);
    std::pair<float,float> process_width(float l, float r);
    float soft_saturate(float x, double drive) const;
    float process_channel_eq(std::size_t ch, float x);
    void update_meters_block(double in_peak, double out_peak, double gr_db,
                             double corr_lr, double corr_l2, double corr_r2,
                             std::size_t frames);

    double sample_rate_ = kDefaultSampleRate;
    std::size_t channels_ = 2;
    EngineParams params_{};
    const Preset* preset_ = nullptr;
    Preset working_;
    MeterState meters_{};

    std::array<std::array<Biquad, kMaxChannels>, kMaxEqStages> eq_filters_{};
    std::size_t eq_stage_count_ = 0;

    // Color / psychoacoustic filters. Stereo paths use indexes 0=L, 1=R.
    std::array<Biquad,2> bass_pre_;
    std::array<Biquad,2> bass_post_hp_;
    std::array<Biquad,2> bass_post_lp_;
    std::array<Biquad,2> warmth_pre_;
    std::array<Biquad,2> presence_pre_;
    std::array<Biquad,2> air_pre_;
    std::array<Biquad,2> deharsh_;
    Biquad side_air_hp_;
    Biquad side_air_tone_;
    Biquad mid_anchor_bp1_;
    Biquad mid_anchor_bp2_;
    Biquad vocal_tickle_bp_;
    Biquad treble_skin_bp_;
    Biquad low_body_bp_;

    // Generated phase-safe width from mid copy.
    Biquad width_pre_hp_;
    Biquad width_lowmid_hp_;
    Biquad width_lowmid_lp_;
    Biquad width_mid_hp_;
    Biquad width_mid_lp_;
    Biquad width_high_hp_;
    Biquad width_air_tone_;

    // Smart Multiband M/S Imager v2: real-side analysis/enhancement.
    Biquad width_side_sub_hp_;
    Biquad width_side_sub_lp_;
    Biquad width_side_body_hp_;
    Biquad width_side_body_lp_;
    Biquad width_side_mid_hp_;
    Biquad width_side_mid_lp_;
    Biquad width_side_high_hp_;
    Biquad width_side_high_lp_;
    Biquad width_side_air_hp_;
    Biquad width_side_air_tone_;

    DynamicsProcessor compressor_;
    LimiterProcessor limiter_;
    OnePoleSmoother bypass_smooth_;

    // Control-rate stereo analysis with true energy correlation.
    static constexpr std::uint32_t kWidthControlInterval = 16;
    std::uint32_t width_control_count_ = 0;
    double width_mid_acc_ = 0.0;
    double width_side_acc_ = 0.0;
    double width_lr_acc_ = 0.0;
    double width_l2_acc_ = 0.0;
    double width_r2_acc_ = 0.0;
    double width_mid_env_ = 1.0e-6;
    double width_side_env_ = 1.0e-6;
    double width_side_fast_env_ = 1.0e-6;
    double width_side_slow_env_ = 1.0e-6;
    double width_corr_lr_env_ = 1.0e-6;
    double width_corr_l2_env_ = 1.0e-6;
    double width_corr_r2_env_ = 1.0e-6;
    double width_corr_env_ = 1.0;
    double width_slow_alpha_ = 0.0;
    double width_fast_alpha_ = 0.0;
    double width_corr_alpha_ = 0.0;

    // Cached linear gains; recalculated only when parameters change.
    double cached_input_gain_ = 1.0;
    double cached_output_gain_ = 1.0;

    // Block-rate metering state.
    double meter_input_env_ = 1.0e-12;
    double meter_output_env_ = 1.0e-12;
    double meter_gr_env_ = 0.0;
    double meter_corr_lr_env_ = 1.0e-6;
    double meter_corr_l2_env_ = 1.0e-6;
    double meter_corr_r2_env_ = 1.0e-6;
};

} // namespace arsonkupik
