#ifndef OSCILLOSCOPE_VISUALIZER_H
#define OSCILLOSCOPE_VISUALIZER_H

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include <vector>

namespace godot {
	class OscilloscopeVisualizer : public Control {
		GDCLASS(OscilloscopeVisualizer, Control)

	public:
		enum DisplayMode {
			DISPLAY_MODE_WAVEFORM = 0,
			DISPLAY_MODE_XY = 1,
			DISPLAY_MODE_LISSAJOUS = 2
		};

	private:
		bool use_bus = true;
		StringName bus = StringName("Master");
		PackedVector2Array samples;

		int sample_count = 1024;
		int display_mode = DISPLAY_MODE_WAVEFORM;
		bool frozen = false;
		bool draw_grid = true;
		bool draw_center = true;
		float gain = 1.0f;
		float line_width = 2.0f;
		float point_size = 2.0f;
		float padding = 8.0f;
		float stereo_gap = 4.0f;

		Color background_color = Color(0.055f, 0.055f, 0.075f, 1.0f);
		Color grid_color = Color(0.95f, 0.65f, 0.95f, 0.22f);
		Color center_color = Color(1.0f, 0.75f, 0.95f, 0.5f);
		Color left_color = Color(1.0f, 0.38f, 0.72f, 1.0f);
		Color right_color = Color(0.35f, 0.85f, 1.0f, 1.0f);
		Color xy_color = Color(1.0f, 0.78f, 0.98f, 0.9f);

		std::vector<Vector2> live_samples;

		void update_samples();
		void append_samples(const PackedVector2Array& p_frames);
		const std::vector<Vector2>& get_display_samples() const;
		Rect2 get_canvas_rect() const;
		void draw_scope_grid(const Rect2& p_rect);
		void draw_waveform(const Rect2& p_rect);
		void draw_xy(const Rect2& p_rect, bool p_lissajous);

	protected:
		static void _bind_methods();
		void _notification(int p_what);
		void _validate_property(PropertyInfo& p_property) const;

	public:
		OscilloscopeVisualizer();
		~OscilloscopeVisualizer();

		void set_use_bus(bool p_enabled);
		bool get_use_bus() const;

		void set_bus(const StringName& p_bus);
		StringName get_bus() const;

		void set_samples(const PackedVector2Array& p_samples);
		PackedVector2Array get_samples() const;

		void set_sample_count(int p_count);
		int get_sample_count() const;

		void set_display_mode(int p_mode);
		int get_display_mode() const;

		void set_frozen(bool p_enabled);
		bool get_frozen() const;

		void set_draw_grid(bool p_enabled);
		bool get_draw_grid() const;

		void set_draw_center(bool p_enabled);
		bool get_draw_center() const;

		void set_gain(float p_gain);
		float get_gain() const;

		void set_line_width(float p_width);
		float get_line_width() const;

		void set_point_size(float p_size);
		float get_point_size() const;

		void set_padding(float p_padding);
		float get_padding() const;

		void set_stereo_gap(float p_gap);
		float get_stereo_gap() const;

		void set_background_color(const Color& p_color);
		Color get_background_color() const;

		void set_grid_color(const Color& p_color);
		Color get_grid_color() const;

		void set_center_color(const Color& p_color);
		Color get_center_color() const;

		void set_left_color(const Color& p_color);
		Color get_left_color() const;

		void set_right_color(const Color& p_color);
		Color get_right_color() const;

		void set_xy_color(const Color& p_color);
		Color get_xy_color() const;
	};
}

VARIANT_ENUM_CAST(OscilloscopeVisualizer::DisplayMode);

#endif // OSCILLOSCOPE_VISUALIZER_H
