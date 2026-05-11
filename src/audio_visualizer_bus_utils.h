#ifndef AUDIO_VISUALIZER_BUS_UTILS_H
#define AUDIO_VISUALIZER_BUS_UTILS_H

#include <godot_cpp/classes/audio_effect_capture.hpp>
#include <godot_cpp/classes/audio_effect_spectrum_analyzer_instance.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {
	namespace AudioVisualizerBusUtils {
		float clamp_float(float p_value, float p_min, float p_max);
		float linear_to_db(float p_linear);
		float db_to_normalized(float p_db, float p_min_db, float p_max_db);
		bool get_spectrum_analyzer_instance(const StringName& p_bus_name, Ref<AudioEffectSpectrumAnalyzerInstance>& r_instance);
		bool get_capture_effect(const StringName& p_bus_name, Ref<AudioEffectCapture>& r_capture);
		bool get_capture_frames(const StringName& p_bus_name, int p_max_frames, PackedVector2Array& r_frames);
	}
}

#endif // AUDIO_VISUALIZER_BUS_UTILS_H
