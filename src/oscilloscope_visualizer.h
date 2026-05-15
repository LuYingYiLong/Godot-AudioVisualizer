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

		enum XYRenderMode {
			XY_RENDER_MODE_POINTS = 0,
			XY_RENDER_MODE_LINE = 1,
			XY_RENDER_MODE_PHOSPHOR = 2
		};

		enum XYOrientation {
			XY_ORIENTATION_AUTO = 0,
			XY_ORIENTATION_LEFT_RIGHT = 1,
			XY_ORIENTATION_MID_SIDE = 2,
			XY_ORIENTATION_ROTATE_NEGATIVE_45 = 3,
			XY_ORIENTATION_INVERT_Y = 4
		};

	private:
		bool use_bus = true;
		StringName bus = StringName("Master");
		int bus_backend = 0;
		PackedVector2Array samples;

		int sample_count = 1024;
		int display_mode = DISPLAY_MODE_WAVEFORM;
		int xy_render_mode = XY_RENDER_MODE_POINTS;
		int xy_orientation = XY_ORIENTATION_AUTO;
		bool frozen = false;
		bool draw_grid = true;
		bool draw_center = true;
		float gain = 1.0f;
		float line_width = 2.0f;
		float point_size = 2.0f;
		float xy_line_width = 2.0f;
		float xy_glow_width = 8.0f;
		float xy_persistence = 0.65f;
		float padding = 8.0f;
		float stereo_gap = 4.0f;

		Color background_color = Color("#000000");
		Color grid_color = Color("#f2f2f238");
		Color center_color = Color("#ffffff80");
		Color left_color = Color("#ffffff");
		Color right_color = Color("#ffffff");
		Color xy_color = Color("#ffffffe6");
		Color xy_glow_color = Color("#80ffe680");

		std::vector<Vector2> live_samples;

		void update_samples();
		void append_samples(const PackedVector2Array& p_frames);
		void decay_live_samples();
		const std::vector<Vector2>& get_display_samples() const;
		Rect2 get_canvas_rect() const;
		void draw_scope_grid(const Rect2& p_rect);
		void draw_waveform(const Rect2& p_rect);
		void draw_xy(const Rect2& p_rect, bool p_lissajous);
		Vector2 get_xy_point(const Vector2& p_sample, const Rect2& p_rect, bool p_lissajous) const;
		void draw_xy_segment_trail(const PackedVector2Array& p_points, const Color& p_color, float p_width, float p_persistence);

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

		void set_bus_backend(int p_backend);
		int get_bus_backend() const;

		void set_samples(const PackedVector2Array& p_samples);
		PackedVector2Array get_samples() const;

		void set_sample_count(int p_count);
		int get_sample_count() const;

		void set_display_mode(int p_mode);
		int get_display_mode() const;

		void set_xy_render_mode(int p_mode);
		int get_xy_render_mode() const;

		void set_xy_orientation(int p_orientation);
		int get_xy_orientation() const;

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

		void set_xy_line_width(float p_width);
		float get_xy_line_width() const;

		void set_xy_glow_width(float p_width);
		float get_xy_glow_width() const;

		void set_xy_persistence(float p_persistence);
		float get_xy_persistence() const;

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

		void set_xy_glow_color(const Color& p_color);
		Color get_xy_glow_color() const;
	};
}

VARIANT_ENUM_CAST(OscilloscopeVisualizer::DisplayMode);
VARIANT_ENUM_CAST(OscilloscopeVisualizer::XYRenderMode);
VARIANT_ENUM_CAST(OscilloscopeVisualizer::XYOrientation);

#endif // OSCILLOSCOPE_VISUALIZER_H
