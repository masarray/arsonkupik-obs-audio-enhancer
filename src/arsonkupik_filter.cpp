#include <obs-module.h>
#include <media-io/audio-io.h>

#include "arsonkupik_dsp.hpp"

#include <algorithm>
#include <memory>
#include <string>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("arsonkupik-obs-audio-enhancer", "en-US")

using arsonkupik::ArSonKuPikEngine;
using arsonkupik::RuntimeParams;

namespace {
struct FilterData {
    obs_source_t* source = nullptr;
    ArSonKuPikEngine engine;
    RuntimeParams params;
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
    const arsonkupik::Preset* p = arsonkupik::find_preset(preset && *preset ? preset : "default");
    return p ? *p : arsonkupik::default_preset();
}

void sync_settings_to_preset(obs_data_t* settings)
{
    const auto& p = preset_from_settings(settings);

    // Keep the visible OBS sliders aligned with the selected factory preset.
    // These are macro values for the user; the real DSP recipe remains the
    // hidden canonical preset in arsonkupik_presets.cpp.
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
    // When the user chooses a preset, show that preset's macro values in the UI.
    // This avoids the confusing old behavior where every preset visually looked
    // like MasAri even though the DSP recipe had changed.
    sync_settings_to_preset(settings);
    return true;
}

bool manual_tuning_modified(obs_properties_t*, obs_property_t*, obs_data_t* settings)
{
    // Returning from manual override to preset mode should restore the selected
    // preset's frozen macro values instead of leaving stale manual numbers.
    if (!obs_data_get_bool(settings, "manual_tuning")) {
        sync_settings_to_preset(settings);
    }
    return true;
}

void read_settings(FilterData* f, obs_data_t* settings)
{
    RuntimeParams p;
    const char* preset = obs_data_get_string(settings, "preset");
    p.preset_id = preset && *preset ? preset : "default";
    p.advanced_override = obs_data_get_bool(settings, "manual_tuning");

    const arsonkupik::Preset* preset_cfg = arsonkupik::find_preset(p.preset_id);
    if (!preset_cfg) preset_cfg = &arsonkupik::default_preset();

    if (p.advanced_override) {
        p.enhance = obs_data_get_double(settings, "enhance");
        p.smart_bass = obs_data_get_double(settings, "smart_bass");
        p.smart_treble = obs_data_get_double(settings, "smart_treble");
        p.vocal_body = obs_data_get_double(settings, "vocal_body");
        p.stereo_magic = obs_data_get_double(settings, "stereo_magic");
        p.output_trim_db = obs_data_get_double(settings, "output_trim_db");
        p.smart_protect = obs_data_get_bool(settings, "smart_protect");
    } else {
        // Preset mode must be a true frozen factory recipe. Do not let stale UI
        // sliders affect the sound when Manual macro tuning is off.
        p.enhance = preset_cfg->macro.enhance;
        p.smart_bass = preset_cfg->macro.smart_bass;
        p.smart_treble = preset_cfg->macro.smart_treble;
        p.vocal_body = preset_cfg->macro.vocal_body;
        p.stereo_magic = preset_cfg->macro.stereo_magic;
        p.output_trim_db = preset_cfg->output.output_gain_db;
        p.smart_protect = preset_cfg->macro.smart_protect;
    }

    p.bypass = obs_data_get_bool(settings, "bypass");
    f->params = p;
}

void* create(obs_data_t* settings, obs_source_t* source)
{
    auto* f = new FilterData();
    f->source = source;
    read_settings(f, settings);
    f->sample_rate = current_sample_rate();
    f->channels = current_channel_count();
    f->engine.prepare(f->sample_rate, f->channels);
    f->engine.set_runtime_params(f->params);
    return f;
}

void destroy(void* data)
{
    delete static_cast<FilterData*>(data);
}

void update(void* data, obs_data_t* settings)
{
    auto* f = static_cast<FilterData*>(data);
    read_settings(f, settings);
    const uint32_t sr = current_sample_rate();
    const size_t ch = current_channel_count();
    if (sr != f->sample_rate || ch != f->channels) {
        f->sample_rate = sr;
        f->channels = ch;
        f->engine.prepare(sr, ch);
    }
    f->engine.set_runtime_params(f->params);
}

void get_defaults(obs_data_t* settings)
{
    obs_data_set_default_string(settings, "preset", "default");
    obs_data_set_default_bool(settings, "manual_tuning", false);
    obs_data_set_default_double(settings, "enhance", 65.0);
    obs_data_set_default_double(settings, "smart_bass", 55.0);
    obs_data_set_default_double(settings, "smart_treble", 70.0);
    obs_data_set_default_double(settings, "vocal_body", 65.0);
    obs_data_set_default_double(settings, "stereo_magic", 85.0);
    obs_data_set_default_double(settings, "output_trim_db", -0.55);
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

    obs_property_t* manual = obs_properties_add_bool(props, "manual_tuning", obs_module_text("ManualTuning"));
    obs_property_set_modified_callback(manual, manual_tuning_modified);

    obs_properties_add_float_slider(props, "enhance", obs_module_text("Enhance"), 0.0, 100.0, 1.0);
    obs_properties_add_float_slider(props, "smart_bass", obs_module_text("SmartBass"), 0.0, 100.0, 1.0);
    obs_properties_add_float_slider(props, "smart_treble", obs_module_text("SmartTreble"), 0.0, 100.0, 1.0);
    obs_properties_add_float_slider(props, "vocal_body", obs_module_text("VocalBody"), 0.0, 100.0, 1.0);
    obs_properties_add_float_slider(props, "stereo_magic", obs_module_text("StereoMagic"), 0.0, 100.0, 1.0);
    obs_properties_add_float_slider(props, "output_trim_db", obs_module_text("OutputTrim"), -12.0, 6.0, 0.1);

    obs_properties_add_text(props, "note", obs_module_text("Note"), OBS_TEXT_INFO);
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
    }

    float* planes[arsonkupik::kMaxChannels] = {};
    const size_t channels = std::min<size_t>(f->channels, arsonkupik::kMaxChannels);
    for (size_t i = 0; i < channels; ++i) {
        planes[i] = reinterpret_cast<float*>(audio->data[i]);
    }
    if (!planes[0]) return audio;
    f->engine.process(planes, channels, audio->frames);
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
