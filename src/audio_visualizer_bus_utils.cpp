#include "audio_visualizer_bus_utils.h"

#include <godot_cpp/classes/audio_effect.hpp>
#include <godot_cpp/classes/audio_effect_capture.hpp>
#include <godot_cpp/classes/audio_effect_instance.hpp>
#include <godot_cpp/classes/audio_effect_spectrum_analyzer.hpp>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace godot {
	namespace AudioVisualizerBusUtils {
		namespace {
			constexpr int CAPTURE_CACHE_MAX_FRAMES = 8192;

			struct CaptureFrameCache {
				StringName bus_name;
				int backend = BUS_BACKEND_GODOT;
				uint64_t frame = UINT64_MAX;
				PackedVector2Array frames;
				bool found_capture = false;
			};

			std::vector<CaptureFrameCache> capture_frame_cache;

			uint64_t get_process_frame() {
				Engine* engine = Engine::get_singleton();
				return engine ? engine->get_process_frames() : 0;
			}

			CaptureFrameCache& get_cache_for_bus(const StringName& p_bus_name, int p_backend) {
				for (CaptureFrameCache& cache : capture_frame_cache) {
					if (cache.bus_name == p_bus_name && cache.backend == p_backend) {
						return cache;
					}
				}

				CaptureFrameCache cache;
				cache.bus_name = p_bus_name;
				cache.backend = p_backend;
				capture_frame_cache.push_back(cache);
				return capture_frame_cache.back();
			}

			Object* get_fmod_audio_bus_layout() {
				if (!ClassDB::class_exists(StringName("FmodServer"))) {
					return nullptr;
				}

				Variant layout_variant = ClassDB::class_call_static(StringName("FmodServer"), StringName("get_audio_bus_layout"));
				return layout_variant;
			}

			Object* get_fmod_audio_bus(const StringName& p_bus_name) {
				Object* layout = get_fmod_audio_bus_layout();
				if (!layout) {
					return nullptr;
				}

				const String bus_name = String(p_bus_name);
				const Variant has_bus_variant = layout->call(StringName("has_audio_bus"), bus_name);
				if (has_bus_variant.get_type() != Variant::BOOL || !static_cast<bool>(has_bus_variant)) {
					return nullptr;
				}

				Variant bus_variant = layout->call(StringName("get_audio_bus"), bus_name);
				return bus_variant;
			}

			Object* get_fmod_channel_group(const StringName& p_bus_name) {
				Object* audio_bus = get_fmod_audio_bus(p_bus_name);
				if (!audio_bus) {
					return nullptr;
				}

				Variant channel_group_variant = audio_bus->call(StringName("get_bus"));
				return channel_group_variant;
			}

			bool is_fmod_bus_active(const StringName& p_bus_name) {
				Object* channel_group = get_fmod_channel_group(p_bus_name);
				if (!channel_group) {
					return true;
				}

				const Variant playing_variant = channel_group->call(StringName("is_playing"));
				if (playing_variant.get_type() == Variant::BOOL && !static_cast<bool>(playing_variant)) {
					return false;
				}

				const Variant paused_variant = channel_group->call(StringName("get_paused"));
				if (paused_variant.get_type() == Variant::BOOL && static_cast<bool>(paused_variant)) {
					return false;
				}

				const Variant audibility_variant = channel_group->call(StringName("get_audibility"));
				const Variant::Type audibility_type = audibility_variant.get_type();
				if (audibility_type == Variant::FLOAT || audibility_type == Variant::INT) {
					const float audibility = static_cast<float>(static_cast<double>(audibility_variant));
					if (audibility <= 0.00001f) {
						return false;
					}
				}

				return true;
			}

			Object* get_fmod_bus_effect(const StringName& p_bus_name, const StringName& p_effect_class) {
				Object* layout = get_fmod_audio_bus_layout();
				if (!layout) {
					return nullptr;
				}

				const String bus_name = String(p_bus_name);
				const Variant has_bus_variant = layout->call(StringName("has_audio_bus"), bus_name);
				if (has_bus_variant.get_type() != Variant::BOOL || !static_cast<bool>(has_bus_variant)) {
					return nullptr;
				}

				const Variant effect_count_variant = layout->call(StringName("get_bus_effect_count"), bus_name);
				const int effect_count = std::max(0, static_cast<int>(static_cast<int64_t>(effect_count_variant)));
				for (int effect_index = 0; effect_index < effect_count; effect_index++) {
					Variant effect_variant = layout->call(StringName("get_bus_effect"), bus_name, effect_index);
					Object* effect = effect_variant;
					if (effect && effect->is_class(String(p_effect_class))) {
						return effect;
					}
				}

				return nullptr;
			}

			bool get_fmod_spectrum_magnitude_for_frequency_range(const StringName& p_bus_name, float p_begin_hz, float p_end_hz, Vector2& r_magnitude) {
				if (!is_fmod_bus_active(p_bus_name)) {
					r_magnitude = Vector2();
					return true;
				}

				Object* analyzer = get_fmod_bus_effect(p_bus_name, StringName("FmodAudioEffectSpectrumAnalyzer"));
				if (!analyzer) {
					return false;
				}

				const Variant magnitude_variant = analyzer->call(StringName("get_magnitude_for_frequency_range"), p_begin_hz, p_end_hz, 1);
				if (magnitude_variant.get_type() != Variant::VECTOR2) {
					return false;
				}

				r_magnitude = magnitude_variant;
				return true;
			}

			bool get_fmod_capture_frames(const StringName& p_bus_name, int p_max_frames, PackedVector2Array& r_frames) {
				if (!is_fmod_bus_active(p_bus_name)) {
					return true;
				}

				Object* capture = get_fmod_bus_effect(p_bus_name, StringName("FmodAudioEffectCapture"));
				if (!capture) {
					return false;
				}

				const Variant available_variant = capture->call(StringName("get_frames_available"));
				const int available = std::max(0, static_cast<int>(static_cast<int64_t>(available_variant)));
				const int frames_to_read = std::min(available, std::max(std::max(1, p_max_frames), CAPTURE_CACHE_MAX_FRAMES));
				if (frames_to_read > 0) {
					const Variant frames_variant = capture->call(StringName("get_buffer"), frames_to_read);
					if (frames_variant.get_type() == Variant::PACKED_VECTOR2_ARRAY) {
						r_frames = frames_variant;
					}
				}

				return true;
			}
		}

		float clamp_float(float p_value, float p_min, float p_max) {
			if (p_value < p_min) {
				return p_min;
			}

			if (p_value > p_max) {
				return p_max;
			}

			return p_value;
		}

		float linear_to_db(float p_linear) {
			if (p_linear <= 1e-20f) {
				return -200.0f;
			}

			return 20.0f * std::log10(p_linear);
		}

		float db_to_normalized(float p_db, float p_min_db, float p_max_db) {
			if (p_max_db <= p_min_db) {
				return 0.0f;
			}

			return clamp_float((p_db - p_min_db) / (p_max_db - p_min_db), 0.0f, 1.0f);
		}

		String get_bus_hint_string(int p_backend) {
			(void)p_backend;

			String bus_list;
			AudioServer* audio_server = AudioServer::get_singleton();

			if (!audio_server) {
				return bus_list;
			}

			const int bus_count = audio_server->get_bus_count();
			for (int i = 0; i < bus_count; i++) {
				if (i > 0) {
					bus_list += ",";
				}
				bus_list += audio_server->get_bus_name(i);
			}

			return bus_list;
		}

		bool get_spectrum_magnitude_for_frequency_range(const StringName& p_bus_name, int p_backend, float p_begin_hz, float p_end_hz, Vector2& r_magnitude) {
			r_magnitude = Vector2();

			if (p_backend == BUS_BACKEND_FMOD_PLAYER) {
				return get_fmod_spectrum_magnitude_for_frequency_range(p_bus_name, p_begin_hz, p_end_hz, r_magnitude);
			}

			Ref<AudioEffectSpectrumAnalyzerInstance> analyzer_instance;
			if (!get_spectrum_analyzer_instance(p_bus_name, analyzer_instance)) {
				return false;
			}

			r_magnitude = analyzer_instance->get_magnitude_for_frequency_range(p_begin_hz, p_end_hz, AudioEffectSpectrumAnalyzerInstance::MAGNITUDE_MAX);
			return true;
		}

		bool get_spectrum_analyzer_instance(const StringName& p_bus_name, Ref<AudioEffectSpectrumAnalyzerInstance>& r_instance) {
			r_instance.unref();

			AudioServer* audio_server = AudioServer::get_singleton();
			if (!audio_server) {
				return false;
			}

			const int bus_index = audio_server->get_bus_index(p_bus_name);
			if (bus_index < 0) {
				return false;
			}

			const int effect_count = audio_server->get_bus_effect_count(bus_index);
			for (int effect_index = 0; effect_index < effect_count; effect_index++) {
				if (!audio_server->is_bus_effect_enabled(bus_index, effect_index)) {
					continue;
				}

				Ref<AudioEffect> effect = audio_server->get_bus_effect(bus_index, effect_index);
				Ref<AudioEffectSpectrumAnalyzer> analyzer = effect;
				if (!analyzer.is_valid()) {
					continue;
				}

				Ref<AudioEffectInstance> effect_instance = audio_server->get_bus_effect_instance(bus_index, effect_index);
				Ref<AudioEffectSpectrumAnalyzerInstance> analyzer_instance = effect_instance;
				if (!analyzer_instance.is_valid()) {
					continue;
				}

				r_instance = analyzer_instance;
				return true;
			}

			return false;
		}

		bool get_capture_effect(const StringName& p_bus_name, Ref<AudioEffectCapture>& r_capture) {
			r_capture.unref();

			AudioServer* audio_server = AudioServer::get_singleton();
			if (!audio_server) {
				return false;
			}

			const int bus_index = audio_server->get_bus_index(p_bus_name);
			if (bus_index < 0) {
				return false;
			}

			const int effect_count = audio_server->get_bus_effect_count(bus_index);
			for (int effect_index = 0; effect_index < effect_count; effect_index++) {
				if (!audio_server->is_bus_effect_enabled(bus_index, effect_index)) {
					continue;
				}

				Ref<AudioEffect> effect = audio_server->get_bus_effect(bus_index, effect_index);
				Ref<AudioEffectCapture> capture = effect;
				if (!capture.is_valid()) {
					continue;
				}

				r_capture = capture;
				return true;
			}

			return false;
		}

		bool get_capture_frames(const StringName& p_bus_name, int p_backend, int p_max_frames, PackedVector2Array& r_frames) {
			r_frames.clear();

			const uint64_t current_frame = get_process_frame();
			CaptureFrameCache& cache = get_cache_for_bus(p_bus_name, p_backend);
			if (cache.frame == current_frame) {
				r_frames = cache.frames;
				return cache.found_capture;
			}

			cache.frame = current_frame;
			cache.frames.clear();
			cache.found_capture = false;

			if (p_backend == BUS_BACKEND_FMOD_PLAYER) {
				cache.found_capture = get_fmod_capture_frames(p_bus_name, p_max_frames, cache.frames);
				r_frames = cache.frames;
				return cache.found_capture;
			}

			Ref<AudioEffectCapture> capture;
			if (!get_capture_effect(p_bus_name, capture)) {
				return false;
			}

			cache.found_capture = true;
			const int available = capture->get_frames_available();
			const int frames_to_read = std::min(available, std::max(std::max(1, p_max_frames), CAPTURE_CACHE_MAX_FRAMES));
			if (frames_to_read > 0) {
				cache.frames = capture->get_buffer(frames_to_read);
			}

			r_frames = cache.frames;
			return true;
		}

		bool get_capture_frames(const StringName& p_bus_name, int p_max_frames, PackedVector2Array& r_frames) {
			return get_capture_frames(p_bus_name, BUS_BACKEND_GODOT, p_max_frames, r_frames);
		}
	}
}
