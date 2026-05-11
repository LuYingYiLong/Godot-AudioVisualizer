#include "realtime_spectrum_visualizer.h"

#include "audio_visualizer_bus_utils.h"

#include <godot_cpp/classes/audio_effect_spectrum_analyzer_instance.hpp>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>

#include <algorithm>
#include <cmath>

namespace godot {
	namespace {
		constexpr float MIN_LINEAR = 1e-7f;

		float mix_float(float p_from, float p_to, float p_weight) {
			return p_from + (p_to - p_from) * p_weight;
		}

		Color mix_color(const Color& p_from, const Color& p_to, float p_weight) {
			return p_from.lerp(p_to, AudioVisualizerBusUtils::clamp_float(p_weight, 0.0f, 1.0f));
		}
	}

	void RealtimeSpectrumVisualizer::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_use_bus", "enabled"), &RealtimeSpectrumVisualizer::set_use_bus);
		ClassDB::bind_method(D_METHOD("get_use_bus"), &RealtimeSpectrumVisualizer::get_use_bus);
		ClassDB::bind_method(D_METHOD("set_bus", "bus"), &RealtimeSpectrumVisualizer::set_bus);
		ClassDB::bind_method(D_METHOD("get_bus"), &RealtimeSpectrumVisualizer::get_bus);
		ClassDB::bind_method(D_METHOD("set_magnitudes", "magnitudes"), &RealtimeSpectrumVisualizer::set_magnitudes);
		ClassDB::bind_method(D_METHOD("get_magnitudes"), &RealtimeSpectrumVisualizer::get_magnitudes);
		ClassDB::bind_method(D_METHOD("set_magnitudes_are_db", "enabled"), &RealtimeSpectrumVisualizer::set_magnitudes_are_db);
		ClassDB::bind_method(D_METHOD("get_magnitudes_are_db"), &RealtimeSpectrumVisualizer::get_magnitudes_are_db);
		ClassDB::bind_method(D_METHOD("set_bar_count", "count"), &RealtimeSpectrumVisualizer::set_bar_count);
		ClassDB::bind_method(D_METHOD("get_bar_count"), &RealtimeSpectrumVisualizer::get_bar_count);
		ClassDB::bind_method(D_METHOD("set_min_frequency", "frequency"), &RealtimeSpectrumVisualizer::set_min_frequency);
		ClassDB::bind_method(D_METHOD("get_min_frequency"), &RealtimeSpectrumVisualizer::get_min_frequency);
		ClassDB::bind_method(D_METHOD("set_max_frequency", "frequency"), &RealtimeSpectrumVisualizer::set_max_frequency);
		ClassDB::bind_method(D_METHOD("get_max_frequency"), &RealtimeSpectrumVisualizer::get_max_frequency);
		ClassDB::bind_method(D_METHOD("set_min_db", "db"), &RealtimeSpectrumVisualizer::set_min_db);
		ClassDB::bind_method(D_METHOD("get_min_db"), &RealtimeSpectrumVisualizer::get_min_db);
		ClassDB::bind_method(D_METHOD("set_max_db", "db"), &RealtimeSpectrumVisualizer::set_max_db);
		ClassDB::bind_method(D_METHOD("get_max_db"), &RealtimeSpectrumVisualizer::get_max_db);
		ClassDB::bind_method(D_METHOD("set_use_log_frequency", "enabled"), &RealtimeSpectrumVisualizer::set_use_log_frequency);
		ClassDB::bind_method(D_METHOD("get_use_log_frequency"), &RealtimeSpectrumVisualizer::get_use_log_frequency);
		ClassDB::bind_method(D_METHOD("set_slope_enabled", "enabled"), &RealtimeSpectrumVisualizer::set_slope_enabled);
		ClassDB::bind_method(D_METHOD("get_slope_enabled"), &RealtimeSpectrumVisualizer::get_slope_enabled);
		ClassDB::bind_method(D_METHOD("set_slope_db_per_octave", "db"), &RealtimeSpectrumVisualizer::set_slope_db_per_octave);
		ClassDB::bind_method(D_METHOD("get_slope_db_per_octave"), &RealtimeSpectrumVisualizer::get_slope_db_per_octave);
		ClassDB::bind_method(D_METHOD("set_slope_pivot_frequency", "frequency"), &RealtimeSpectrumVisualizer::set_slope_pivot_frequency);
		ClassDB::bind_method(D_METHOD("get_slope_pivot_frequency"), &RealtimeSpectrumVisualizer::get_slope_pivot_frequency);
		ClassDB::bind_method(D_METHOD("set_attack", "attack"), &RealtimeSpectrumVisualizer::set_attack);
		ClassDB::bind_method(D_METHOD("get_attack"), &RealtimeSpectrumVisualizer::get_attack);
		ClassDB::bind_method(D_METHOD("set_release", "release"), &RealtimeSpectrumVisualizer::set_release);
		ClassDB::bind_method(D_METHOD("get_release"), &RealtimeSpectrumVisualizer::get_release);
		ClassDB::bind_method(D_METHOD("set_peak_hold_enabled", "enabled"), &RealtimeSpectrumVisualizer::set_peak_hold_enabled);
		ClassDB::bind_method(D_METHOD("get_peak_hold_enabled"), &RealtimeSpectrumVisualizer::get_peak_hold_enabled);
		ClassDB::bind_method(D_METHOD("set_peak_hold_time", "time"), &RealtimeSpectrumVisualizer::set_peak_hold_time);
		ClassDB::bind_method(D_METHOD("get_peak_hold_time"), &RealtimeSpectrumVisualizer::get_peak_hold_time);
		ClassDB::bind_method(D_METHOD("set_peak_fall_db_per_second", "rate"), &RealtimeSpectrumVisualizer::set_peak_fall_db_per_second);
		ClassDB::bind_method(D_METHOD("get_peak_fall_db_per_second"), &RealtimeSpectrumVisualizer::get_peak_fall_db_per_second);
		ClassDB::bind_method(D_METHOD("set_draw_mode", "mode"), &RealtimeSpectrumVisualizer::set_draw_mode);
		ClassDB::bind_method(D_METHOD("get_draw_mode"), &RealtimeSpectrumVisualizer::get_draw_mode);
		ClassDB::bind_method(D_METHOD("set_draw_grid", "enabled"), &RealtimeSpectrumVisualizer::set_draw_grid);
		ClassDB::bind_method(D_METHOD("get_draw_grid"), &RealtimeSpectrumVisualizer::get_draw_grid);
		ClassDB::bind_method(D_METHOD("set_draw_peaks", "enabled"), &RealtimeSpectrumVisualizer::set_draw_peaks);
		ClassDB::bind_method(D_METHOD("get_draw_peaks"), &RealtimeSpectrumVisualizer::get_draw_peaks);
		ClassDB::bind_method(D_METHOD("set_bar_gap", "gap"), &RealtimeSpectrumVisualizer::set_bar_gap);
		ClassDB::bind_method(D_METHOD("get_bar_gap"), &RealtimeSpectrumVisualizer::get_bar_gap);
		ClassDB::bind_method(D_METHOD("set_line_width", "width"), &RealtimeSpectrumVisualizer::set_line_width);
		ClassDB::bind_method(D_METHOD("get_line_width"), &RealtimeSpectrumVisualizer::get_line_width);
		ClassDB::bind_method(D_METHOD("set_padding", "padding"), &RealtimeSpectrumVisualizer::set_padding);
		ClassDB::bind_method(D_METHOD("get_padding"), &RealtimeSpectrumVisualizer::get_padding);
		ClassDB::bind_method(D_METHOD("set_background_color", "color"), &RealtimeSpectrumVisualizer::set_background_color);
		ClassDB::bind_method(D_METHOD("get_background_color"), &RealtimeSpectrumVisualizer::get_background_color);
		ClassDB::bind_method(D_METHOD("set_grid_color", "color"), &RealtimeSpectrumVisualizer::set_grid_color);
		ClassDB::bind_method(D_METHOD("get_grid_color"), &RealtimeSpectrumVisualizer::get_grid_color);
		ClassDB::bind_method(D_METHOD("set_low_color", "color"), &RealtimeSpectrumVisualizer::set_low_color);
		ClassDB::bind_method(D_METHOD("get_low_color"), &RealtimeSpectrumVisualizer::get_low_color);
		ClassDB::bind_method(D_METHOD("set_mid_color", "color"), &RealtimeSpectrumVisualizer::set_mid_color);
		ClassDB::bind_method(D_METHOD("get_mid_color"), &RealtimeSpectrumVisualizer::get_mid_color);
		ClassDB::bind_method(D_METHOD("set_high_color", "color"), &RealtimeSpectrumVisualizer::set_high_color);
		ClassDB::bind_method(D_METHOD("get_high_color"), &RealtimeSpectrumVisualizer::get_high_color);
		ClassDB::bind_method(D_METHOD("set_line_color", "color"), &RealtimeSpectrumVisualizer::set_line_color);
		ClassDB::bind_method(D_METHOD("get_line_color"), &RealtimeSpectrumVisualizer::get_line_color);
		ClassDB::bind_method(D_METHOD("set_fill_color", "color"), &RealtimeSpectrumVisualizer::set_fill_color);
		ClassDB::bind_method(D_METHOD("get_fill_color"), &RealtimeSpectrumVisualizer::get_fill_color);
		ClassDB::bind_method(D_METHOD("set_peak_color", "color"), &RealtimeSpectrumVisualizer::set_peak_color);
		ClassDB::bind_method(D_METHOD("get_peak_color"), &RealtimeSpectrumVisualizer::get_peak_color);

		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_bus"), "set_use_bus", "get_use_bus");
		ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "bus", PROPERTY_HINT_ENUM_SUGGESTION, "Master"), "set_bus", "get_bus");
		ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "magnitudes"), "set_magnitudes", "get_magnitudes");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "magnitudes_are_db"), "set_magnitudes_are_db", "get_magnitudes_are_db");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "bar_count", PROPERTY_HINT_RANGE, "8,512,1"), "set_bar_count", "get_bar_count");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_frequency", PROPERTY_HINT_RANGE, "1.0,20000.0,1.0"), "set_min_frequency", "get_min_frequency");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_frequency", PROPERTY_HINT_RANGE, "20.0,48000.0,1.0"), "set_max_frequency", "get_max_frequency");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_db", PROPERTY_HINT_RANGE, "-160.0,0.0,1.0"), "set_min_db", "get_min_db");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_db", PROPERTY_HINT_RANGE, "-80.0,24.0,1.0"), "set_max_db", "get_max_db");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_log_frequency"), "set_use_log_frequency", "get_use_log_frequency");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "slope_enabled"), "set_slope_enabled", "get_slope_enabled");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "slope_db_per_octave", PROPERTY_HINT_RANGE, "-12.0,12.0,0.1"), "set_slope_db_per_octave", "get_slope_db_per_octave");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "slope_pivot_frequency", PROPERTY_HINT_RANGE, "20.0,20000.0,1.0"), "set_slope_pivot_frequency", "get_slope_pivot_frequency");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "attack", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_attack", "get_attack");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "release", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_release", "get_release");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "peak_hold_enabled"), "set_peak_hold_enabled", "get_peak_hold_enabled");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "peak_hold_time", PROPERTY_HINT_RANGE, "0.0,10.0,0.1"), "set_peak_hold_time", "get_peak_hold_time");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "peak_fall_db_per_second", PROPERTY_HINT_RANGE, "0.0,120.0,1.0"), "set_peak_fall_db_per_second", "get_peak_fall_db_per_second");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "draw_mode", PROPERTY_HINT_ENUM, "Bars,Line,Filled Line"), "set_draw_mode", "get_draw_mode");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "draw_grid"), "set_draw_grid", "get_draw_grid");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "draw_peaks"), "set_draw_peaks", "get_draw_peaks");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bar_gap", PROPERTY_HINT_RANGE, "0.0,12.0,0.5"), "set_bar_gap", "get_bar_gap");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "line_width", PROPERTY_HINT_RANGE, "1.0,16.0,0.5"), "set_line_width", "get_line_width");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "padding", PROPERTY_HINT_RANGE, "0.0,64.0,0.5"), "set_padding", "get_padding");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "background_color"), "set_background_color", "get_background_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "grid_color"), "set_grid_color", "get_grid_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "low_color"), "set_low_color", "get_low_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "mid_color"), "set_mid_color", "get_mid_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "high_color"), "set_high_color", "get_high_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "line_color"), "set_line_color", "get_line_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "fill_color"), "set_fill_color", "get_fill_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "peak_color"), "set_peak_color", "get_peak_color");
	}

	RealtimeSpectrumVisualizer::RealtimeSpectrumVisualizer() {
		set_clip_contents(true);
		set_process(true);
		ensure_cache();
	}

	RealtimeSpectrumVisualizer::~RealtimeSpectrumVisualizer() {
	}

	void RealtimeSpectrumVisualizer::_notification(int p_what) {
		if (p_what == NOTIFICATION_PROCESS) {
			update_spectrum((float)get_process_delta_time());
			queue_redraw();
			return;
		}

		if (p_what != NOTIFICATION_DRAW) {
			return;
		}

		const Vector2 size = get_size();
		if (size.x <= 0.0f || size.y <= 0.0f) {
			return;
		}

		draw_rect(Rect2(Vector2(0.0f, 0.0f), size), background_color);

		const float pad = std::max(0.0f, padding);
		Rect2 canvas(Vector2(pad, pad), Vector2(std::max(1.0f, size.x - pad * 2.0f), std::max(1.0f, size.y - pad * 2.0f)));

		if (draw_grid) {
			draw_grid_lines(canvas);
		}

		if (draw_mode == DRAW_MODE_LINE || draw_mode == DRAW_MODE_FILLED_LINE) {
			draw_curve(canvas, draw_mode == DRAW_MODE_FILLED_LINE);
		}
		else {
			draw_bars(canvas);
		}

		if (draw_peaks && peak_hold_enabled) {
			draw_peak_marks(canvas);
		}
	}

	void RealtimeSpectrumVisualizer::_validate_property(PropertyInfo& p_property) const {
		const String name = p_property.name;
		if (name == StringName("bus")) {
			String bus_list;
			AudioServer* audio_server = AudioServer::get_singleton();

			if (audio_server) {
				int bus_count = audio_server->get_bus_count();
				for (int i = 0; i < bus_count; i++) {
					if (i > 0) bus_list += ",";
					bus_list += audio_server->get_bus_name(i);
				}
			}

			p_property.hint = PROPERTY_HINT_ENUM;
			p_property.hint_string = bus_list;
		}
	}

	void RealtimeSpectrumVisualizer::ensure_cache() {
		const int count = std::max(1, bar_count);
		if ((int)current_db.size() == count) {
			return;
		}

		current_db.assign(count, min_db);
		target_db.assign(count, min_db);
		peak_db.assign(count, min_db);
		peak_age.assign(count, 0.0f);
	}

	void RealtimeSpectrumVisualizer::update_spectrum(float p_delta) {
		ensure_cache();

		bool found_analyzer = false;
		if (use_bus) {
			read_bus_spectrum(found_analyzer);
		}

		if (!use_bus || !found_analyzer) {
			read_manual_spectrum();
		}

		const float delta = std::max(0.0f, p_delta);
		for (int i = 0; i < bar_count; i++) {
			const float target = target_db[(size_t)i];
			const float weight = target > current_db[(size_t)i] ? attack : release;
			current_db[(size_t)i] = mix_float(current_db[(size_t)i], target, AudioVisualizerBusUtils::clamp_float(weight, 0.0f, 1.0f));

			if (!peak_hold_enabled) {
				peak_db[(size_t)i] = current_db[(size_t)i];
				peak_age[(size_t)i] = 0.0f;
				continue;
			}

			if (current_db[(size_t)i] >= peak_db[(size_t)i]) {
				peak_db[(size_t)i] = current_db[(size_t)i];
				peak_age[(size_t)i] = 0.0f;
			}
			else {
				peak_age[(size_t)i] += delta;
				if (peak_age[(size_t)i] > peak_hold_time) {
					peak_db[(size_t)i] = std::max(min_db, peak_db[(size_t)i] - peak_fall_db_per_second * delta);
				}
			}
		}
	}

	void RealtimeSpectrumVisualizer::read_bus_spectrum(bool& r_found_analyzer) {
		r_found_analyzer = false;

		Ref<AudioEffectSpectrumAnalyzerInstance> analyzer_instance;
		if (!AudioVisualizerBusUtils::get_spectrum_analyzer_instance(bus, analyzer_instance)) {
			return;
		}

		AudioServer* audio_server = AudioServer::get_singleton();
		const float nyquist = audio_server ? std::max(20.0f, audio_server->get_mix_rate() * 0.5f) : max_frequency;
		const float max_freq = std::min(std::max(min_frequency + 1.0f, max_frequency), nyquist);

		for (int i = 0; i < bar_count; i++) {
			const float start_pos = (float)i / (float)bar_count;
			const float end_pos = (float)(i + 1) / (float)bar_count;
			const float from_hz = band_frequency(start_pos);
			const float to_hz = std::max(from_hz + 1.0f, std::min(max_freq, band_frequency(end_pos)));
			const Vector2 magnitude = analyzer_instance->get_magnitude_for_frequency_range(from_hz, to_hz, AudioEffectSpectrumAnalyzerInstance::MAGNITUDE_MAX);
			float db = AudioVisualizerBusUtils::linear_to_db(std::max(std::max(magnitude.x, magnitude.y), MIN_LINEAR));

			if (slope_enabled) {
				const float center_hz = std::sqrt(std::max(1.0f, from_hz) * std::max(1.0f, to_hz));
				db += std::log2(std::max(1.0f, center_hz) / std::max(1.0f, slope_pivot_frequency)) * slope_db_per_octave;
			}

			target_db[(size_t)i] = AudioVisualizerBusUtils::clamp_float(db, min_db, max_db);
		}

		r_found_analyzer = true;
	}

	void RealtimeSpectrumVisualizer::read_manual_spectrum() {
		const int source_count = magnitudes.size();
		if (source_count <= 0) {
			std::fill(target_db.begin(), target_db.end(), min_db);
			return;
		}

		const float* source = magnitudes.ptr();
		for (int i = 0; i < bar_count; i++) {
			const float position = bar_count > 1 ? (float)i / (float)(bar_count - 1) : 0.0f;
			const int source_index = std::min(source_count - 1, std::max(0, (int)std::lround(position * (float)(source_count - 1))));
			float value = source[source_index];
			if (!std::isfinite(value)) {
				value = magnitudes_are_db ? min_db : 0.0f;
			}

			const float db = magnitudes_are_db ? value : AudioVisualizerBusUtils::linear_to_db(std::max(MIN_LINEAR, std::abs(value)));
			target_db[(size_t)i] = AudioVisualizerBusUtils::clamp_float(db, min_db, max_db);
		}
	}

	float RealtimeSpectrumVisualizer::band_frequency(float p_position) const {
		const float t = AudioVisualizerBusUtils::clamp_float(p_position, 0.0f, 1.0f);
		const float min_freq = std::max(1.0f, min_frequency);
		const float max_freq = std::max(min_freq + 1.0f, max_frequency);

		if (use_log_frequency) {
			return std::pow(10.0f, std::log10(min_freq) + t * (std::log10(max_freq) - std::log10(min_freq)));
		}

		return min_freq + (max_freq - min_freq) * t;
	}

	float RealtimeSpectrumVisualizer::normalized_to_y(float p_normalized, float p_top, float p_height) const {
		return p_top + (1.0f - AudioVisualizerBusUtils::clamp_float(p_normalized, 0.0f, 1.0f)) * p_height;
	}

	Color RealtimeSpectrumVisualizer::get_bar_color(float p_normalized) const {
		if (p_normalized < 0.5f) {
			return mix_color(low_color, mid_color, p_normalized * 2.0f);
		}

		return mix_color(mid_color, high_color, (p_normalized - 0.5f) * 2.0f);
	}

	void RealtimeSpectrumVisualizer::draw_grid_lines(const Rect2& p_rect) {
		const int db_step_count = 4;
		for (int i = 0; i <= db_step_count; i++) {
			const float y = p_rect.position.y + p_rect.size.y * (float)i / (float)db_step_count;
			draw_line(Vector2(p_rect.position.x, y), Vector2(p_rect.position.x + p_rect.size.x, y), grid_color, 1.0f);
		}

		const int freq_step_count = 6;
		for (int i = 0; i <= freq_step_count; i++) {
			const float x = p_rect.position.x + p_rect.size.x * (float)i / (float)freq_step_count;
			draw_line(Vector2(x, p_rect.position.y), Vector2(x, p_rect.position.y + p_rect.size.y), grid_color, 1.0f);
		}
	}

	void RealtimeSpectrumVisualizer::draw_bars(const Rect2& p_rect) {
		if (bar_count <= 0) {
			return;
		}

		const float full_step = p_rect.size.x / (float)bar_count;
		const float gap = std::min(std::max(0.0f, bar_gap), std::max(0.0f, full_step - 1.0f));
		const float width = std::max(1.0f, full_step - gap);
		const float bottom = p_rect.position.y + p_rect.size.y;

		for (int i = 0; i < bar_count; i++) {
			const float normalized = AudioVisualizerBusUtils::db_to_normalized(current_db[(size_t)i], min_db, max_db);
			const float x = p_rect.position.x + (float)i * full_step + gap * 0.5f;
			const float y = normalized_to_y(normalized, p_rect.position.y, p_rect.size.y);
			draw_rect(Rect2(Vector2(x, y), Vector2(width, std::max(1.0f, bottom - y))), get_bar_color(normalized));
		}
	}

	void RealtimeSpectrumVisualizer::draw_curve(const Rect2& p_rect, bool p_filled) {
		if (bar_count <= 1) {
			return;
		}

		PackedVector2Array points;
		points.resize(bar_count);
		for (int i = 0; i < bar_count; i++) {
			const float x = p_rect.position.x + ((float)i / (float)(bar_count - 1)) * p_rect.size.x;
			const float normalized = AudioVisualizerBusUtils::db_to_normalized(current_db[(size_t)i], min_db, max_db);
			points.set(i, Vector2(x, normalized_to_y(normalized, p_rect.position.y, p_rect.size.y)));
		}

		if (p_filled) {
			PackedVector2Array polygon;
			polygon.resize(bar_count + 2);
			polygon.set(0, Vector2(p_rect.position.x, p_rect.position.y + p_rect.size.y));
			for (int i = 0; i < bar_count; i++) {
				polygon.set(i + 1, points[i]);
			}
			polygon.set(bar_count + 1, Vector2(p_rect.position.x + p_rect.size.x, p_rect.position.y + p_rect.size.y));
			draw_colored_polygon(polygon, fill_color);
		}

		draw_polyline(points, line_color, line_width, true);
	}

	void RealtimeSpectrumVisualizer::draw_peak_marks(const Rect2& p_rect) {
		if (bar_count <= 0) {
			return;
		}

		const float full_step = p_rect.size.x / (float)bar_count;
		const float mark_width = draw_mode == DRAW_MODE_BARS ? std::max(1.0f, full_step - std::max(0.0f, bar_gap)) : 3.0f;

		for (int i = 0; i < bar_count; i++) {
			const float normalized = AudioVisualizerBusUtils::db_to_normalized(peak_db[(size_t)i], min_db, max_db);
			const float x = p_rect.position.x + (float)i * full_step + (full_step - mark_width) * 0.5f;
			const float y = normalized_to_y(normalized, p_rect.position.y, p_rect.size.y);
			draw_line(Vector2(x, y), Vector2(x + mark_width, y), peak_color, 1.0f);
		}
	}

	void RealtimeSpectrumVisualizer::set_use_bus(bool p_enabled) {
		if (use_bus == p_enabled) {
			return;
		}

		use_bus = p_enabled;
		queue_redraw();
	}

	bool RealtimeSpectrumVisualizer::get_use_bus() const {
		return use_bus;
	}

	void RealtimeSpectrumVisualizer::set_bus(const StringName& p_bus) {
		if (bus == p_bus) {
			return;
		}

		bus = p_bus;
		queue_redraw();
	}

	StringName RealtimeSpectrumVisualizer::get_bus() const {
		return bus;
	}

	void RealtimeSpectrumVisualizer::set_magnitudes(const PackedFloat32Array& p_magnitudes) {
		magnitudes = p_magnitudes;
		update_spectrum(0.0f);
		queue_redraw();
	}

	PackedFloat32Array RealtimeSpectrumVisualizer::get_magnitudes() const {
		return magnitudes;
	}

	void RealtimeSpectrumVisualizer::set_magnitudes_are_db(bool p_enabled) {
		magnitudes_are_db = p_enabled;
		queue_redraw();
	}

	bool RealtimeSpectrumVisualizer::get_magnitudes_are_db() const {
		return magnitudes_are_db;
	}

	void RealtimeSpectrumVisualizer::set_bar_count(int p_count) {
		bar_count = std::max(1, p_count);
		ensure_cache();
		queue_redraw();
	}

	int RealtimeSpectrumVisualizer::get_bar_count() const {
		return bar_count;
	}

	void RealtimeSpectrumVisualizer::set_min_frequency(float p_frequency) {
		min_frequency = std::max(1.0f, std::min(p_frequency, max_frequency - 1.0f));
		queue_redraw();
	}

	float RealtimeSpectrumVisualizer::get_min_frequency() const {
		return min_frequency;
	}

	void RealtimeSpectrumVisualizer::set_max_frequency(float p_frequency) {
		max_frequency = std::max(p_frequency, min_frequency + 1.0f);
		queue_redraw();
	}

	float RealtimeSpectrumVisualizer::get_max_frequency() const {
		return max_frequency;
	}

	void RealtimeSpectrumVisualizer::set_min_db(float p_db) {
		min_db = std::min(p_db, max_db - 1.0f);
		queue_redraw();
	}

	float RealtimeSpectrumVisualizer::get_min_db() const {
		return min_db;
	}

	void RealtimeSpectrumVisualizer::set_max_db(float p_db) {
		max_db = std::max(p_db, min_db + 1.0f);
		queue_redraw();
	}

	float RealtimeSpectrumVisualizer::get_max_db() const {
		return max_db;
	}

	void RealtimeSpectrumVisualizer::set_use_log_frequency(bool p_enabled) {
		use_log_frequency = p_enabled;
		queue_redraw();
	}

	bool RealtimeSpectrumVisualizer::get_use_log_frequency() const {
		return use_log_frequency;
	}

	void RealtimeSpectrumVisualizer::set_slope_enabled(bool p_enabled) {
		slope_enabled = p_enabled;
		queue_redraw();
	}

	bool RealtimeSpectrumVisualizer::get_slope_enabled() const {
		return slope_enabled;
	}

	void RealtimeSpectrumVisualizer::set_slope_db_per_octave(float p_db) {
		slope_db_per_octave = p_db;
		queue_redraw();
	}

	float RealtimeSpectrumVisualizer::get_slope_db_per_octave() const {
		return slope_db_per_octave;
	}

	void RealtimeSpectrumVisualizer::set_slope_pivot_frequency(float p_frequency) {
		slope_pivot_frequency = std::max(1.0f, p_frequency);
		queue_redraw();
	}

	float RealtimeSpectrumVisualizer::get_slope_pivot_frequency() const {
		return slope_pivot_frequency;
	}

	void RealtimeSpectrumVisualizer::set_attack(float p_attack) {
		attack = AudioVisualizerBusUtils::clamp_float(p_attack, 0.0f, 1.0f);
	}

	float RealtimeSpectrumVisualizer::get_attack() const {
		return attack;
	}

	void RealtimeSpectrumVisualizer::set_release(float p_release) {
		release = AudioVisualizerBusUtils::clamp_float(p_release, 0.0f, 1.0f);
	}

	float RealtimeSpectrumVisualizer::get_release() const {
		return release;
	}

	void RealtimeSpectrumVisualizer::set_peak_hold_enabled(bool p_enabled) {
		peak_hold_enabled = p_enabled;
		queue_redraw();
	}

	bool RealtimeSpectrumVisualizer::get_peak_hold_enabled() const {
		return peak_hold_enabled;
	}

	void RealtimeSpectrumVisualizer::set_peak_hold_time(float p_time) {
		peak_hold_time = std::max(0.0f, p_time);
	}

	float RealtimeSpectrumVisualizer::get_peak_hold_time() const {
		return peak_hold_time;
	}

	void RealtimeSpectrumVisualizer::set_peak_fall_db_per_second(float p_rate) {
		peak_fall_db_per_second = std::max(0.0f, p_rate);
	}

	float RealtimeSpectrumVisualizer::get_peak_fall_db_per_second() const {
		return peak_fall_db_per_second;
	}

	void RealtimeSpectrumVisualizer::set_draw_mode(int p_mode) {
		draw_mode = std::max(0, std::min(2, p_mode));
		queue_redraw();
	}

	int RealtimeSpectrumVisualizer::get_draw_mode() const {
		return draw_mode;
	}

	void RealtimeSpectrumVisualizer::set_draw_grid(bool p_enabled) {
		draw_grid = p_enabled;
		queue_redraw();
	}

	bool RealtimeSpectrumVisualizer::get_draw_grid() const {
		return draw_grid;
	}

	void RealtimeSpectrumVisualizer::set_draw_peaks(bool p_enabled) {
		draw_peaks = p_enabled;
		queue_redraw();
	}

	bool RealtimeSpectrumVisualizer::get_draw_peaks() const {
		return draw_peaks;
	}

	void RealtimeSpectrumVisualizer::set_bar_gap(float p_gap) {
		bar_gap = std::max(0.0f, p_gap);
		queue_redraw();
	}

	float RealtimeSpectrumVisualizer::get_bar_gap() const {
		return bar_gap;
	}

	void RealtimeSpectrumVisualizer::set_line_width(float p_width) {
		line_width = std::max(1.0f, p_width);
		queue_redraw();
	}

	float RealtimeSpectrumVisualizer::get_line_width() const {
		return line_width;
	}

	void RealtimeSpectrumVisualizer::set_padding(float p_padding) {
		padding = std::max(0.0f, p_padding);
		queue_redraw();
	}

	float RealtimeSpectrumVisualizer::get_padding() const {
		return padding;
	}

	void RealtimeSpectrumVisualizer::set_background_color(const Color& p_color) {
		background_color = p_color;
		queue_redraw();
	}

	Color RealtimeSpectrumVisualizer::get_background_color() const {
		return background_color;
	}

	void RealtimeSpectrumVisualizer::set_grid_color(const Color& p_color) {
		grid_color = p_color;
		queue_redraw();
	}

	Color RealtimeSpectrumVisualizer::get_grid_color() const {
		return grid_color;
	}

	void RealtimeSpectrumVisualizer::set_low_color(const Color& p_color) {
		low_color = p_color;
		queue_redraw();
	}

	Color RealtimeSpectrumVisualizer::get_low_color() const {
		return low_color;
	}

	void RealtimeSpectrumVisualizer::set_mid_color(const Color& p_color) {
		mid_color = p_color;
		queue_redraw();
	}

	Color RealtimeSpectrumVisualizer::get_mid_color() const {
		return mid_color;
	}

	void RealtimeSpectrumVisualizer::set_high_color(const Color& p_color) {
		high_color = p_color;
		queue_redraw();
	}

	Color RealtimeSpectrumVisualizer::get_high_color() const {
		return high_color;
	}

	void RealtimeSpectrumVisualizer::set_line_color(const Color& p_color) {
		line_color = p_color;
		queue_redraw();
	}

	Color RealtimeSpectrumVisualizer::get_line_color() const {
		return line_color;
	}

	void RealtimeSpectrumVisualizer::set_fill_color(const Color& p_color) {
		fill_color = p_color;
		queue_redraw();
	}

	Color RealtimeSpectrumVisualizer::get_fill_color() const {
		return fill_color;
	}

	void RealtimeSpectrumVisualizer::set_peak_color(const Color& p_color) {
		peak_color = p_color;
		queue_redraw();
	}

	Color RealtimeSpectrumVisualizer::get_peak_color() const {
		return peak_color;
	}
}
