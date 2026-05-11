#include "audio_visualizer_bus_utils.h"

#include <godot_cpp/classes/audio_effect.hpp>
#include <godot_cpp/classes/audio_effect_capture.hpp>
#include <godot_cpp/classes/audio_effect_instance.hpp>
#include <godot_cpp/classes/audio_effect_spectrum_analyzer.hpp>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/engine.hpp>

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
				uint64_t frame = UINT64_MAX;
				PackedVector2Array frames;
				bool found_capture = false;
			};

			std::vector<CaptureFrameCache> capture_frame_cache;

			uint64_t get_process_frame() {
				Engine* engine = Engine::get_singleton();
				return engine ? engine->get_process_frames() : 0;
			}

			CaptureFrameCache& get_cache_for_bus(const StringName& p_bus_name) {
				for (CaptureFrameCache& cache : capture_frame_cache) {
					if (cache.bus_name == p_bus_name) {
						return cache;
					}
				}

				CaptureFrameCache cache;
				cache.bus_name = p_bus_name;
				capture_frame_cache.push_back(cache);
				return capture_frame_cache.back();
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

		bool get_capture_frames(const StringName& p_bus_name, int p_max_frames, PackedVector2Array& r_frames) {
			r_frames.clear();

			const uint64_t current_frame = get_process_frame();
			CaptureFrameCache& cache = get_cache_for_bus(p_bus_name);
			if (cache.frame == current_frame) {
				r_frames = cache.frames;
				return cache.found_capture;
			}

			cache.frame = current_frame;
			cache.frames.clear();
			cache.found_capture = false;

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
	}
}
