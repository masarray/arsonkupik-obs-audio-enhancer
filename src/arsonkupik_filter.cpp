#include <obs-module.h>
#include <media-io/audio-io.h>

#include "arsonkupik_dsp.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("arsonkupik-obs-audio-enhancer", "en-US")

using arsonkupik::ArSonKuPikEngine;
using arsonkupik::RuntimeParams;

namespace {
constexpr double kPi = 3.1415926535897932384626433832795;
constexpr double kKnobSmoothingSeconds = 0.080;
constexpr double kPresetFadeSeconds = 0.005;

struct FilterData {
    obs_source_t* source = nullptr;
    ArSonKuPikEngine engine;
    RuntimeParams params;
    RuntimeParams target_params;
    RuntimeParams applied_params;
    RuntimeParams pending_preset_params;
    bool pending_preset_switch = false;
    bool fade_in_after_preset = false;
    uint32_t sample_rate = 48000;
    size_t channels = 2;
};

const char* get_name(void*)
{
    return obs_module_text("ArSonKuPikSmartEnhancer");
}

uint32_t current_sample_rate()
{
    audio_t* audio = obs_get_audio();
    return audio ? audio_output_get_sample_rate(audio) : 48000;
}

size_t current_channel_count()
{
    audio_t* audio = obs_get_audio();
    return audio ? std::max<size_t>(1, audio_output_get_channels(audio)) : 2;
}

const arsonkupik::Preset& preset_from_settings(obs_data_t* settings)
{
    const char* preset = obs_data_get_string(settings, "preset");
    const arsonkupik::Preset* p = preset && *preset ? arsonkupik::find_preset(preset) : nullptr;
    return p ? *p : arsonkupik::default_preset();
}

void sync_settings_to_preset(obs_data_t* settings)
{
    const auto& p = preset_from_settings(settings);
    obs_data_set_double(settings, "enhance", p.macro.enhance);
    obs_data_set_double(settings, "smart_bass", p.macro.smart_bass);
    obs_data_set_double(settings, "smart_treble", p.macro.smart_treble);
    obs_data_set_double(settings, "vocal_body", p.macro.vocal_body);
    obs_data_set_double(settings, "stereo_magic", p.macro.stereo_magic);
    obs_data_set_double(settings, "output_trim_db", p.output.output_gain_db);
    obs_data_set_bool(settings, "smart_protect", p.macro.smart_protect);
}

bool preset_modified(obs_properties_t*, obs_property_t*, obs_data_t* settings)
{
    sync_settings_to_preset(settings);
    return true;
}

RuntimeParams read_settings(obs_data_t* settings)
{
    RuntimeParams p;
    const char* preset = obs_data_get_string(settings, "preset");
    p.preset_id = preset && *preset ? preset : "default";
    p.bypass = obs_data_get_bool(settings, "bypass");

    // Sliders are always live. Preset selection loads a recipe into the sliders,
    // then every slider edit becomes the target macro value. Smoothing happens
    // per audio block so mouse automation does not zipper/crackle.
    p.advanced_override = true;
    p.enhance = obs_data_get_double(settings, "enhance");
    p.smart_bass = obs_data_get_double(settings, "smart_bass");
    p.smart_treble = obs_data_get_double(settings, "smart_treble");
    p.vocal_body = obs_data_get_double(settings, "vocal_body");
    p.stereo_magic = obs_data_get_double(settings, "stereo_magic");
    p.output_trim_db = obs_data_get_double(settings, "output_trim_db");
    p.smart_protect = obs_data_get_bool(settings, "smart_protect");
    return p;
}

bool runtime_params_close(const RuntimeParams& a, const RuntimeParams& b)
{
    auto near = [](double x, double y, double eps) { return std::abs(x - y) <= eps; };
    return a.preset_id == b.preset_id &&
           a.advanced_override == b.advanced_override &&
           a.smart_protect == b.smart_protect &&
           a.bypass == b.bypass &&
           near(a.enhance, b.enhance, 0.020) &&
           near(a.smart_bass, b.smart_bass, 0.020) &&
           near(a.smart_treble, b.smart_treble, 0.020) &&
           near(a.vocal_body, b.vocal_body, 0.020) &&
           near(a.stereo_magic, b.stereo_magic, 0.020) &&
           near(a.output_trim_db, b.output_trim_db, 0.005);
}

bool smooth_runtime_params(FilterData* f, std::size_t frames)
{
    const RuntimeParams before = f->params;
    const double denom = std::max(1.0, static_cast<double>(f->sample_rate) * kKnobSmoothingSeconds);
    const double alpha = 1.0 - std::exp(-static_cast<double>(frames) / denom);
    auto smooth = [alpha](double current, double target, double snap_eps) {
        const double next = current + (target - current) * alpha;
        return std::abs(next - target) <= snap_eps ? target : next;
    };

    // The preset id is topology; continuous knobs are smoothed inside that topology.
    // Snap-to-target stops infinite tiny retunes after the knob has audibly settled.
    f->params.preset_id = f->target_params.preset_id;
    f->params.enhance = smooth(f->params.enhance, f->target_params.enhance, 0.010);
    f->params.smart_bass = smooth(f->params.smart_bass, f->target_params.smart_bass, 0.010);
    f->params.smart_treble = smooth(f->params.smart_treble, f->target_params.smart_treble, 0.010);
    f->params.vocal_body = smooth(f->params.vocal_body, f->target_params.vocal_body, 0.010);
    f->params.stereo_magic = smooth(f->params.stereo_magic, f->target_params.stereo_magic, 0.010);
    f->params.output_trim_db = smooth(f->params.output_trim_db, f->target_params.output_trim_db, 0.003);
    f->params.smart_protect = f->target_params.smart_protect;
    f->params.bypass = f->target_params.bypass;
    f->params.advanced_override = true;
    return !runtime_params_close(before, f->params);
}

void apply_equal_power_fade(float** planes, std::size_t channels, std::size_t frames, uint32_t sample_rate, bool fade_in)
{
    if (!planes || channels == 0 || frames == 0) return;
    const std::size_t fade_frames = std::max<std::size_t>(1, std::min<std::size_t>(frames, static_cast<std::size_t>(sample_rate * kPresetFadeSeconds)));
    const double denom = static_cast<double>(std::max<std::size_t>(1, fade_frames - 1));

    for (std::size_t i = 0; i < frames; ++i) {
        double gain = 1.0;
        if (i < fade_frames) {
            const double t = static_cast<double>(i) / denom;
            gain = fade_in ? std::sin(t * kPi * 0.5) : std::cos(t * kPi * 0.5);
        } else if (!fade_in) {
            gain = 0.0;
        }
        for (std::size_t ch = 0; ch < channels; ++ch) {
            if (planes[ch]) planes[ch][i] = static_cast<float>(planes[ch][i] * gain);
        }
    }
}

void* create(obs_data_t* settings, obs_source_t* source)
{
    auto* f = new FilterData();
    f->source = source;
    f->params = read_settings(settings);
    f->target_params = f->params;
    f->pending_preset_params = f->params;
    f->sample_rate = current_sample_rate();
    f->channels = current_channel_count();
    f->engine.prepare(f->sample_rate, f->channels);
    f->engine.set_runtime_params(f->params);
    f->applied_params = f->params;
    return f;
}

void destroy(void* data)
{
    delete static_cast<FilterData*>(data);
}

void update(void* data, obs_data_t* settings)
{
    auto* f = static_cast<FilterData*>(data);
    const RuntimeParams next = read_settings(settings);

    // Preset changes can alter EQ topology. Defer the actual swap to the audio
    // callback and micro-fade around it, instead of rebuilding immediately from
    // the UI thread while audio is hot.
    if (next.preset_id != f->target_params.preset_id) {
        f->pending_preset_params = next;
        f->pending_preset_switch = true;
    } else {
        f->target_params = next;
    }

    const uint32_t sr = current_sample_rate();
    const size_t ch = current_channel_count();
    if (sr != f->sample_rate || ch != f->channels) {
        f->sample_rate = sr;
        f->channels = ch;
        f->engine.prepare(sr, ch);
        f->engine.set_runtime_params(f->params);
        f->applied_params = f->params;
    }
}

void get_defaults(obs_data_t* settings)
{
    obs_data_set_default_string(settings, "preset", "default");
    obs_data_set_default_double(settings, "enhance", 78.0);
    obs_data_set_default_double(settings, "smart_bass", 68.0);
    obs_data_set_default_double(settings, "smart_treble", 80.0);
    obs_data_set_default_double(settings, "vocal_body", 76.0);
    obs_data_set_default_double(settings, "stereo_magic", 88.0);
    obs_data_set_default_double(settings, "output_trim_db", 0.75);
    obs_data_set_default_bool(settings, "smart_protect", true);
    obs_data_set_default_bool(settings, "bypass", false);
}

obs_properties_t* get_properties(void*)
{
    obs_properties_t* props = obs_properties_create();

    obs_property_t* preset = obs_properties_add_list(
        props, "preset", obs_module_text("Preset"),
        OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    for (const auto& p : arsonkupik::factory_presets()) {
        obs_property_list_add_string(preset, p.name.c_str(), p.id.c_str());
    }
    obs_property_set_modified_callback(preset, preset_modified);

    obs_properties_add_bool(props, "bypass", obs_module_text("Bypass"));
    obs_properties_add_bool(props, "smart_protect", obs_module_text("SmartProtect"));

    obs_properties_add_float_slider(props, "enhance", obs_module_text("Enhance"), 0.0, 100.0, 1.0);
    obs_properties_add_float_slider(props, "smart_bass", obs_module_text("SmartBass"), 0.0, 100.0, 1.0);
    obs_properties_add_float_slider(props, "smart_treble", obs_module_text("SmartTreble"), 0.0, 100.0, 1.0);
    obs_properties_add_float_slider(props, "vocal_body", obs_module_text("VocalBody"), 0.0, 100.0, 1.0);
    obs_properties_add_float_slider(props, "stereo_magic", obs_module_text("StereoMagic"), 0.0, 100.0, 1.0);
    obs_properties_add_float_slider(props, "output_trim_db", obs_module_text("OutputTrim"), -6.0, 6.0, 0.1);

    return props;
}

obs_audio_data* filter_audio(void* data, obs_audio_data* audio)
{
    auto* f = static_cast<FilterData*>(data);
    if (!f || !audio || audio->frames == 0) return audio;

    const uint32_t sr = current_sample_rate();
    const size_t ch = current_channel_count();
    if (sr != f->sample_rate || ch != f->channels) {
        f->sample_rate = sr;
        f->channels = ch;
        f->engine.prepare(sr, ch);
        f->engine.set_runtime_params(f->params);
        f->applied_params = f->params;
    }

    float* planes[arsonkupik::kMaxChannels] = {};
    const size_t channels = std::min<size_t>(f->channels, arsonkupik::kMaxChannels);
    for (size_t i = 0; i < channels; ++i) {
        planes[i] = reinterpret_cast<float*>(audio->data[i]);
    }
    if (!planes[0]) return audio;

    if (f->pending_preset_switch) {
        f->engine.process(planes, channels, audio->frames);
        apply_equal_power_fade(planes, channels, audio->frames, f->sample_rate, false);

        f->params = f->pending_preset_params;
        f->target_params = f->pending_preset_params;
        f->pending_preset_switch = false;
        f->fade_in_after_preset = true;
        f->engine.set_runtime_params(f->params);
        f->applied_params = f->params;
        return audio;
    }

    const bool smoothed = smooth_runtime_params(f, audio->frames);
    if (smoothed || !runtime_params_close(f->applied_params, f->params)) {
        f->engine.set_runtime_params(f->params);
        f->applied_params = f->params;
    }
    f->engine.process(planes, channels, audio->frames);

    if (f->fade_in_after_preset) {
        apply_equal_power_fade(planes, channels, audio->frames, f->sample_rate, true);
        f->fade_in_after_preset = false;
    }

    return audio;
}

} // namespace

extern "C" {
static obs_source_info arsonkupik_smart_enhancer_filter = {};

bool obs_module_load(void)
{
    arsonkupik_smart_enhancer_filter.id = "arsonkupik_smart_enhancer_filter";
    arsonkupik_smart_enhancer_filter.type = OBS_SOURCE_TYPE_FILTER;
    arsonkupik_smart_enhancer_filter.output_flags = OBS_SOURCE_AUDIO;
    arsonkupik_smart_enhancer_filter.get_name = get_name;
    arsonkupik_smart_enhancer_filter.create = create;
    arsonkupik_smart_enhancer_filter.destroy = destroy;
    arsonkupik_smart_enhancer_filter.get_defaults = get_defaults;
    arsonkupik_smart_enhancer_filter.get_properties = get_properties;
    arsonkupik_smart_enhancer_filter.update = update;
    arsonkupik_smart_enhancer_filter.filter_audio = filter_audio;

    obs_register_source(&arsonkupik_smart_enhancer_filter);
    blog(LOG_INFO, "ArSonKuPik Smart Enhancer loaded");
    return true;
}

MODULE_EXPORT const char* obs_module_description(void)
{
    return "ArSonKuPik Smart Audio Enhancer native OBS audio filter";
}
}
