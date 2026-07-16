#include <obs-module.h>
#include <media-io/audio-io.h>

#include "arsonkupik_dsp.hpp"
#include "arsonkupik_transition.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("arsonkupik-obs-audio-enhancer", "en-US")

using arsonkupik::ArSonKuPikTransitionProcessor;
using arsonkupik::EngineParams;

namespace {

static_assert(std::atomic<std::uint64_t>::is_always_lock_free,
              "ArSonKuPik requires lock-free 64-bit atomics on supported x64 builds");
static_assert(std::atomic<std::uint32_t>::is_always_lock_free,
              "ArSonKuPik requires lock-free 32-bit atomics on supported builds");

std::uint64_t encode_double(double value)
{
    std::uint64_t bits = 0;
    static_assert(sizeof(bits) == sizeof(value), "unexpected double size");
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

double decode_double(std::uint64_t bits)
{
    double value = 0.0;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

struct AtomicEngineTarget {
    std::atomic<std::uint64_t> sequence{0};
    std::atomic<std::uint32_t> preset_index{0};
    std::atomic<std::uint64_t> enhance{encode_double(65.0)};
    std::atomic<std::uint64_t> smart_bass{encode_double(55.0)};
    std::atomic<std::uint64_t> smart_treble{encode_double(70.0)};
    std::atomic<std::uint64_t> vocal_body{encode_double(65.0)};
    std::atomic<std::uint64_t> stereo_magic{encode_double(85.0)};
    std::atomic<std::uint64_t> output_trim_db{encode_double(-0.55)};
    std::atomic<std::uint32_t> flags{0};

    void publish(const EngineParams& p)
    {
        sequence.fetch_add(1, std::memory_order_acq_rel);
        preset_index.store(p.preset_index, std::memory_order_relaxed);
        enhance.store(encode_double(p.enhance), std::memory_order_relaxed);
        smart_bass.store(encode_double(p.smart_bass), std::memory_order_relaxed);
        smart_treble.store(encode_double(p.smart_treble), std::memory_order_relaxed);
        vocal_body.store(encode_double(p.vocal_body), std::memory_order_relaxed);
        stereo_magic.store(encode_double(p.stereo_magic), std::memory_order_relaxed);
        output_trim_db.store(encode_double(p.output_trim_db), std::memory_order_relaxed);
        const std::uint32_t packed = (p.smart_protect ? 1U : 0U)
                                   | (p.bypass ? 2U : 0U)
                                   | (p.advanced_override ? 4U : 0U);
        flags.store(packed, std::memory_order_relaxed);
        sequence.fetch_add(1, std::memory_order_release);
    }

    EngineParams load_or(const EngineParams& fallback) const
    {
        for (int attempt = 0; attempt < 3; ++attempt) {
            const std::uint64_t before = sequence.load(std::memory_order_acquire);
            if ((before & 1U) != 0U) continue;

            EngineParams p;
            p.preset_index = preset_index.load(std::memory_order_relaxed);
            p.enhance = decode_double(enhance.load(std::memory_order_relaxed));
            p.smart_bass = decode_double(smart_bass.load(std::memory_order_relaxed));
            p.smart_treble = decode_double(smart_treble.load(std::memory_order_relaxed));
            p.vocal_body = decode_double(vocal_body.load(std::memory_order_relaxed));
            p.stereo_magic = decode_double(stereo_magic.load(std::memory_order_relaxed));
            p.output_trim_db = decode_double(output_trim_db.load(std::memory_order_relaxed));
            const std::uint32_t packed = flags.load(std::memory_order_relaxed);
            p.smart_protect = (packed & 1U) != 0U;
            p.bypass = (packed & 2U) != 0U;
            p.advanced_override = (packed & 4U) != 0U;

            const std::uint64_t after = sequence.load(std::memory_order_acquire);
            if (before == after && (after & 1U) == 0U) return p;
        }
        return fallback;
    }
};

struct FilterData {
    obs_source_t* source = nullptr;
    ArSonKuPikTransitionProcessor processor;
    AtomicEngineTarget published_target;
    EngineParams last_requested{};
    std::uint32_t sample_rate = 48000;
    std::size_t channels = 2;
};

const char* get_name(void*)
{
    return obs_module_text("ArSonKuPikSmartEnhancer");
}

std::uint32_t current_sample_rate()
{
    audio_t* audio = obs_get_audio();
    return audio ? audio_output_get_sample_rate(audio) : 48000;
}

std::size_t current_channel_count()
{
    audio_t* audio = obs_get_audio();
    return audio ? std::max<std::size_t>(1, audio_output_get_channels(audio)) : 2;
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

EngineParams read_settings(obs_data_t* settings)
{
    EngineParams p;
    const char* preset = obs_data_get_string(settings, "preset");
    p.preset_index = static_cast<std::uint32_t>(
        arsonkupik::preset_index_from_id(preset && *preset ? preset : "default"));
    p.advanced_override = true;
    p.enhance = obs_data_get_double(settings, "enhance");
    p.smart_bass = obs_data_get_double(settings, "smart_bass");
    p.smart_treble = obs_data_get_double(settings, "smart_treble");
    p.vocal_body = obs_data_get_double(settings, "vocal_body");
    p.stereo_magic = obs_data_get_double(settings, "stereo_magic");
    p.output_trim_db = obs_data_get_double(settings, "output_trim_db");
    p.smart_protect = obs_data_get_bool(settings, "smart_protect");
    p.bypass = obs_data_get_bool(settings, "bypass");
    return p;
}

void* create(obs_data_t* settings, obs_source_t* source)
{
    auto* f = new FilterData();
    f->source = source;
    f->sample_rate = current_sample_rate();
    f->channels = std::min<std::size_t>(current_channel_count(), arsonkupik::kMaxChannels);
    f->last_requested = read_settings(settings);
    f->published_target.publish(f->last_requested);
    f->processor.prepare(f->sample_rate, f->channels, f->last_requested);
    return f;
}

void destroy(void* data)
{
    delete static_cast<FilterData*>(data);
}

void update(void* data, obs_data_t* settings)
{
    auto* f = static_cast<FilterData*>(data);
    if (!f) return;
    f->published_target.publish(read_settings(settings));
}

void get_defaults(obs_data_t* settings)
{
    const auto& p = arsonkupik::default_preset();
    obs_data_set_default_string(settings, "preset", p.id.c_str());
    obs_data_set_default_double(settings, "enhance", p.macro.enhance);
    obs_data_set_default_double(settings, "smart_bass", p.macro.smart_bass);
    obs_data_set_default_double(settings, "smart_treble", p.macro.smart_treble);
    obs_data_set_default_double(settings, "vocal_body", p.macro.vocal_body);
    obs_data_set_default_double(settings, "stereo_magic", p.macro.stereo_magic);
    obs_data_set_default_double(settings, "output_trim_db", p.output.output_gain_db);
    obs_data_set_default_bool(settings, "smart_protect", p.macro.smart_protect);
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

    const std::uint32_t sr = current_sample_rate();
    const std::size_t channels = std::min<std::size_t>(
        current_channel_count(), arsonkupik::kMaxChannels);
    const EngineParams requested = f->published_target.load_or(f->last_requested);

    if (sr != f->sample_rate || channels != f->channels) {
        f->sample_rate = sr;
        f->channels = channels;
        f->processor.prepare(f->sample_rate, f->channels, requested);
    }

    float* planes[arsonkupik::kMaxChannels]{};
    for (std::size_t ch = 0; ch < f->channels; ++ch) {
        planes[ch] = reinterpret_cast<float*>(audio->data[ch]);
    }
    if (!planes[0]) return audio;

    f->processor.process(planes, f->channels, audio->frames, requested);
    f->last_requested = requested;
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