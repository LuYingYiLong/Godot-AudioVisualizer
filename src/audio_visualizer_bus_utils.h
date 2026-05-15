#ifndef AUDIO_VISUALIZER_BUS_UTILS_H
#define AUDIO_VISUALIZER_BUS_UTILS_H

#include <godot_cpp/classes/audio_effect_capture.hpp>
#include <godot_cpp/classes/audio_effect_spectrum_analyzer_instance.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace godot {
	namespace AudioVisualizerBusUtils {
		enum BusBackend {
			BUS_BACKEND_GODOT = 0,
			BUS_BACKEND_FMOD_PLAYER = 1
		};

		float clamp_float(float p_value, float p_min, float p_max);
		float linear_to_db(float p_linear);
		float db_to_normalized(float p_db, float p_min_db, float p_max_db);
		String get_bus_hint_string(int p_backend);
		bool get_spectrum_magnitude_for_frequency_range(const StringName& p_bus_name, int p_backend, float p_begin_hz, float p_end_hz, Vector2& r_magnitude);
		bool get_spectrum_analyzer_instance(const StringName& p_bus_name, Ref<AudioEffectSpectrumAnalyzerInstance>& r_instance);
		bool get_capture_effect(const StringName& p_bus_name, Ref<AudioEffectCapture>& r_capture);
		bool get_capture_frames(const StringName& p_bus_name, int p_backend, int p_max_frames, PackedVector2Array& r_frames);
		bool get_capture_frames(const StringName& p_bus_name, int p_max_frames, PackedVector2Array& r_frames);
	}
}

#endif // AUDIO_VISUALIZER_BUS_UTILS_H
