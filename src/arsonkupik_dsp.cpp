#include "arsonkupik_dsp.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace arsonkupik {
namespace {
constexpr double kPi = 3.1415926535897932384626433832795;
constexpr double kMinDb = -120.0;
float sane(float x)
{
    if (!std::isfinite(x)) return 0.0f;
    return std::clamp(x, -8.0f, 8.0f);
}

} // namespace

double db_to_gain(double db) { return std::pow(10.0, db / 20.0); }
double gain_to_db(double gain) { return gain <= 1e-12 ? kMinDb : 20.0 * std::log10(gain); }
double clamp(double value, double lo, double hi) { return std::max(lo, std::min(hi, value)); }
double clamp01(double value) { return clamp(value, 0.0, 1.0); }

void Biquad::reset()
{
    z1_ = 0.0;
    z2_ = 0.0;
}

void Biquad::set_identity()
{
    b0_ = 1.0; b1_ = 0.0; b2_ = 0.0; a1_ = 0.0; a2_ = 0.0;
}

void Biquad::set(EqType type, double sample_rate, double frequency, double q, double gain_db)
{
    if (sample_rate <= 0.0 || frequency <= 0.0) {
        set_identity();
        return;
    }
    frequency = clamp(frequency, 8.0, sample_rate * 0.475);
    q = std::max(0.05, q);

    const double w0 = 2.0 * kPi * frequency / sample_rate;
    const double cw = std::cos(w0);
    const double sw = std::sin(w0);
    const double A = std::pow(10.0, gain_db / 40.0);
    double alpha = sw / (2.0 * q);

    double b0, b1, b2, a0, a1, a2;
    switch (type) {
    case EqType::LowCut:
        b0 = (1.0 + cw) / 2.0;
        b1 = -(1.0 + cw);
        b2 = (1.0 + cw) / 2.0;
        a0 = 1.0 + alpha;
        a1 = -2.0 * cw;
        a2 = 1.0 - alpha;
        break;
    case EqType::HighCut:
        b0 = (1.0 - cw) / 2.0;
        b1 = 1.0 - cw;
        b2 = (1.0 - cw) / 2.0;
        a0 = 1.0 + alpha;
        a1 = -2.0 * cw;
        a2 = 1.0 - alpha;
        break;
    case EqType::Bell:
        b0 = 1.0 + alpha * A;
        b1 = -2.0 * cw;
        b2 = 1.0 - alpha * A;
        a0 = 1.0 + alpha / A;
        a1 = -2.0 * cw;
        a2 = 1.0 - alpha / A;
        break;
    case EqType::Notch:
        b0 = 1.0;
        b1 = -2.0 * cw;
        b2 = 1.0;
        a0 = 1.0 + alpha;
        a1 = -2.0 * cw;
        a2 = 1.0 - alpha;
        break;
    case EqType::LowShelf: {
        const double S = std::max(0.1, q);
        alpha = sw / 2.0 * std::sqrt((A + 1.0 / A) * (1.0 / S - 1.0) + 2.0);
        const double twoSqrtAAlpha = 2.0 * std::sqrt(A) * alpha;
        b0 = A * ((A + 1.0) - (A - 1.0) * cw + twoSqrtAAlpha);
        b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cw);
        b2 = A * ((A + 1.0) - (A - 1.0) * cw - twoSqrtAAlpha);
        a0 = (A + 1.0) + (A - 1.0) * cw + twoSqrtAAlpha;
        a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cw);
        a2 = (A + 1.0) + (A - 1.0) * cw - twoSqrtAAlpha;
        break;
    }
    case EqType::HighShelf: {
        const double S = std::max(0.1, q);
        alpha = sw / 2.0 * std::sqrt((A + 1.0 / A) * (1.0 / S - 1.0) + 2.0);
        const double twoSqrtAAlpha = 2.0 * std::sqrt(A) * alpha;
        b0 = A * ((A + 1.0) + (A - 1.0) * cw + twoSqrtAAlpha);
        b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cw);
        b2 = A * ((A + 1.0) + (A - 1.0) * cw - twoSqrtAAlpha);
        a0 = (A + 1.0) - (A - 1.0) * cw + twoSqrtAAlpha;
        a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cw);
        a2 = (A + 1.0) - (A - 1.0) * cw - twoSqrtAAlpha;
        break;
    }
    case EqType::AllPass:
    default:
        b0 = 1.0 - alpha;
        b1 = -2.0 * cw;
        b2 = 1.0 + alpha;
        a0 = 1.0 + alpha;
        a1 = -2.0 * cw;
        a2 = 1.0 - alpha;
        break;
    }

    b0_ = b0 / a0;
    b1_ = b1 / a0;
    b2_ = b2 / a0;
    a1_ = a1 / a0;
    a2_ = a2 / a0;
}

float Biquad::process(float x)
{
    const double y = b0_ * x + z1_;
    z1_ = b1_ * x - a1_ * y + z2_;
    z2_ = b2_ * x - a2_ * y;
    if (!std::isfinite(z1_) || std::abs(z1_) < 1.0e-30) z1_ = 0.0;
    if (!std::isfinite(z2_) || std::abs(z2_) < 1.0e-30) z2_ = 0.0;
    return sane(static_cast<float>(y));
}

void OnePoleSmoother::prepare(double sample_rate, double seconds, double initial)
{
    seconds = std::max(0.001, seconds);
    alpha_ = 1.0 - std::exp(-1.0 / (sample_rate * seconds));
    state_ = initial;
}

double OnePoleSmoother::process(double target)
{
    state_ += alpha_ * (target - state_);
    return state_;
}

void OnePoleSmoother::reset(double value) { state_ = value; }

void DynamicsProcessor::update_coefficients()
{
    attack_coeff_ = std::exp(-1.0 / (sample_rate_ * std::max(0.001, cfg_.attack_sec)));
    release_coeff_ = std::exp(-1.0 / (sample_rate_ * std::max(0.001, cfg_.release_sec)));
    wet_mix_ = clamp01(cfg_.parallel_mix / 100.0);
    dry_mix_ = 1.0 - wet_mix_;
}

void DynamicsProcessor::prepare(double sample_rate, const CompressorConfig& cfg)
{
    sample_rate_ = sample_rate > 0.0 ? sample_rate : kDefaultSampleRate;
    cfg_ = cfg;
    update_coefficients();
    reset();
}

void DynamicsProcessor::set_config(const CompressorConfig& cfg)
{
    const bool was_enabled = cfg_.enabled;
    cfg_ = cfg;
    update_coefficients();
    if (was_enabled != cfg_.enabled) {
        control_countdown_ = 0;
        if (!cfg_.enabled) last_gr_db_ = 0.0;
    }
}

void DynamicsProcessor::reset()
{
    env_ = 0.0;
    last_gr_db_ = 0.0;
    wet_gain_current_ = db_to_gain(cfg_.makeup_gain_db);
    wet_gain_step_ = 0.0;
    control_countdown_ = 0;
}

std::pair<float,float> DynamicsProcessor::process(float l, float r)
{
    if (!cfg_.enabled) {
        last_gr_db_ = 0.0;
        return {l, r};
    }

    const double peak = std::max(std::abs(static_cast<double>(l)), std::abs(static_cast<double>(r)));
    const double coeff = peak > env_ ? attack_coeff_ : release_coeff_;
    env_ = coeff * env_ + (1.0 - coeff) * peak;

    // The logarithmic transfer curve is evaluated at control rate, then linearly
    // interpolated for eight samples. Envelope tracking remains sample accurate.
    constexpr std::uint32_t kControlInterval = 8;
    if (control_countdown_ == 0) {
        const double in_db = gain_to_db(env_ + 1e-12);
        const double ratio = std::max(1.0, cfg_.ratio);
        const double over = in_db - cfg_.threshold_db;
        double gr_db = 0.0;
        if (cfg_.knee_db > 0.0) {
            const double half_knee = cfg_.knee_db * 0.5;
            if (over > -half_knee && over < half_knee) {
                const double x = over + half_knee;
                const double compressed = x * x / (2.0 * cfg_.knee_db);
                gr_db = compressed * (1.0 / ratio - 1.0);
            } else if (over >= half_knee) {
                gr_db = over * (1.0 / ratio - 1.0);
            }
        } else if (over > 0.0) {
            gr_db = over * (1.0 / ratio - 1.0);
        }

        last_gr_db_ = std::abs(std::min(0.0, gr_db));
        const double wet_gain_target = db_to_gain(gr_db + cfg_.makeup_gain_db);
        wet_gain_step_ = (wet_gain_target - wet_gain_current_) / static_cast<double>(kControlInterval);
        control_countdown_ = kControlInterval;
    }

    wet_gain_current_ += wet_gain_step_;
    --control_countdown_;

    // Linear parallel mixing is unity when dry and wet are identical. Equal-power
    // mixing created an intrinsic ~3 dB lift because both lanes are correlated.
    const double l_out = static_cast<double>(l) * dry_mix_ + static_cast<double>(l) * wet_gain_current_ * wet_mix_;
    const double r_out = static_cast<double>(r) * dry_mix_ + static_cast<double>(r) * wet_gain_current_ * wet_mix_;
    return {sane(static_cast<float>(l_out)), sane(static_cast<float>(r_out))};
}

void LimiterProcessor::update_coefficients()
{
    const double release_sec = cfg_.punch_protect ? 0.105 : 0.060;
    release_coeff_ = std::exp(-1.0 / (sample_rate_ * release_sec));
    drive_gain_ = db_to_gain(clamp(cfg_.limiter_drive, 0.0, 1.4) * 1.05);
    ceiling_gain_ = db_to_gain(std::min(-0.05, cfg_.limiter_ceiling_db));
}

void LimiterProcessor::prepare(double sample_rate, const OutputConfig& cfg)
{
    sample_rate_ = sample_rate > 0.0 ? sample_rate : kDefaultSampleRate;
    cfg_ = cfg;
    update_coefficients();
    reset();
}

void LimiterProcessor::set_config(const OutputConfig& cfg)
{
    const bool was_enabled = cfg_.limiter_enabled;
    cfg_ = cfg;
    update_coefficients();
    if (was_enabled != cfg_.limiter_enabled) {
        gain_ = 1.0;
        last_gr_db_ = 0.0;
        meter_countdown_ = 0;
    }
}

void LimiterProcessor::reset()
{
    gain_ = 1.0;
    last_gr_db_ = 0.0;
    meter_countdown_ = 0;
}

std::pair<float,float> LimiterProcessor::process(float l, float r)
{
    if (!cfg_.limiter_enabled) {
        gain_ = 1.0;
        last_gr_db_ = 0.0;
        return {l, r};
    }

    l = sane(static_cast<float>(static_cast<double>(l) * drive_gain_));
    r = sane(static_cast<float>(static_cast<double>(r) * drive_gain_));

    const double peak = std::max(std::abs(static_cast<double>(l)), std::abs(static_cast<double>(r)));
    const double required_gain = peak > ceiling_gain_ ? ceiling_gain_ / (peak + 1e-12) : 1.0;

    // Instantaneous linked attack prevents sample peaks crossing the configured
    // ceiling. Release is cached and smoothed to avoid chatter and pumping.
    if (required_gain < gain_) {
        gain_ = required_gain;
    } else {
        gain_ = release_coeff_ * gain_ + (1.0 - release_coeff_) * required_gain;
    }

    l = sane(static_cast<float>(static_cast<double>(l) * gain_));
    r = sane(static_cast<float>(static_cast<double>(r) * gain_));

    if (meter_countdown_ == 0) {
        last_gr_db_ = std::max(0.0, -gain_to_db(gain_));
        meter_countdown_ = 16;
    }
    --meter_countdown_;
    return {l, r};
}

ArSonKuPikEngine::ArSonKuPikEngine()
{
    preset_ = &default_preset();
    working_.eq.reserve(kMaxEqStages);
    load_working_from_preset();
    update_cached_gains();
}

void ArSonKuPikEngine::load_working_from_preset()
{
    if (!preset_) preset_ = &default_preset();
    working_.eq.assign(preset_->eq.begin(), preset_->eq.end());
    working_.compressor = preset_->compressor;
    working_.color = preset_->color;
    working_.width = preset_->width;
    working_.output = preset_->output;
    working_.macro = preset_->macro;
}

void ArSonKuPikEngine::prepare(double sample_rate, std::size_t channels)
{
    sample_rate_ = sample_rate > 0.0 ? sample_rate : kDefaultSampleRate;
    channels_ = std::max<std::size_t>(1, std::min<std::size_t>(channels, kMaxChannels));
    bypass_smooth_.prepare(sample_rate_, 0.075, params_.bypass ? 1.0 : 0.0);

    width_slow_alpha_ = 1.0 - std::exp(-static_cast<double>(kWidthControlInterval) / (sample_rate_ * 0.055));
    width_fast_alpha_ = 1.0 - std::exp(-static_cast<double>(kWidthControlInterval) / (sample_rate_ * 0.006));
    width_corr_alpha_ = 1.0 - std::exp(-static_cast<double>(kWidthControlInterval) / (sample_rate_ * 0.090));

    compressor_.prepare(sample_rate_, working_.compressor);
    limiter_.prepare(sample_rate_, working_.output);
    rebuild_from_preset();
    reset();
}

void ArSonKuPikEngine::set_runtime_params(const RuntimeParams& params)
{
    EngineParams realtime;
    realtime.preset_index = static_cast<std::uint32_t>(preset_index_from_id(params.preset_id));
    realtime.enhance = params.enhance;
    realtime.smart_bass = params.smart_bass;
    realtime.smart_treble = params.smart_treble;
    realtime.vocal_body = params.vocal_body;
    realtime.stereo_magic = params.stereo_magic;
    realtime.output_trim_db = params.output_trim_db;
    realtime.smart_protect = params.smart_protect;
    realtime.bypass = params.bypass;
    realtime.advanced_override = params.advanced_override;
    set_realtime_params(realtime);
}

void ArSonKuPikEngine::set_realtime_params(const EngineParams& params)
{
    params_ = params;
    preset_ = &preset_at_index(params_.preset_index);
    load_working_from_preset();
    apply_macros();
    rebuild_from_preset();
    update_cached_gains();
}

void ArSonKuPikEngine::reset()
{
    for (auto& chs : eq_filters_) for (auto& b : chs) b.reset();
    for (auto& b : bass_pre_) b.reset();
    for (auto& b : bass_post_hp_) b.reset();
    for (auto& b : bass_post_lp_) b.reset();
    for (auto& b : warmth_pre_) b.reset();
    for (auto& b : presence_pre_) b.reset();
    for (auto& b : air_pre_) b.reset();
    for (auto& b : deharsh_) b.reset();
    side_air_hp_.reset(); side_air_tone_.reset(); mid_anchor_bp1_.reset(); mid_anchor_bp2_.reset();
    vocal_tickle_bp_.reset(); treble_skin_bp_.reset(); low_body_bp_.reset(); width_pre_hp_.reset();
    width_lowmid_hp_.reset(); width_lowmid_lp_.reset(); width_mid_hp_.reset(); width_mid_lp_.reset();
    width_high_hp_.reset(); width_air_tone_.reset();
    width_side_sub_hp_.reset(); width_side_sub_lp_.reset(); width_side_body_hp_.reset(); width_side_body_lp_.reset();
    width_side_mid_hp_.reset(); width_side_mid_lp_.reset(); width_side_high_hp_.reset(); width_side_high_lp_.reset();
    width_side_air_hp_.reset(); width_side_air_tone_.reset();
    width_control_count_ = 0;
    width_mid_acc_ = 0.0; width_side_acc_ = 0.0;
    width_lr_acc_ = 0.0; width_l2_acc_ = 0.0; width_r2_acc_ = 0.0;
    width_mid_env_ = 1.0e-6; width_side_env_ = 1.0e-6;
    width_side_fast_env_ = 1.0e-6; width_side_slow_env_ = 1.0e-6;
    width_corr_lr_env_ = 1.0e-6; width_corr_l2_env_ = 1.0e-6; width_corr_r2_env_ = 1.0e-6; width_corr_env_ = 1.0;
    meter_input_env_ = 1.0e-12; meter_output_env_ = 1.0e-12; meter_gr_env_ = 0.0;
    meter_corr_lr_env_ = 1.0e-6; meter_corr_l2_env_ = 1.0e-6; meter_corr_r2_env_ = 1.0e-6;
    meters_ = {};
    bypass_smooth_.reset(params_.bypass ? 1.0 : 0.0);
    compressor_.reset(); limiter_.reset();
}

} // namespace arsonkupik
