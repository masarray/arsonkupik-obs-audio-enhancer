#include "arsonkupik_dsp.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace arsonkupik {
namespace {
constexpr double kInternalChainHeadroomDb = -8.0;
constexpr double kInternalRestoreDb = 4.9;


struct QTableView { const double* values = nullptr; std::size_t count = 0; };
QTableView butterworth_q_values(int slope)
{
    static constexpr double q48[] = {0.50979558, 0.60134489, 0.89997622, 2.56291545};
    static constexpr double q36[] = {0.51763809, 0.70710678, 1.93185165};
    static constexpr double q24[] = {0.54119610, 1.30656296};
    static constexpr double q12[] = {0.70710678};
    switch (slope) {
    case 48: return {q48, 4};
    case 36: return {q36, 3};
    case 24: return {q24, 2};
    default: return {q12, 1};
    }
}
} // namespace

void ArSonKuPikEngine::apply_macros()
{
    const double ui_enhance = params_.advanced_override ? params_.enhance : working_.macro.enhance;
    const double ui_bass    = params_.advanced_override ? params_.smart_bass : working_.macro.smart_bass;
    const double ui_treble  = params_.advanced_override ? params_.smart_treble : working_.macro.smart_treble;
    const double ui_vocal   = params_.advanced_override ? params_.vocal_body : working_.macro.vocal_body;
    const double ui_stereo  = params_.advanced_override ? params_.stereo_magic : working_.macro.stereo_magic;

    auto exp01 = [](double x, double curve) { return std::pow(clamp01(x), curve); };
    auto bipolar = [&](double percent, double curve) {
        const double x = clamp01(percent / 100.0);
        const double signedLinear = (x - 0.5) * 2.0;
        return signedLinear >= 0.0 ? exp01(signedLinear, curve) : -exp01(-signedLinear, curve);
    };
    auto endpointLift = [](double amount) {
        const double x = clamp01((amount - 0.55) / 0.45);
        return x * x * (3.0 - 2.0 * x);
    };
    auto topologyActivation = [](double amount) {
        const double x = clamp01(amount / 0.025);
        return x * x * (3.0 - 2.0 * x);
    };
    auto addBand = [&](EqType type, double freq, double gain, double q, int slope = 12, bool enabled = true) {
        EqBand b;
        b.type = type;
        b.frequency = freq;
        b.gain_db = gain;
        b.q = q;
        b.slope_db_per_oct = slope;
        b.enabled = enabled;
        working_.eq.push_back(b);
    };

    // Slightly lower curves make moderate factory-preset values more audible
    // while preserving the same neutral point and exact 0/100 endpoints.
    const double enhance = exp01(ui_enhance / 100.0, 0.92);
    const double bassBip = bipolar(ui_bass, 1.18);
    const double trebleBip = bipolar(ui_treble, 1.18);
    const double vocalBip = bipolar(ui_vocal, 1.08);
    const double stereoBip = bipolar(ui_stereo, 1.10);
    const double bassBoost = std::max(0.0, bassBip);
    const double bassClean = std::max(0.0, -bassBip);
    const double trebleBoost = std::max(0.0, trebleBip);
    const double trebleClean = std::max(0.0, -trebleBip);
    const double vocal = std::max(0.0, vocalBip);
    const double vocalTuck = std::max(0.0, -vocalBip);
    const double stereoWide = std::max(0.0, stereoBip);
    const double stereoNarrow = std::max(0.0, -stereoBip);
    const double bassEndpoint = endpointLift(bassBoost);
    const double trebleEndpoint = endpointLift(trebleBoost);
    const double masari = clamp01(working_.macro.masari_feel / 100.0);

    const double bassBoostActive = topologyActivation(bassBoost);
    const double bassCleanActive = topologyActivation(bassClean);
    const double trebleBoostActive = topologyActivation(trebleBoost);
    const double trebleCleanActive = topologyActivation(trebleClean);
    const double vocalActive = topologyActivation(vocal);
    const double vocalTuckActive = topologyActivation(vocalTuck);

    const double colorActivation = clamp01(enhance * 0.70 + vocal * 0.58 + vocalTuck * 0.28
                                         + bassBoost * 0.34 + trebleBoost * 0.36
                                         + bassClean * 0.12 + trebleClean * 0.22);
    working_.color.enabled = true; // keep filter state warm; mix reaches zero at neutral
    working_.color.mix = clamp(20.0 * vocal + 31.0 * enhance + 8.0 * vocalTuck
                             + 9.0 * bassBoost + 13.5 * trebleBoost + masari,
                             0.0, 54.0);

    working_.color.smart_bass = clamp(100.0 * bassBoost, 0.0, 100.0);
    working_.color.body = clamp(21.0 * bassBoost + 10.0 * bassEndpoint + 7.0 * vocal + 3.0 * enhance, 0.0, 46.0);
    working_.color.warmth = clamp(13.0 * bassBoost + 6.2 * bassEndpoint + 5.3 * vocal + 2.0 * enhance, 0.0, 36.0);
    working_.color.drive = clamp(0.10 + 1.18 * enhance + 1.10 * bassBoost + 0.25 * bassEndpoint + 0.42 * vocal, 0.0, 3.45);

    addBand(EqType::LowShelf, 58.0 + 12.0 * bassBoost,
            (0.35 + 4.95 * bassBoost + 1.90 * bassEndpoint) * bassBoostActive, 0.64);
    addBand(EqType::Bell, 104.0 + 20.0 * bassBoost,
            (0.18 + 3.10 * bassBoost + 1.15 * bassEndpoint) * bassBoostActive, 0.82);
    addBand(EqType::Bell, 235.0 + 70.0 * bassBoost,
            (0.10 + 1.55 * bassBoost + 0.45 * bassEndpoint) * bassBoostActive, 0.78);

    const double lowCutTargetHz = 8.0 + 78.0 * bassClean;
    const double lowCutSecondBlend = topologyActivation(clamp01((bassClean - 0.50) / 0.18));
    const double lowCutSecondHz = 8.0 + (lowCutTargetHz - 8.0) * lowCutSecondBlend;
    addBand(EqType::LowCut, lowCutTargetHz, 0.0, 0.70710678, 12, true);
    addBand(EqType::LowCut, lowCutSecondHz, 0.0, 1.30656296, 12, true);
    addBand(EqType::Bell, 185.0 + 95.0 * bassClean,
            (-0.45 - 4.95 * bassClean) * bassCleanActive, 0.82);
    addBand(EqType::Bell, 380.0 + 160.0 * bassClean,
            (-0.12 - 2.45 * bassClean) * bassCleanActive, 0.95);

    working_.color.air = clamp(112.0 * trebleBoost + 28.0 * trebleEndpoint - 13.0 * trebleClean, -14.0, 140.0);
    working_.color.god_particles = clamp(100.0 * trebleBoost + 12.0 * enhance, 0.0, 100.0);
    working_.color.vocal_tickle = clamp(32.0 * trebleBoost + 14.0 * trebleEndpoint + 56.0 * vocal + 8.0 * enhance, 0.0, 114.0);
    working_.color.velvet_treble = clamp(18.0 + 82.0 * trebleClean + 18.0 * trebleBoost, 0.0, 100.0);
    working_.color.ai_high_repair = clamp(18.0 + 78.0 * trebleClean, 0.0, 100.0);
    addBand(EqType::HighShelf, 8400.0 + 2600.0 * trebleBoost,
            (0.48 + 7.10 * trebleBoost + 2.10 * trebleEndpoint) * trebleBoostActive, 0.56);
    addBand(EqType::Bell, 3100.0 + 600.0 * trebleBoost,
            (0.16 + 2.65 * trebleBoost + 1.10 * trebleEndpoint) * trebleBoostActive, 0.90);
    addBand(EqType::Bell, 5600.0 + 1700.0 * trebleClean,
            (-0.75 - 5.60 * trebleClean) * trebleCleanActive, 1.45);
    addBand(EqType::Bell, 9200.0,
            (-0.30 - 2.85 * trebleClean) * trebleCleanActive, 0.78);

    working_.color.vocal_presence = clamp(100.0 * vocal + 8.0 * enhance, 0.0, 100.0);
    working_.color.mid_projection = clamp(100.0 * vocal + 14.0 * enhance, 0.0, 100.0);
    working_.color.stereo_mid = clamp(50.0 + 45.0 * vocal - 24.0 * vocalTuck, 0.0, 100.0);
    addBand(EqType::Bell, 1450.0 + 380.0 * vocal, (0.40 + 4.80 * vocal) * vocalActive, 0.80);
    addBand(EqType::Bell, 310.0, (-0.04 - 0.46 * vocal) * vocalActive, 1.06);
    addBand(EqType::Bell, 520.0, (0.16 + 1.85 * vocal) * vocalActive, 0.74);
    addBand(EqType::Bell, 3350.0, (0.20 + 3.25 * vocal) * vocalActive, 1.00);
    addBand(EqType::Bell, 5200.0, (0.08 + 1.45 * vocal) * vocalActive, 1.18);
    addBand(EqType::Bell, 1650.0, (-0.40 - 3.80 * vocalTuck) * vocalTuckActive, 0.90);
    addBand(EqType::Bell, 3300.0, (-0.24 - 2.35 * vocalTuck) * vocalTuckActive, 1.12);
    addBand(EqType::Bell, 520.0, (-0.08 - 0.95 * vocalTuck) * vocalTuckActive, 0.82);

    working_.color.harmonics = clamp(72.0 * enhance + 14.0 * trebleBoost, 0.0, 100.0);
    working_.color.mix = clamp(working_.color.mix + enhance * (4.0 + masari * 2.0), 0.0, 54.0);
    working_.compressor.enabled = true; // zero parallel mix is the neutral state
    working_.compressor.threshold_db = clamp(-15.5 - 4.0 * enhance - 2.2 * vocal, -24.0, -11.5);
    working_.compressor.ratio = clamp(1.04 + enhance * 0.34 + vocal * 0.40, 1.0, 1.52);
    working_.compressor.attack_sec = clamp(0.046 - 0.015 * vocal + 0.006 * (1.0 - enhance), 0.018, 0.064);
    working_.compressor.release_sec = clamp(0.145 + 0.040 * enhance + 0.035 * vocal, 0.120, 0.260);
    working_.compressor.parallel_mix = clamp(40.0 * enhance + 24.0 * vocal, 0.0, 42.0);
    working_.compressor.makeup_gain_db = clamp(0.10 + 0.85 * enhance + 0.95 * vocal, 0.0, 1.35);

    working_.width.enabled = true; // keep M/S analysis and filter history warm
    if (stereoNarrow > 0.006) {
        const double monoScale = 100.0 * (1.0 - stereoNarrow);
        working_.width.mix = 100.0;
        working_.width.width = monoScale;
        working_.width.low_mid_width = monoScale;
        working_.width.mid_width = monoScale;
        working_.width.high_width = monoScale;
        working_.width.source_protect = 100.0;
        working_.width.side_tone = 0.0;
    } else if (stereoWide > 0.006) {
        working_.width.mix = clamp(30.0 + 68.0 * stereoWide, 0.0, 100.0);
        working_.width.width = clamp(100.0 + 100.0 * stereoWide, 100.0, 200.0);
        working_.width.low_mid_width = clamp(100.0 + 22.0 * stereoWide, 100.0, 135.0);
        working_.width.mid_width = clamp(100.0 + 50.0 * stereoWide, 100.0, 168.0);
        working_.width.high_width = clamp(100.0 + 116.0 * stereoWide, 100.0, 224.0);
        working_.width.source_protect = clamp(78.0 - 12.0 * stereoWide, 58.0, 96.0);
        working_.width.side_tone = clamp(0.6 + 3.8 * stereoWide, 0.0, 5.0);
    } else {
        working_.width.mix = 0.0;
        working_.width.width = 100.0;
        working_.width.low_mid_width = 100.0;
        working_.width.mid_width = 100.0;
        working_.width.high_width = 100.0;
        working_.width.source_protect = 100.0;
        working_.width.side_tone = 0.0;
    }

    const double perceived_benefit_db = enhance * 0.72 + vocal * 0.38
                                      + trebleBoost * 0.10 + bassBoost * 0.06
                                      - vocalTuck * 0.08;
    working_.output.output_gain_db = params_.output_trim_db
                                   + perceived_benefit_db
                                   + preset_->calibrated_wow_trim_db
                                   - 1.00;
    working_.output.bypass = params_.bypass;
    working_.output.punch_protect = params_.smart_protect;

    const double headroom = bassBoost * 3.05 + trebleBoost * 0.30 + enhance * 0.16 + vocal * 0.10;
    if (params_.smart_protect) {
        working_.output.output_gain_db -= std::max(0.0, headroom - 0.22);
        working_.output.limiter_enabled = true;
        working_.output.limiter_ceiling_db = std::min(working_.output.limiter_ceiling_db, -1.15 - bassBoost * 0.75);
        working_.output.limiter_drive = clamp(working_.output.limiter_drive + bassBoost * 0.040 + enhance * 0.026, 0.0, 0.92);
    }
}

void ArSonKuPikEngine::rebuild_all_processors()
{
    rebuild_eq();
    rebuild_color_filters();
    rebuild_width_filters();
    compressor_.set_config(working_.compressor);
    ++debug_counters_.compressor_updates;
    limiter_.set_config(working_.output);
    ++debug_counters_.limiter_updates;
    update_cached_gains();
}

void ArSonKuPikEngine::rebuild_eq()
{
    ++debug_counters_.eq_rebuilds;
    struct EqStageSpec {
        EqType type = EqType::Bell;
        double frequency = 1000.0;
        double q = 0.70710678;
        double gain_db = 0.0;
    };
    std::array<EqStageSpec, kMaxEqStages> stages{};
    std::size_t stage_count = 0;
    auto add_stage = [&](EqType type, double frequency, double q, double gain_db) {
        if (stage_count < kMaxEqStages) stages[stage_count++] = EqStageSpec{type, frequency, q, gain_db};
    };
    for (const auto& band : working_.eq) {
        if (!band.enabled) continue;
        if (band.type == EqType::LowCut || band.type == EqType::HighCut) {
            const auto qs = butterworth_q_values(band.slope_db_per_oct);
            for (std::size_t i = 0; i < qs.count; ++i) add_stage(band.type, band.frequency, qs.values[i], band.gain_db);
        } else {
            add_stage(band.type, band.frequency, band.q, band.gain_db);
        }
    }
    if (eq_stage_count_ != stage_count) {
        for (auto& chs : eq_filters_) for (auto& b : chs) b.reset();
        eq_stage_count_ = stage_count;
    }
    for (std::size_t stage = 0; stage < eq_stage_count_; ++stage) {
        const auto& spec = stages[stage];
        for (auto& b : eq_filters_[stage]) b.set(spec.type, sample_rate_, spec.frequency, spec.q, spec.gain_db);
    }
}

void ArSonKuPikEngine::update_cached_gains()
{
    ++debug_counters_.gain_updates;
    cached_input_gain_ = db_to_gain(working_.output.input_gain_db + kInternalChainHeadroomDb);
    cached_output_gain_ = db_to_gain(working_.output.output_gain_db + kInternalRestoreDb);
}

void ArSonKuPikEngine::rebuild_color_filters()
{
    ++debug_counters_.color_rebuilds;
    const auto& c = working_.color;
    const double body_center = clamp(c.body_freq, 95.0, 260.0);
    const double warm_center = clamp(c.warmth_freq, 300.0, 760.0);
    const double harmonic_center = clamp(c.harmonics_freq, 1200.0, 3600.0);
    const double air_center = clamp(c.air_freq, 6500.0, 16000.0);
    for (int i = 0; i < 2; ++i) {
        bass_pre_[i].set(EqType::Bell, sample_rate_, clamp(body_center * 0.72, 50.0, 180.0), 0.58, 0.0);
        bass_post_hp_[i].set(EqType::LowCut, sample_rate_, 34.0, 0.707, 0.0);
        bass_post_lp_[i].set(EqType::HighCut, sample_rate_, clamp(body_center * 2.35, 230.0, 420.0), 0.64, 0.0);
        warmth_pre_[i].set(EqType::Bell, sample_rate_, warm_center, 0.78, 0.0);
        presence_pre_[i].set(EqType::Bell, sample_rate_, harmonic_center, 0.68, 0.0);
        air_pre_[i].set(EqType::HighShelf, sample_rate_, air_center, 0.50, 0.0);
        const double velvet = clamp01(c.velvet_treble / 100.0);
        const double repair = clamp01(c.ai_high_repair / 100.0);
        deharsh_[i].set(EqType::Bell, sample_rate_, 6800.0 + repair * 1800.0, 1.10, -0.15 - velvet * 0.65);
    }
    side_air_hp_.set(EqType::LowCut, sample_rate_, 7800.0, 0.707, 0.0);
    side_air_tone_.set(EqType::HighShelf, sample_rate_, 10800.0, 0.58, clamp(c.air * 0.036 + c.god_particles * 0.014, -1.2, 4.8));
    mid_anchor_bp1_.set(EqType::LowCut, sample_rate_, 540.0, 0.707, 0.0);
    mid_anchor_bp2_.set(EqType::HighCut, sample_rate_, 4100.0, 0.707, 0.0);
    vocal_tickle_bp_.set(EqType::Bell, sample_rate_, 1450.0, 0.72, 0.0);
    treble_skin_bp_.set(EqType::Bell, sample_rate_, 8750.0, 0.86, 0.0);
    low_body_bp_.set(EqType::Bell, sample_rate_, clamp(body_center * 1.45, 220.0, 300.0), 0.42, 0.0);
}

void ArSonKuPikEngine::rebuild_width_filters()
{
    ++debug_counters_.width_rebuilds;
    const auto& w = working_.width;
    const double tone = std::max(0.0, w.side_tone);
    const double generated_low_cut = w.mono_bass ? std::max(210.0, w.mono_bass_freq) : 150.0;
    width_pre_hp_.set(EqType::LowCut, sample_rate_, generated_low_cut, 0.707, 0.0);
    width_lowmid_hp_.set(EqType::LowCut, sample_rate_, 360.0, 0.707, 0.0);
    width_lowmid_lp_.set(EqType::HighCut, sample_rate_, 900.0, 0.707, 0.0);
    width_mid_hp_.set(EqType::LowCut, sample_rate_, 900.0, 0.707, 0.0);
    width_mid_lp_.set(EqType::HighCut, sample_rate_, 4200.0 + tone * 22.0, 0.707, 0.0);
    width_high_hp_.set(EqType::LowCut, sample_rate_, 7200.0 + tone * 62.0, 0.707, 0.0);
    width_air_tone_.set(EqType::HighShelf, sample_rate_, 11200.0 + tone * 180.0, 0.62, clamp(w.side_tone * 0.20, -1.2, 4.2));
    width_side_sub_hp_.set(EqType::LowCut, sample_rate_, 22.0, 0.707, 0.0);
    width_side_sub_lp_.set(EqType::HighCut, sample_rate_, clamp(w.mono_bass_freq, 95.0, 170.0), 0.707, 0.0);
    width_side_body_hp_.set(EqType::LowCut, sample_rate_, clamp(w.mono_bass_freq * 0.82, 85.0, 150.0), 0.707, 0.0);
    width_side_body_lp_.set(EqType::HighCut, sample_rate_, 680.0, 0.707, 0.0);
    width_side_mid_hp_.set(EqType::LowCut, sample_rate_, 680.0, 0.707, 0.0);
    width_side_mid_lp_.set(EqType::HighCut, sample_rate_, 3600.0, 0.707, 0.0);
    width_side_high_hp_.set(EqType::LowCut, sample_rate_, 3600.0, 0.707, 0.0);
    width_side_high_lp_.set(EqType::HighCut, sample_rate_, 11200.0, 0.707, 0.0);
    width_side_air_hp_.set(EqType::LowCut, sample_rate_, 7800.0 + tone * 42.0, 0.707, 0.0);
    width_side_air_tone_.set(EqType::HighShelf, sample_rate_, 11200.0 + tone * 190.0, 0.62, clamp(w.side_tone * 0.22, -1.2, 4.6));
}

} // namespace arsonkupik
