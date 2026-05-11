#ifndef REALTIME_SPECTRUM_VISUALIZER_H
#define REALTIME_SPECTRUM_VISUALIZER_H

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include <vector>

namespace godot {
	class RealtimeSpectrumVisualizer : public Control {
		GDCLASS(RealtimeSpectrumVisualizer, Control)

	public:
		enum DrawMode {
			DRAW_MODE_BARS = 0,
			DRAW_MODE_LINE = 1,
			DRAW_MODE_FILLED_LINE = 2
		};

	private:
		bool use_bus = true;
		StringName bus = StringName("Master");
		PackedFloat32Array magnitudes;
		bool magnitudes_are_db = false;

		int bar_count = 96;
		float min_frequency = 20.0f;
		float max_frequency = 20000.0f;
		float min_db = -80.0f;
		float max_db = 0.0f;
		bool use_log_frequency = true;
		bool slope_enabled = true;
		float slope_db_per_octave = 4.5f;
		float slope_pivot_frequency = 1000.0f;

		float attack = 0.55f;
		float release = 0.12f;
		bool peak_hold_enabled = true;
		float peak_hold_time = 1.5f;
		float peak_fall_db_per_second = 18.0f;

		int draw_mode = DRAW_MODE_BARS;
		bool draw_grid = true;
		bool draw_peaks = true;
		float bar_gap = 1.0f;
		float line_width = 2.0f;
		float padding = 6.0f;

		Color background_color = Color(0.065f, 0.055f, 0.09f, 1.0f);
		Color grid_color = Color(0.95f, 0.65f, 0.95f, 0.22f);
		Color low_color = Color(0.15f, 0.95f, 0.75f, 1.0f);
		Color mid_color = Color(1.0f, 0.82f, 0.22f, 1.0f);
		Color high_color = Color(1.0f, 0.23f, 0.52f, 1.0f);
		Color line_color = Color(1.0f, 0.72f, 0.95f, 1.0f);
		Color fill_color = Color(1.0f, 0.24f, 0.65f, 0.22f);
		Color peak_color = Color(1.0f, 1.0f, 1.0f, 0.85f);

		std::vector<float> current_db;
		std::vector<float> target_db;
		std::vector<float> peak_db;
		std::vector<float> peak_age;

		void ensure_cache();
		void update_spectrum(float p_delta);
		void read_bus_spectrum(bool& r_found_analyzer);
		void read_manual_spectrum();
		float band_frequency(float p_position) const;
		float normalized_to_y(float p_normalized, float p_top, float p_height) const;
		Color get_bar_color(float p_normalized) const;
		void draw_grid_lines(const Rect2& p_rect);
		void draw_bars(const Rect2& p_rect);
		void draw_curve(const Rect2& p_rect, bool p_filled);
		void draw_peak_marks(const Rect2& p_rect);

	protected:
		static void _bind_methods();
		void _notification(int p_what);
		void _validate_property(PropertyInfo& p_property) const;

	public:
		RealtimeSpectrumVisualizer();
		~RealtimeSpectrumVisualizer();

		void set_use_bus(bool p_enabled);
		bool get_use_bus() const;

		void set_bus(const StringName& p_bus);
		StringName get_bus() const;

		void set_magnitudes(const PackedFloat32Array& p_magnitudes);
		PackedFloat32Array get_magnitudes() const;

		void set_magnitudes_are_db(bool p_enabled);
		bool get_magnitudes_are_db() const;

		void set_bar_count(int p_count);
		int get_bar_count() const;

		void set_min_frequency(float p_frequency);
		float get_min_frequency() const;

		void set_max_frequency(float p_frequency);
		float get_max_frequency() const;

		void set_min_db(float p_db);
		float get_min_db() const;

		void set_max_db(float p_db);
		float get_max_db() const;

		void set_use_log_frequency(bool p_enabled);
		bool get_use_log_frequency() const;

		void set_slope_enabled(bool p_enabled);
		bool get_slope_enabled() const;

		void set_slope_db_per_octave(float p_db);
		float get_slope_db_per_octave() const;

		void set_slope_pivot_frequency(float p_frequency);
		float get_slope_pivot_frequency() const;

		void set_attack(float p_attack);
		float get_attack() const;

		void set_release(float p_release);
		float get_release() const;

		void set_peak_hold_enabled(bool p_enabled);
		bool get_peak_hold_enabled() const;

		void set_peak_hold_time(float p_time);
		float get_peak_hold_time() const;

		void set_peak_fall_db_per_second(float p_rate);
		float get_peak_fall_db_per_second() const;

		void set_draw_mode(int p_mode);
		int get_draw_mode() const;

		void set_draw_grid(bool p_enabled);
		bool get_draw_grid() const;

		void set_draw_peaks(bool p_enabled);
		bool get_draw_peaks() const;

		void set_bar_gap(float p_gap);
		float get_bar_gap() const;

		void set_line_width(float p_width);
		float get_line_width() const;

		void set_padding(float p_padding);
		float get_padding() const;

		void set_background_color(const Color& p_color);
		Color get_background_color() const;

		void set_grid_color(const Color& p_color);
		Color get_grid_color() const;

		void set_low_color(const Color& p_color);
		Color get_low_color() const;

		void set_mid_color(const Color& p_color);
		Color get_mid_color() const;

		void set_high_color(const Color& p_color);
		Color get_high_color() const;

		void set_line_color(const Color& p_color);
		Color get_line_color() const;

		void set_fill_color(const Color& p_color);
		Color get_fill_color() const;

		void set_peak_color(const Color& p_color);
		Color get_peak_color() const;
	};
}

VARIANT_ENUM_CAST(RealtimeSpectrumVisualizer::DrawMode);

#endif // REALTIME_SPECTRUM_VISUALIZER_H
