#ifndef WAVE_VISUALIZER_H
#define WAVE_VISUALIZER_H

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include <cstdint>

namespace godot {
	class WaveVisualizer : public Control {
		GDCLASS(WaveVisualizer, Control)

	private:
		PackedFloat32Array audio_samples;

		Color bg_color = Color(0.1f, 0.1f, 0.1f, 1.0f);
		Color wave_color = Color(0.2f, 0.8f, 0.3f, 1.0f);
		Color accent_color = Color(0.18f, 0.95f, 1.0f, 1.0f);
		Color fill_color = Color(0.2f, 0.8f, 0.95f, 0.28f);
		Color glow_color = Color(0.4f, 0.85f, 1.0f, 0.22f);
		Color center_color = Color(0.3f, 0.3f, 0.3f, 0.5f);

		enum WaveDirection {
			DIRECTION_LEFT_TO_RIGHT = 0,
			DIRECTION_BOTTOM_TO_TOP = 1,
			DIRECTION_TOP_TO_BOTTOM = 2
		};

		enum RenderMode {
			RENDER_MODE_LINES = 0,
			RENDER_MODE_MIRROR_FILL = 1,
			RENDER_MODE_RIBBON = 2,
			RENDER_MODE_GLOW = 3,
			RENDER_MODE_DOTS = 4
		};

		int direction = DIRECTION_LEFT_TO_RIGHT;
		int render_mode = RENDER_MODE_LINES;
		Vector2i virtual_size = Vector2i(0, 0);
		Vector2i view_offset = Vector2i(0, 0);
		float line_width = 1.0f;
		float amplitude_scale = 1.0f;
		bool antialiased = true;
		bool draw_center_line = true;
		bool smoothing_enabled = true;
		int smoothing_radius = 2;
		float smoothing_strength = 0.65f;
		bool gradient_enabled = true;
		float fill_strength = 0.65f;
		float glow_width = 8.0f;
		float dot_size = 2.0f;
		float ribbon_fill_width = 1.0f;
		bool show_view_properties = true;
		bool show_line_properties = true;
		bool show_art_properties = true;
		bool show_color_properties = true;

		void _on_size_changed();
		void draw_waveform();
		void draw_horizontal_waveform();
		void draw_vertical_waveform(bool p_invert_axis);
		bool get_sample_range(int64_t p_start, int64_t p_end, float& r_min, float& r_max) const;
		bool get_smoothed_sample_range(int64_t p_virtual_index, int64_t p_virtual_span, double p_samples_per_unit, float& r_min, float& r_max) const;
		Color get_wave_color_for_level(float p_level) const;
		Color with_alpha(const Color& p_color, float p_alpha_multiplier) const;

	protected:
		static void _bind_methods();
		void _notification(int p_what);
		void _validate_property(PropertyInfo& p_property) const;

	public:
		WaveVisualizer();
		~WaveVisualizer();

		void set_audio_samples(const PackedFloat32Array& p_samples);
		PackedFloat32Array get_audio_samples() const;

		void set_wave_direction(int p_direction);
		int get_wave_direction() const;

		void set_render_mode(int p_mode);
		int get_render_mode() const;

		void set_background_color(const Color& p_color);
		Color get_background_color() const;

		void set_wave_color(const Color& p_color);
		Color get_wave_color() const;

		void set_accent_color(const Color& p_color);
		Color get_accent_color() const;

		void set_fill_color(const Color& p_color);
		Color get_fill_color() const;

		void set_glow_color(const Color& p_color);
		Color get_glow_color() const;

		void set_center_color(const Color& p_color);
		Color get_center_color() const;

		void set_virtual_size(const Vector2i& p_size);
		Vector2i get_virtual_size() const;

		void set_view_offset(const Vector2i& p_offset);
		Vector2i get_view_offset() const;

		void set_line_width(float p_width);
		float get_line_width() const;

		void set_amplitude_scale(float p_scale);
		float get_amplitude_scale() const;

		void set_antialiased(bool p_enabled);
		bool get_antialiased() const;

		void set_draw_center_line(bool p_enabled);
		bool get_draw_center_line() const;

		void set_smoothing_enabled(bool p_enabled);
		bool get_smoothing_enabled() const;

		void set_smoothing_radius(int p_radius);
		int get_smoothing_radius() const;

		void set_smoothing_strength(float p_strength);
		float get_smoothing_strength() const;

		void set_gradient_enabled(bool p_enabled);
		bool get_gradient_enabled() const;

		void set_fill_strength(float p_strength);
		float get_fill_strength() const;

		void set_glow_width(float p_width);
		float get_glow_width() const;

		void set_dot_size(float p_size);
		float get_dot_size() const;

		void set_ribbon_fill_width(float p_width);
		float get_ribbon_fill_width() const;

		void set_show_view_properties(bool p_enabled);
		bool get_show_view_properties() const;

		void set_show_line_properties(bool p_enabled);
		bool get_show_line_properties() const;

		void set_show_art_properties(bool p_enabled);
		bool get_show_art_properties() const;

		void set_show_color_properties(bool p_enabled);
		bool get_show_color_properties() const;

		void update_region(const Rect2i& p_region);

		void generate_waveform();
		void update_display();

		void clear();
		Error save_to_file(const String& p_path);
		Vector2i get_image_size() const;
	};
}

#endif // WAVE_VISUALIZER_H
