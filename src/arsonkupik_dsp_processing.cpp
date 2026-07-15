#include "arsonkupik_dsp.hpp"

#include <algorithm>
#include <cmath>

namespace arsonkupik {
namespace {
float sane(float x)
{
    if (!std::isfinite(x)) return 0.0f;
    if (std::abs(x) < 1.0e-30f) return 0.0f;
    return std::clamp(x, -8.0f, 8.0f);
}

double mode_factor(const std::string& mode)
{
    if (mode == "mastering") return 0.96;
    if (mode == "modern") return 0.92;
    if (mode == "warm") return 0.84;
    return 0.58;
}
} // namespace

float ArSonKuPikEngine::soft_saturate(float x, double drive) const
{
    const double d = clamp(drive, 0.1, 2.4);
    const double shaped = std::tanh(static_cast<double>(x) * d) / std::max(0.1, d);
    const double wet = clamp((d - 1.0) * 0.014, 0.0, 0.026);
    const double y = static_cast<double>(x) * (1.0 - wet) + shaped * wet;
    return sane(static_cast<float>(y));
}

float ArSonKuPikEngine::process_channel_eq(std::size_t ch, float x)
{
    for (std::size_t stage = 0; stage < eq_stage_count_; ++stage) {
        x = eq_filters_[stage][ch].process(x);
    }
    return x;
}

std::pair<float,float> ArSonKuPikEngine::process_color(float l, float r)
{
    const auto& c = working_.color;
    if (!c.enabled || c.mix <= 0.0) return {l, r};

    const double mix = clamp01(c.mix / 100.0);
    const double harmonic = clamp01(c.harmonics / 100.0);
    const double bass_amt = clamp01(c.smart_bass / 100.0);
    const double air_amt = clamp(c.air / 48.0, -0.5, 1.0);
    const double god = clamp01(c.god_particles / 100.0);
    const double stereo_mid = clamp01(c.stereo_mid / 100.0);
    const double vocal_presence = clamp01(c.vocal_presence / 100.0);
    const double mid_projection = clamp01(c.mid_projection / 100.0);
    const double vocal_tickle = clamp01(c.vocal_tickle / 100.0);
    const double velvet = clamp01(c.velvet_treble / 100.0);
    const double mode = mode_factor(c.mode);
    const double drive = clamp(c.drive * 0.22 + c.harmonics * 0.002, 0.0, 1.60) * mode;

    const float dry_l = l;
    const float dry_r = r;

    float bass_l = bass_pre_[0].process(l);
    float bass_r = bass_pre_[1].process(r);
    bass_l = soft_saturate(bass_l, 0.92 + drive * 0.10 + bass_amt * 0.35);
    bass_r = soft_saturate(bass_r, 0.92 + drive * 0.10 + bass_amt * 0.35);
    bass_l = bass_post_lp_[0].process(bass_post_hp_[0].process(bass_l));
    bass_r = bass_post_lp_[1].process(bass_post_hp_[1].process(bass_r));
    const double bass_wet = mix * (0.024 + bass_amt * 0.078 + std::max(0.0, c.body) * 0.0022);

    const float warm_l = soft_saturate(warmth_pre_[0].process(l), 0.94 + drive * 0.08 + c.warmth * 0.006);
    const float warm_r = soft_saturate(warmth_pre_[1].process(r), 0.94 + drive * 0.08 + c.warmth * 0.006);
    const double warm_wet = mix * clamp(0.020 + c.warmth * 0.0020, 0.0, 0.078);

    const float pres_l = soft_saturate(presence_pre_[0].process(l), 0.92 + drive * 0.06 + harmonic * 0.10);
    const float pres_r = soft_saturate(presence_pre_[1].process(r), 0.92 + drive * 0.06 + harmonic * 0.10);
    const double pres_wet = mix * (0.020 + harmonic * 0.003 + vocal_presence * 0.038);

    const float air_l = soft_saturate(air_pre_[0].process(l), 0.76 + std::max(0.0, air_amt) * 0.40 + harmonic * 0.06);
    const float air_r = soft_saturate(air_pre_[1].process(r), 0.76 + std::max(0.0, air_amt) * 0.40 + harmonic * 0.06);
    const double air_wet = mix * std::max(0.0, 0.042 + std::max(0.0, air_amt) * 0.135 + harmonic * 0.001 + velvet * 0.014);

    const double mid = 0.5 * (l + r);
    const double side = 0.5 * (l - r);

    float side_air = side_air_tone_.process(side_air_hp_.process(static_cast<float>(side)));
    side_air = soft_saturate(side_air, 0.74 + god * 0.28 + std::max(0.0, air_amt) * 0.26);
    const double side_air_wet = mix * clamp(god * (0.034 + std::max(0.0, air_amt) * 0.066 + harmonic * 0.0008), 0.0, 0.105);

    float mid_anchor = mid_anchor_bp2_.process(mid_anchor_bp1_.process(static_cast<float>(mid)));
    mid_anchor = soft_saturate(mid_anchor, 0.86 + stereo_mid * 0.38 + vocal_presence * 0.28);
    const double mid_wet = mix * clamp(stereo_mid * 0.032 + vocal_presence * 0.042 + mid_projection * 0.044, 0.0, 0.120);

    float low_body = low_body_bp_.process(static_cast<float>(mid));
    low_body = soft_saturate(low_body, 0.82 + bass_amt * 0.30 + std::max(0.0, c.body) * 0.006);
    const double low_body_wet = mix * clamp(bass_amt * 0.022 + std::max(0.0, c.body) * 0.0021 + std::max(0.0, c.warmth) * 0.0012, 0.0, 0.070);

    float tickle = vocal_tickle_bp_.process(static_cast<float>(mid));
    tickle = soft_saturate(tickle, 0.82 + vocal_tickle * 0.34 + harmonic * 0.04);
    const double tickle_wet = mix * clamp(vocal_tickle * 0.036, 0.0, 0.052);

    float skin = treble_skin_bp_.process(static_cast<float>(mid));
    skin = soft_saturate(skin, 0.72 + god * 0.20 + std::max(0.0, air_amt) * 0.18);
    const double skin_wet = mix * clamp((god * 0.020 + std::max(0.0, air_amt) * 0.020 + vocal_tickle * 0.014) * (1.0 - velvet * 0.04), 0.0, 0.055);

    l += static_cast<float>(bass_l * bass_wet + warm_l * warm_wet + pres_l * pres_wet + air_l * air_wet);
    r += static_cast<float>(bass_r * bass_wet + warm_r * warm_wet + pres_r * pres_wet + air_r * air_wet);
    l += static_cast<float>(side_air * side_air_wet + mid_anchor * mid_wet + low_body * low_body_wet + tickle * tickle_wet + skin * skin_wet);
    r += static_cast<float>(-side_air * side_air_wet + mid_anchor * mid_wet + low_body * low_body_wet + tickle * tickle_wet + skin * skin_wet);

    l = deharsh_[0].process(l);
    r = deharsh_[1].process(r);

    const double comp = 1.0 / (1.0 + mix * (drive / 12.2) * 0.012 + harmonic * mix * 0.002
                              + side_air_wet * 0.008 + mid_wet * 0.010 + low_body_wet * 0.006 + air_wet * 0.004);
    l = static_cast<float>(l * comp);
    r = static_cast<float>(r * comp);

    const double dry_keep = clamp(1.0 + mix * 0.080, 1.010, 1.115);
    l = static_cast<float>(dry_l * dry_keep + (l - dry_l));
    r = static_cast<float>(dry_r * dry_keep + (r - dry_r));
    return {sane(l), sane(r)};
}

std::pair<float,float> ArSonKuPikEngine::process_width(float l, float r)
{
    const auto& w = working_.width;
    if (!w.enabled || w.mix <= 0.0) return {l, r};

    const double width_mix = clamp01(w.mix / 100.0);
    const double master_expand = clamp((w.width - 100.0) / 100.0, 0.0, 1.0);
    const double source_protect = clamp01(w.source_protect / 100.0);
    auto additive_gain = [&](double percent, double weight, double linked, double max_value) {
        const double band_expand = clamp((percent - 100.0) / 100.0, 0.0, 1.0);
        return clamp(band_expand * weight + master_expand * linked, 0.0, max_value) * width_mix;
    };

    const double mid = 0.5 * (static_cast<double>(l) + static_cast<double>(r));
    const double side = 0.5 * (static_cast<double>(l) - static_cast<double>(r));
    const double mid_abs = std::abs(mid);
    const double side_abs = std::abs(side);

    width_mid_acc_ += mid_abs;
    width_side_acc_ += side_abs;
    width_lr_acc_ += static_cast<double>(l) * static_cast<double>(r);
    width_l2_acc_ += static_cast<double>(l) * static_cast<double>(l);
    width_r2_acc_ += static_cast<double>(r) * static_cast<double>(r);
    ++width_control_count_;
    if (width_control_count_ >= kWidthControlInterval) {
        const double inv = 1.0 / static_cast<double>(width_control_count_);
        const double mid_avg = width_mid_acc_ * inv;
        const double side_avg = width_side_acc_ * inv;
        const double lr_avg = width_lr_acc_ * inv;
        const double l2_avg = width_l2_acc_ * inv;
        const double r2_avg = width_r2_acc_ * inv;

        width_mid_env_ += width_slow_alpha_ * (mid_avg - width_mid_env_);
        width_side_env_ += width_slow_alpha_ * (side_avg - width_side_env_);
        width_side_fast_env_ += width_fast_alpha_ * (side_avg - width_side_fast_env_);
        width_side_slow_env_ += width_slow_alpha_ * (side_avg - width_side_slow_env_);
        width_corr_lr_env_ += width_corr_alpha_ * (lr_avg - width_corr_lr_env_);
        width_corr_l2_env_ += width_corr_alpha_ * (l2_avg - width_corr_l2_env_);
        width_corr_r2_env_ += width_corr_alpha_ * (r2_avg - width_corr_r2_env_);
        const double corr_denom = std::sqrt(std::max(1.0e-18, width_corr_l2_env_ * width_corr_r2_env_));
        width_corr_env_ = clamp(width_corr_lr_env_ / corr_denom, -1.0, 1.0);

        width_control_count_ = 0;
        width_mid_acc_ = 0.0;
        width_side_acc_ = 0.0;
        width_lr_acc_ = 0.0;
        width_l2_acc_ = 0.0;
        width_r2_acc_ = 0.0;
    }

    const double side_ratio = width_side_env_ / (width_mid_env_ + 1e-6);
    const double center_dominance = width_mid_env_ / (width_side_env_ + 1e-6);
    const double transient_score = clamp((width_side_fast_env_ / (width_side_slow_env_ + 1e-6) - 1.0) * 0.48, 0.0, 1.0);
    const double vocal_prob = clamp((center_dominance - 1.35) / 2.25, 0.0, 1.0) * (1.0 - transient_score * 0.55);
    const double pan_event = clamp((side_ratio - 0.34) / 0.78, 0.0, 1.0) * clamp((width_corr_env_ + 0.08) / 0.58, 0.0, 1.0);
    const double anti_phase = clamp((0.18 - width_corr_env_) / 0.55, 0.0, 1.0);
    const double corr_guard = clamp((width_corr_env_ + 0.12) / 0.52, 0.0, 1.0);
    const double low_risk = anti_phase * source_protect * (1.0 - transient_score * 0.72)
                          * clamp((side_ratio - 0.20) / 0.55, 0.0, 1.0);
    const double low_reduce = clamp(low_risk * 0.68, 0.0, 0.68);

    const float side_f = static_cast<float>(side);
    const float side_sub = width_side_sub_lp_.process(width_side_sub_hp_.process(side_f));
    const float side_body = width_side_body_lp_.process(width_side_body_hp_.process(side_f));
    const float side_mid = width_side_mid_lp_.process(width_side_mid_hp_.process(side_f));
    const float side_high = width_side_high_lp_.process(width_side_high_hp_.process(side_f));
    float side_air = width_side_air_tone_.process(width_side_air_hp_.process(side_f));
    side_air = soft_saturate(side_air, 0.68 + width_mix * 0.12 + std::max(0.0, w.side_tone) * 0.008);

    const double body_gain = additive_gain(w.low_mid_width, 0.052, 0.016, 0.068)
                           * (0.60 + transient_score * 0.70 + pan_event * 0.35) * corr_guard;
    const double mid_gain = additive_gain(w.mid_width, 0.104, 0.040, 0.142)
                          * (1.0 - vocal_prob * 0.52) * corr_guard;
    const double high_gain = additive_gain(w.high_width, 0.215, 0.088, 0.285)
                           * (1.0 - vocal_prob * 0.16) * (0.55 + corr_guard * 0.45);
    const double air_gain = additive_gain(w.high_width, 0.145, 0.064, 0.210)
                          * (1.0 - anti_phase * 0.72) * (0.78 + pan_event * 0.20);

    const float generated = width_pre_hp_.process(static_cast<float>(mid));
    const float lowmid_gen = width_lowmid_lp_.process(width_lowmid_hp_.process(generated));
    const float mid_gen = width_mid_lp_.process(width_mid_hp_.process(generated));
    const float air_gen = width_air_tone_.process(width_high_hp_.process(generated));
    const double gen_lowmid_gain = additive_gain(w.low_mid_width, 0.012, 0.004, 0.018)
                                 * (1.0 - vocal_prob * 0.88) * (1.0 - pan_event * 0.62) * corr_guard;
    const double gen_mid_gain = additive_gain(w.mid_width, 0.022, 0.008, 0.035)
                              * (1.0 - vocal_prob * 0.86) * (1.0 - pan_event * 0.70) * corr_guard;
    const double gen_air_gain = additive_gain(w.high_width, 0.052, 0.020, 0.075)
                              * (1.0 - vocal_prob * 0.42) * (1.0 - anti_phase * 0.66);

    const double base_side_gain = w.width < 100.0 ? clamp(w.width / 100.0, 0.0, 1.0) : 1.0;
    double side_out = side * base_side_gain;
    side_out -= static_cast<double>(side_sub) * low_reduce;
    side_out += static_cast<double>(side_body) * body_gain;
    side_out += static_cast<double>(side_mid) * mid_gain;
    side_out += static_cast<double>(side_high) * high_gain;
    side_out += static_cast<double>(side_air) * air_gain;
    side_out += static_cast<double>(lowmid_gen) * gen_lowmid_gain;
    side_out += static_cast<double>(mid_gen) * gen_mid_gain;
    side_out += static_cast<double>(air_gen) * gen_air_gain;

    const double mid_anchor_gain = 1.0 + width_mix * vocal_prob * 0.010 + width_mix * source_protect * 0.004;
    return {sane(static_cast<float>(mid * mid_anchor_gain + side_out)),
            sane(static_cast<float>(mid * mid_anchor_gain - side_out))};
}

void ArSonKuPikEngine::update_meters_block(double in_peak, double out_peak, double gr_db,
                                           double corr_lr, double corr_l2, double corr_r2,
                                           std::size_t frames)
{
    const double meter_alpha = 1.0 - std::exp(-static_cast<double>(frames) / (sample_rate_ * 0.180));
    const double corr_alpha = 1.0 - std::exp(-static_cast<double>(frames) / (sample_rate_ * 0.120));
    meter_input_env_ += meter_alpha * (in_peak - meter_input_env_);
    meter_output_env_ += meter_alpha * (out_peak - meter_output_env_);
    meter_gr_env_ += meter_alpha * (gr_db - meter_gr_env_);
    meter_corr_lr_env_ += corr_alpha * (corr_lr - meter_corr_lr_env_);
    meter_corr_l2_env_ += corr_alpha * (corr_l2 - meter_corr_l2_env_);
    meter_corr_r2_env_ += corr_alpha * (corr_r2 - meter_corr_r2_env_);

    meters_.input_peak_db = gain_to_db(meter_input_env_ + 1.0e-12);
    meters_.output_peak_db = gain_to_db(meter_output_env_ + 1.0e-12);
    meters_.gain_reduction_db = meter_gr_env_;
    const double denom = std::sqrt(std::max(1.0e-18, meter_corr_l2_env_ * meter_corr_r2_env_));
    meters_.correlation = clamp(meter_corr_lr_env_ / denom, -1.0, 1.0);
    meters_.clipping = out_peak >= 0.985 || gr_db > 8.0;
}

void ArSonKuPikEngine::process(float** planes, std::size_t channels, std::size_t frames)
{
    if (!planes || channels == 0 || frames == 0) return;
    channels = std::min<std::size_t>(channels, channels_);
    if (channels == 0 || !planes[0]) return;

    const double bypass_t = params_.bypass ? 1.0 : 0.0;
    double block_in_peak = 0.0;
    double block_out_peak = 0.0;
    double block_gr = 0.0;
    double block_corr_lr = 0.0;
    double block_corr_l2 = 0.0;
    double block_corr_r2 = 0.0;

    for (std::size_t i = 0; i < frames; ++i) {
        float original[kMaxChannels]{};
        for (std::size_t ch = 0; ch < channels; ++ch) {
            original[ch] = planes[ch] ? sane(planes[ch][i]) : 0.0f;
        }

        float l = original[0];
        float r = channels > 1 ? original[1] : original[0];
        const double in_l = l;
        const double in_r = r;
        block_in_peak = std::max(block_in_peak, std::max(std::abs(in_l), std::abs(in_r)));
        block_corr_lr += in_l * in_r;
        block_corr_l2 += in_l * in_l;
        block_corr_r2 += in_r * in_r;

        l = sane(static_cast<float>(static_cast<double>(l) * cached_input_gain_));
        r = sane(static_cast<float>(static_cast<double>(r) * cached_input_gain_));
        l = process_channel_eq(0, l);
        r = process_channel_eq(1, r);

        const auto comp = compressor_.process(l, r);
        l = comp.first;
        r = comp.second;
        const auto col = process_color(l, r);
        l = col.first;
        r = col.second;
        const auto wid = process_width(l, r);
        l = wid.first;
        r = wid.second;

        l = sane(static_cast<float>(static_cast<double>(l) * cached_output_gain_));
        r = sane(static_cast<float>(static_cast<double>(r) * cached_output_gain_));
        if (working_.output.limiter_enabled) {
            const auto lim = limiter_.process(l, r);
            l = lim.first;
            r = lim.second;
        }

        const double bypass = bypass_smooth_.process(bypass_t);
        l = sane(static_cast<float>(static_cast<double>(l) * (1.0 - bypass) + static_cast<double>(original[0]) * bypass));
        r = sane(static_cast<float>(static_cast<double>(r) * (1.0 - bypass)
                                  + static_cast<double>(channels > 1 ? original[1] : original[0]) * bypass));
        planes[0][i] = l;
        if (channels > 1 && planes[1]) planes[1][i] = r;
        for (std::size_t ch = 2; ch < channels; ++ch) {
            if (planes[ch]) planes[ch][i] = original[ch];
        }

        block_out_peak = std::max(block_out_peak,
                                  std::max(std::abs(static_cast<double>(l)), std::abs(static_cast<double>(r))));
        block_gr = std::max(block_gr, std::max(compressor_.gain_reduction_db(), limiter_.gain_reduction_db()));
    }

    const double inv_frames = 1.0 / static_cast<double>(frames);
    update_meters_block(block_in_peak, block_out_peak, block_gr,
                        block_corr_lr * inv_frames,
                        block_corr_l2 * inv_frames,
                        block_corr_r2 * inv_frames,
                        frames);
}

} // namespace arsonkupik
