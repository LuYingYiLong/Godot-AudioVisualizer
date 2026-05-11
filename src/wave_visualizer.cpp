#include "wave_visualizer.h"

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <algorithm>
#include <cmath>

namespace godot {
	namespace {
		inline float clamp_float(float p_value, float p_min, float p_max) {
			if (p_value < p_min) {
				return p_min;
			}

			if (p_value > p_max) {
				return p_max;
			}

			return p_value;
		}

		inline Color multiply_alpha(Color p_color, float p_multiplier) {
			p_color.a *= clamp_float(p_multiplier, 0.0f, 1.0f);
			return p_color;
		}
	}

	void WaveVisualizer::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_audio_samples", "samples"), &WaveVisualizer::set_audio_samples);
		ClassDB::bind_method(D_METHOD("get_audio_samples"), &WaveVisualizer::get_audio_samples);

		ClassDB::bind_method(D_METHOD("set_wave_direction", "direction"), &WaveVisualizer::set_wave_direction);
		ClassDB::bind_method(D_METHOD("get_wave_direction"), &WaveVisualizer::get_wave_direction);

		ClassDB::bind_method(D_METHOD("set_render_mode", "mode"), &WaveVisualizer::set_render_mode);
		ClassDB::bind_method(D_METHOD("get_render_mode"), &WaveVisualizer::get_render_mode);

		ClassDB::bind_method(D_METHOD("set_background_color", "color"), &WaveVisualizer::set_background_color);
		ClassDB::bind_method(D_METHOD("get_background_color"), &WaveVisualizer::get_background_color);

		ClassDB::bind_method(D_METHOD("set_wave_color", "color"), &WaveVisualizer::set_wave_color);
		ClassDB::bind_method(D_METHOD("get_wave_color"), &WaveVisualizer::get_wave_color);

		ClassDB::bind_method(D_METHOD("set_accent_color", "color"), &WaveVisualizer::set_accent_color);
		ClassDB::bind_method(D_METHOD("get_accent_color"), &WaveVisualizer::get_accent_color);

		ClassDB::bind_method(D_METHOD("set_fill_color", "color"), &WaveVisualizer::set_fill_color);
		ClassDB::bind_method(D_METHOD("get_fill_color"), &WaveVisualizer::get_fill_color);

		ClassDB::bind_method(D_METHOD("set_glow_color", "color"), &WaveVisualizer::set_glow_color);
		ClassDB::bind_method(D_METHOD("get_glow_color"), &WaveVisualizer::get_glow_color);

		ClassDB::bind_method(D_METHOD("set_center_color", "color"), &WaveVisualizer::set_center_color);
		ClassDB::bind_method(D_METHOD("get_center_color"), &WaveVisualizer::get_center_color);

		ClassDB::bind_method(D_METHOD("set_virtual_size", "size"), &WaveVisualizer::set_virtual_size);
		ClassDB::bind_method(D_METHOD("get_virtual_size"), &WaveVisualizer::get_virtual_size);

		ClassDB::bind_method(D_METHOD("set_view_offset", "offset"), &WaveVisualizer::set_view_offset);
		ClassDB::bind_method(D_METHOD("get_view_offset"), &WaveVisualizer::get_view_offset);

		ClassDB::bind_method(D_METHOD("set_line_width", "width"), &WaveVisualizer::set_line_width);
		ClassDB::bind_method(D_METHOD("get_line_width"), &WaveVisualizer::get_line_width);

		ClassDB::bind_method(D_METHOD("set_amplitude_scale", "scale"), &WaveVisualizer::set_amplitude_scale);
		ClassDB::bind_method(D_METHOD("get_amplitude_scale"), &WaveVisualizer::get_amplitude_scale);

		ClassDB::bind_method(D_METHOD("set_antialiased", "enabled"), &WaveVisualizer::set_antialiased);
		ClassDB::bind_method(D_METHOD("get_antialiased"), &WaveVisualizer::get_antialiased);

		ClassDB::bind_method(D_METHOD("set_draw_center_line", "enabled"), &WaveVisualizer::set_draw_center_line);
		ClassDB::bind_method(D_METHOD("get_draw_center_line"), &WaveVisualizer::get_draw_center_line);

		ClassDB::bind_method(D_METHOD("set_smoothing_enabled", "enabled"), &WaveVisualizer::set_smoothing_enabled);
		ClassDB::bind_method(D_METHOD("get_smoothing_enabled"), &WaveVisualizer::get_smoothing_enabled);

		ClassDB::bind_method(D_METHOD("set_smoothing_radius", "radius"), &WaveVisualizer::set_smoothing_radius);
		ClassDB::bind_method(D_METHOD("get_smoothing_radius"), &WaveVisualizer::get_smoothing_radius);

		ClassDB::bind_method(D_METHOD("set_smoothing_strength", "strength"), &WaveVisualizer::set_smoothing_strength);
		ClassDB::bind_method(D_METHOD("get_smoothing_strength"), &WaveVisualizer::get_smoothing_strength);

		ClassDB::bind_method(D_METHOD("set_gradient_enabled", "enabled"), &WaveVisualizer::set_gradient_enabled);
		ClassDB::bind_method(D_METHOD("get_gradient_enabled"), &WaveVisualizer::get_gradient_enabled);

		ClassDB::bind_method(D_METHOD("set_fill_strength", "strength"), &WaveVisualizer::set_fill_strength);
		ClassDB::bind_method(D_METHOD("get_fill_strength"), &WaveVisualizer::get_fill_strength);

		ClassDB::bind_method(D_METHOD("set_glow_width", "width"), &WaveVisualizer::set_glow_width);
		ClassDB::bind_method(D_METHOD("get_glow_width"), &WaveVisualizer::get_glow_width);

		ClassDB::bind_method(D_METHOD("set_dot_size", "size"), &WaveVisualizer::set_dot_size);
		ClassDB::bind_method(D_METHOD("get_dot_size"), &WaveVisualizer::get_dot_size);

		ClassDB::bind_method(D_METHOD("set_ribbon_fill_width", "width"), &WaveVisualizer::set_ribbon_fill_width);
		ClassDB::bind_method(D_METHOD("get_ribbon_fill_width"), &WaveVisualizer::get_ribbon_fill_width);

		ClassDB::bind_method(D_METHOD("set_show_view_properties", "enabled"), &WaveVisualizer::set_show_view_properties);
		ClassDB::bind_method(D_METHOD("get_show_view_properties"), &WaveVisualizer::get_show_view_properties);
		ClassDB::bind_method(D_METHOD("set_show_line_properties", "enabled"), &WaveVisualizer::set_show_line_properties);
		ClassDB::bind_method(D_METHOD("get_show_line_properties"), &WaveVisualizer::get_show_line_properties);
		ClassDB::bind_method(D_METHOD("set_show_art_properties", "enabled"), &WaveVisualizer::set_show_art_properties);
		ClassDB::bind_method(D_METHOD("get_show_art_properties"), &WaveVisualizer::get_show_art_properties);
		ClassDB::bind_method(D_METHOD("set_show_color_properties", "enabled"), &WaveVisualizer::set_show_color_properties);
		ClassDB::bind_method(D_METHOD("get_show_color_properties"), &WaveVisualizer::get_show_color_properties);

		ClassDB::bind_method(D_METHOD("update_region", "region"), &WaveVisualizer::update_region);
		ClassDB::bind_method(D_METHOD("generate_waveform"), &WaveVisualizer::generate_waveform);
		ClassDB::bind_method(D_METHOD("update_display"), &WaveVisualizer::update_display);
		ClassDB::bind_method(D_METHOD("clear"), &WaveVisualizer::clear);
		ClassDB::bind_method(D_METHOD("save_to_file", "path"), &WaveVisualizer::save_to_file);
		ClassDB::bind_method(D_METHOD("get_image_size"), &WaveVisualizer::get_image_size);

		ADD_GROUP("Inspector", "show_");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_view_properties"), "set_show_view_properties", "get_show_view_properties");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_line_properties"), "set_show_line_properties", "get_show_line_properties");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_art_properties"), "set_show_art_properties", "get_show_art_properties");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_color_properties"), "set_show_color_properties", "get_show_color_properties");

		ADD_GROUP("View", "");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "wave_direction", PROPERTY_HINT_ENUM, "Left to Right,Bottom to Top,Top to Bottom"), "set_wave_direction", "get_wave_direction");
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "virtual_size"), "set_virtual_size", "get_virtual_size");
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "view_offset"), "set_view_offset", "get_view_offset");

		ADD_GROUP("Line", "");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "line_width", PROPERTY_HINT_RANGE, "0.5,16.0,0.5"), "set_line_width", "get_line_width");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "amplitude_scale", PROPERTY_HINT_RANGE, "0.0,8.0,0.01"), "set_amplitude_scale", "get_amplitude_scale");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "antialiased"), "set_antialiased", "get_antialiased");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "draw_center_line"), "set_draw_center_line", "get_draw_center_line");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "smoothing_enabled"), "set_smoothing_enabled", "get_smoothing_enabled");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "smoothing_radius", PROPERTY_HINT_RANGE, "0,32,1"), "set_smoothing_radius", "get_smoothing_radius");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "smoothing_strength", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_smoothing_strength", "get_smoothing_strength");

		ADD_GROUP("Art", "");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "render_mode", PROPERTY_HINT_ENUM, "Lines,Mirror Fill,Ribbon,Glow,Dots"), "set_render_mode", "get_render_mode");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "gradient_enabled"), "set_gradient_enabled", "get_gradient_enabled");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fill_strength", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_fill_strength", "get_fill_strength");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "glow_width", PROPERTY_HINT_RANGE, "0.0,64.0,0.5"), "set_glow_width", "get_glow_width");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "dot_size", PROPERTY_HINT_RANGE, "0.5,32.0,0.5"), "set_dot_size", "get_dot_size");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ribbon_fill_width", PROPERTY_HINT_RANGE, "0.5,32.0,0.5"), "set_ribbon_fill_width", "get_ribbon_fill_width");

		ADD_GROUP("Colors", "");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "background_color"), "set_background_color", "get_background_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "wave_color"), "set_wave_color", "get_wave_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "accent_color"), "set_accent_color", "get_accent_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "fill_color"), "set_fill_color", "get_fill_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "glow_color"), "set_glow_color", "get_glow_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "center_color"), "set_center_color", "get_center_color");
	}

	WaveVisualizer::WaveVisualizer() {
		set_clip_contents(true);
	}

	WaveVisualizer::~WaveVisualizer() {
		audio_samples.clear();
	}

	void WaveVisualizer::_notification(int p_what) {
		switch (p_what) {
		case NOTIFICATION_READY:
		case NOTIFICATION_RESIZED:
			queue_redraw();
			break;
		case NOTIFICATION_DRAW:
			draw_waveform();
			break;
		}
	}

	void WaveVisualizer::_on_size_changed() {
		queue_redraw();
	}

	void WaveVisualizer::_validate_property(PropertyInfo& p_property) const {
		const String name = p_property.name;
		const bool view_property = name == "wave_direction" || name == "virtual_size" || name == "view_offset";
		const bool line_property = name == "line_width" || name == "amplitude_scale" || name == "antialiased" || name == "draw_center_line" || name == "smoothing_enabled" || name == "smoothing_radius" || name == "smoothing_strength";
		const bool art_property = name == "render_mode" || name == "gradient_enabled" || name == "fill_strength" || name == "glow_width" || name == "dot_size" || name == "ribbon_fill_width";
		const bool color_property = name == "background_color" || name == "wave_color" || name == "accent_color" || name == "fill_color" || name == "glow_color" || name == "center_color";

		if ((view_property && !show_view_properties) ||
				(line_property && !show_line_properties) ||
				(art_property && !show_art_properties) ||
				(color_property && !show_color_properties)) {
			p_property.usage = PROPERTY_USAGE_NO_EDITOR;
		}
	}

	bool WaveVisualizer::get_sample_range(int64_t p_start, int64_t p_end, float& r_min, float& r_max) const {
		const int64_t sample_count = audio_samples.size();
		if (sample_count <= 0) {
			return false;
		}

		const int64_t start = std::max((int64_t)0, std::min(p_start, sample_count - 1));
		const int64_t end = std::max(start + 1, std::min(p_end, sample_count));
		const float* samples = audio_samples.ptr();

		r_min = 1.0f;
		r_max = -1.0f;
		bool has_data = false;

		for (int64_t i = start; i < end; i++) {
			float value = samples[i];
			if (!std::isfinite(value)) {
				continue;
			}

			value = clamp_float(value * amplitude_scale, -1.0f, 1.0f);
			r_min = std::min(r_min, value);
			r_max = std::max(r_max, value);
			has_data = true;
		}

		return has_data;
	}

	bool WaveVisualizer::get_smoothed_sample_range(int64_t p_virtual_index, int64_t p_virtual_span, double p_samples_per_unit, float& r_min, float& r_max) const {
		if (!smoothing_enabled || smoothing_radius <= 0 || smoothing_strength <= 0.0f) {
			return false;
		}

		float min_sum = 0.0f;
		float max_sum = 0.0f;
		int count = 0;

		for (int offset = -smoothing_radius; offset <= smoothing_radius; offset++) {
			const int64_t neighbor = p_virtual_index + offset;
			if (neighbor < 0 || neighbor >= p_virtual_span) {
				continue;
			}

			const int64_t sample_start = (int64_t)std::floor((double)neighbor * p_samples_per_unit);
			const int64_t sample_end = (int64_t)std::ceil((double)(neighbor + 1) * p_samples_per_unit);
			float neighbor_min = 0.0f;
			float neighbor_max = 0.0f;
			if (!get_sample_range(sample_start, sample_end, neighbor_min, neighbor_max)) {
				continue;
			}

			min_sum += neighbor_min;
			max_sum += neighbor_max;
			count++;
		}

		if (count <= 0) {
			return false;
		}

		const float smooth_min = min_sum / (float)count;
		const float smooth_max = max_sum / (float)count;
		const float strength = clamp_float(smoothing_strength, 0.0f, 1.0f);
		r_min += (smooth_min - r_min) * strength;
		r_max += (smooth_max - r_max) * strength;
		return true;
	}

	Color WaveVisualizer::get_wave_color_for_level(float p_level) const {
		if (!gradient_enabled) {
			return wave_color;
		}

		return wave_color.lerp(accent_color, clamp_float(p_level, 0.0f, 1.0f));
	}

	Color WaveVisualizer::with_alpha(const Color& p_color, float p_alpha_multiplier) const {
		return multiply_alpha(p_color, p_alpha_multiplier);
	}

	void WaveVisualizer::draw_waveform() {
		const Vector2 size = get_size();
		if (size.x <= 0.0f || size.y <= 0.0f) {
			return;
		}

		draw_rect(Rect2(Vector2(0.0f, 0.0f), size), bg_color);

		if (audio_samples.is_empty()) {
			return;
		}

		switch (direction) {
		case DIRECTION_LEFT_TO_RIGHT:
			draw_horizontal_waveform();
			break;
		case DIRECTION_BOTTOM_TO_TOP:
			draw_vertical_waveform(false);
			break;
		case DIRECTION_TOP_TO_BOTTOM:
			draw_vertical_waveform(true);
			break;
		}
	}

	void WaveVisualizer::draw_horizontal_waveform() {
		const Vector2 size = get_size();
		const int width = std::max(1, (int)std::ceil(size.x));
		const int64_t sample_count = audio_samples.size();
		const int64_t virtual_span = virtual_size.x > 0 ? virtual_size.x : width;

		if (virtual_span <= 0 || sample_count <= 0) {
			return;
		}

		const float center_y = size.y * 0.5f;
		const float half_height = std::max(1.0f, size.y * 0.5f - line_width);
		const double samples_per_unit = (double)sample_count / (double)virtual_span;
		const float draw_width = std::max(0.5f, line_width);

		if (draw_center_line) {
			draw_line(Vector2(0.0f, center_y), Vector2(size.x, center_y), center_color, 1.0f, antialiased);
		}

		bool has_previous = false;
		float previous_top = center_y;
		float previous_bottom = center_y;
		float previous_x = 0.0f;

		for (int x = 0; x < width; x++) {
			const int64_t virtual_x = (int64_t)x + view_offset.x;
			if (virtual_x < 0 || virtual_x >= virtual_span) {
				has_previous = false;
				continue;
			}

			const int64_t sample_start = (int64_t)std::floor((double)virtual_x * samples_per_unit);
			const int64_t sample_end = (int64_t)std::ceil((double)(virtual_x + 1) * samples_per_unit);
			float min_value = 0.0f;
			float max_value = 0.0f;
			if (!get_sample_range(sample_start, sample_end, min_value, max_value)) {
				has_previous = false;
				continue;
			}

			get_smoothed_sample_range(virtual_x, virtual_span, samples_per_unit, min_value, max_value);

			const float level = std::max(std::abs(min_value), std::abs(max_value));
			const Color stroke_color = get_wave_color_for_level(level);
			const Color top_color = stroke_color;
			const Color bottom_color = gradient_enabled ? accent_color.lerp(wave_color, clamp_float(level, 0.0f, 1.0f)) : stroke_color;
			const Color current_fill_color = with_alpha(fill_color, fill_strength);
			const Color current_glow_color = glow_color;
			const float y_top = clamp_float(center_y - max_value * half_height, 0.0f, size.y);
			const float y_bottom = clamp_float(center_y - min_value * half_height, 0.0f, size.y);
			const float x_pos = (float)x + 0.5f;

			switch (render_mode) {
			case RENDER_MODE_MIRROR_FILL: {
				const float y_mirror_top = clamp_float(center_y - level * half_height, 0.0f, size.y);
				const float y_mirror_bottom = clamp_float(center_y + level * half_height, 0.0f, size.y);
				draw_line(Vector2(x_pos, y_mirror_top), Vector2(x_pos, y_mirror_bottom), current_fill_color, std::max(draw_width, ribbon_fill_width), antialiased);
				draw_line(Vector2(x_pos, y_mirror_top), Vector2(x_pos, y_mirror_bottom), stroke_color, draw_width, antialiased);
			} break;
			case RENDER_MODE_RIBBON:
				draw_line(Vector2(x_pos, y_top), Vector2(x_pos, y_bottom), current_fill_color, std::max(0.5f, ribbon_fill_width), antialiased);
				if (has_previous) {
					draw_line(Vector2(previous_x, previous_top), Vector2(x_pos, y_top), top_color, draw_width, antialiased);
					draw_line(Vector2(previous_x, previous_bottom), Vector2(x_pos, y_bottom), bottom_color, draw_width, antialiased);
				}
				break;
			case RENDER_MODE_GLOW:
				if (glow_width > 0.0f) {
					draw_line(Vector2(x_pos, y_top), Vector2(x_pos, y_bottom), current_glow_color, draw_width + glow_width, true);
				}
				draw_line(Vector2(x_pos, y_top), Vector2(x_pos, y_bottom), stroke_color, draw_width, antialiased);
				break;
			case RENDER_MODE_DOTS:
				draw_circle(Vector2(x_pos, y_top), std::max(0.5f, dot_size), top_color, true, -1.0f, antialiased);
				draw_circle(Vector2(x_pos, y_bottom), std::max(0.5f, dot_size), bottom_color, true, -1.0f, antialiased);
				break;
			case RENDER_MODE_LINES:
			default:
				draw_line(Vector2(x_pos, y_top), Vector2(x_pos, y_bottom), stroke_color, draw_width, antialiased);
				break;
			}

			previous_top = y_top;
			previous_bottom = y_bottom;
			previous_x = x_pos;
			has_previous = true;
		}
	}

	void WaveVisualizer::draw_vertical_waveform(bool p_invert_axis) {
		const Vector2 size = get_size();
		const int height = std::max(1, (int)std::ceil(size.y));
		const int64_t sample_count = audio_samples.size();
		const int64_t virtual_span = virtual_size.y > 0 ? virtual_size.y : height;

		if (virtual_span <= 0 || sample_count <= 0) {
			return;
		}

		const float center_x = size.x * 0.5f;
		const float half_width = std::max(1.0f, size.x * 0.5f - line_width);
		const double samples_per_unit = (double)sample_count / (double)virtual_span;
		const float draw_width = std::max(0.5f, line_width);

		if (draw_center_line) {
			draw_line(Vector2(center_x, 0.0f), Vector2(center_x, size.y), center_color, 1.0f, antialiased);
		}

		bool has_previous = false;
		float previous_left = center_x;
		float previous_right = center_x;
		float previous_y = 0.0f;

		for (int y = 0; y < height; y++) {
			const int64_t virtual_y = (int64_t)y + view_offset.y;
			const int64_t sample_y = p_invert_axis ? virtual_span - 1 - virtual_y : virtual_y;
			if (sample_y < 0 || sample_y >= virtual_span) {
				has_previous = false;
				continue;
			}

			const int64_t sample_start = (int64_t)std::floor((double)sample_y * samples_per_unit);
			const int64_t sample_end = (int64_t)std::ceil((double)(sample_y + 1) * samples_per_unit);
			float min_value = 0.0f;
			float max_value = 0.0f;
			if (!get_sample_range(sample_start, sample_end, min_value, max_value)) {
				has_previous = false;
				continue;
			}

			get_smoothed_sample_range(sample_y, virtual_span, samples_per_unit, min_value, max_value);

			const float level = std::max(std::abs(min_value), std::abs(max_value));
			const Color stroke_color = get_wave_color_for_level(level);
			const Color left_color = stroke_color;
			const Color right_color = gradient_enabled ? accent_color.lerp(wave_color, clamp_float(level, 0.0f, 1.0f)) : stroke_color;
			const Color current_fill_color = with_alpha(fill_color, fill_strength);
			const Color current_glow_color = glow_color;
			const float x_left = clamp_float(center_x - max_value * half_width, 0.0f, size.x);
			const float x_right = clamp_float(center_x - min_value * half_width, 0.0f, size.x);
			const float y_pos = (float)y + 0.5f;

			switch (render_mode) {
			case RENDER_MODE_MIRROR_FILL: {
				const float x_mirror_left = clamp_float(center_x - level * half_width, 0.0f, size.x);
				const float x_mirror_right = clamp_float(center_x + level * half_width, 0.0f, size.x);
				draw_line(Vector2(x_mirror_left, y_pos), Vector2(x_mirror_right, y_pos), current_fill_color, std::max(draw_width, ribbon_fill_width), antialiased);
				draw_line(Vector2(x_mirror_left, y_pos), Vector2(x_mirror_right, y_pos), stroke_color, draw_width, antialiased);
			} break;
			case RENDER_MODE_RIBBON:
				draw_line(Vector2(x_left, y_pos), Vector2(x_right, y_pos), current_fill_color, std::max(0.5f, ribbon_fill_width), antialiased);
				if (has_previous) {
					draw_line(Vector2(previous_left, previous_y), Vector2(x_left, y_pos), left_color, draw_width, antialiased);
					draw_line(Vector2(previous_right, previous_y), Vector2(x_right, y_pos), right_color, draw_width, antialiased);
				}
				break;
			case RENDER_MODE_GLOW:
				if (glow_width > 0.0f) {
					draw_line(Vector2(x_left, y_pos), Vector2(x_right, y_pos), current_glow_color, draw_width + glow_width, true);
				}
				draw_line(Vector2(x_left, y_pos), Vector2(x_right, y_pos), stroke_color, draw_width, antialiased);
				break;
			case RENDER_MODE_DOTS:
				draw_circle(Vector2(x_left, y_pos), std::max(0.5f, dot_size), left_color, true, -1.0f, antialiased);
				draw_circle(Vector2(x_right, y_pos), std::max(0.5f, dot_size), right_color, true, -1.0f, antialiased);
				break;
			case RENDER_MODE_LINES:
			default:
				draw_line(Vector2(x_left, y_pos), Vector2(x_right, y_pos), stroke_color, draw_width, antialiased);
				break;
			}

			previous_left = x_left;
			previous_right = x_right;
			previous_y = y_pos;
			has_previous = true;
		}
	}

	void WaveVisualizer::set_audio_samples(const PackedFloat32Array& p_samples) {
		audio_samples = p_samples;
		queue_redraw();
	}

	PackedFloat32Array WaveVisualizer::get_audio_samples() const {
		return audio_samples;
	}

	void WaveVisualizer::set_wave_direction(int p_direction) {
		const int new_direction = std::max(0, std::min(2, p_direction));
		if (direction == new_direction) {
			return;
		}

		direction = new_direction;
		queue_redraw();
	}

	int WaveVisualizer::get_wave_direction() const {
		return direction;
	}

	void WaveVisualizer::set_render_mode(int p_mode) {
		const int new_mode = std::max(0, std::min(4, p_mode));
		if (render_mode == new_mode) {
			return;
		}

		render_mode = new_mode;
		queue_redraw();
	}

	int WaveVisualizer::get_render_mode() const {
		return render_mode;
	}

	void WaveVisualizer::set_virtual_size(const Vector2i& p_size) {
		if (virtual_size == p_size) {
			return;
		}

		virtual_size = p_size;
		queue_redraw();
	}

	Vector2i WaveVisualizer::get_virtual_size() const {
		return virtual_size;
	}

	void WaveVisualizer::set_view_offset(const Vector2i& p_offset) {
		if (view_offset == p_offset) {
			return;
		}

		view_offset = p_offset;
		queue_redraw();
	}

	Vector2i WaveVisualizer::get_view_offset() const {
		return view_offset;
	}

	void WaveVisualizer::set_line_width(float p_width) {
		line_width = std::max(0.5f, p_width);
		queue_redraw();
	}

	float WaveVisualizer::get_line_width() const {
		return line_width;
	}

	void WaveVisualizer::set_amplitude_scale(float p_scale) {
		amplitude_scale = std::max(0.0f, p_scale);
		queue_redraw();
	}

	float WaveVisualizer::get_amplitude_scale() const {
		return amplitude_scale;
	}

	void WaveVisualizer::set_antialiased(bool p_enabled) {
		antialiased = p_enabled;
		queue_redraw();
	}

	bool WaveVisualizer::get_antialiased() const {
		return antialiased;
	}

	void WaveVisualizer::set_draw_center_line(bool p_enabled) {
		draw_center_line = p_enabled;
		queue_redraw();
	}

	bool WaveVisualizer::get_draw_center_line() const {
		return draw_center_line;
	}

	void WaveVisualizer::set_smoothing_enabled(bool p_enabled) {
		if (smoothing_enabled == p_enabled) {
			return;
		}

		smoothing_enabled = p_enabled;
		queue_redraw();
	}

	bool WaveVisualizer::get_smoothing_enabled() const {
		return smoothing_enabled;
	}

	void WaveVisualizer::set_smoothing_radius(int p_radius) {
		smoothing_radius = std::max(0, p_radius);
		queue_redraw();
	}

	int WaveVisualizer::get_smoothing_radius() const {
		return smoothing_radius;
	}

	void WaveVisualizer::set_smoothing_strength(float p_strength) {
		smoothing_strength = clamp_float(p_strength, 0.0f, 1.0f);
		queue_redraw();
	}

	float WaveVisualizer::get_smoothing_strength() const {
		return smoothing_strength;
	}

	void WaveVisualizer::set_gradient_enabled(bool p_enabled) {
		if (gradient_enabled == p_enabled) {
			return;
		}

		gradient_enabled = p_enabled;
		queue_redraw();
	}

	bool WaveVisualizer::get_gradient_enabled() const {
		return gradient_enabled;
	}

	void WaveVisualizer::set_fill_strength(float p_strength) {
		fill_strength = clamp_float(p_strength, 0.0f, 1.0f);
		queue_redraw();
	}

	float WaveVisualizer::get_fill_strength() const {
		return fill_strength;
	}

	void WaveVisualizer::set_glow_width(float p_width) {
		glow_width = std::max(0.0f, p_width);
		queue_redraw();
	}

	float WaveVisualizer::get_glow_width() const {
		return glow_width;
	}

	void WaveVisualizer::set_dot_size(float p_size) {
		dot_size = std::max(0.5f, p_size);
		queue_redraw();
	}

	float WaveVisualizer::get_dot_size() const {
		return dot_size;
	}

	void WaveVisualizer::set_ribbon_fill_width(float p_width) {
		ribbon_fill_width = std::max(0.5f, p_width);
		queue_redraw();
	}

	float WaveVisualizer::get_ribbon_fill_width() const {
		return ribbon_fill_width;
	}

	void WaveVisualizer::set_show_view_properties(bool p_enabled) {
		if (show_view_properties == p_enabled) {
			return;
		}

		show_view_properties = p_enabled;
		notify_property_list_changed();
	}

	bool WaveVisualizer::get_show_view_properties() const {
		return show_view_properties;
	}

	void WaveVisualizer::set_show_line_properties(bool p_enabled) {
		if (show_line_properties == p_enabled) {
			return;
		}

		show_line_properties = p_enabled;
		notify_property_list_changed();
	}

	bool WaveVisualizer::get_show_line_properties() const {
		return show_line_properties;
	}

	void WaveVisualizer::set_show_art_properties(bool p_enabled) {
		if (show_art_properties == p_enabled) {
			return;
		}

		show_art_properties = p_enabled;
		notify_property_list_changed();
	}

	bool WaveVisualizer::get_show_art_properties() const {
		return show_art_properties;
	}

	void WaveVisualizer::set_show_color_properties(bool p_enabled) {
		if (show_color_properties == p_enabled) {
			return;
		}

		show_color_properties = p_enabled;
		notify_property_list_changed();
	}

	bool WaveVisualizer::get_show_color_properties() const {
		return show_color_properties;
	}

	void WaveVisualizer::update_region(const Rect2i& p_region) {
		queue_redraw();
	}

	void WaveVisualizer::generate_waveform() {
		queue_redraw();
	}

	void WaveVisualizer::update_display() {
		queue_redraw();
	}

	void WaveVisualizer::clear() {
		audio_samples.clear();
		queue_redraw();
	}

	Error WaveVisualizer::save_to_file(const String& p_path) {
		const Vector2 control_size = get_size();
		const int width = std::max(1, (int)std::ceil(control_size.x));
		const int height = std::max(1, (int)std::ceil(control_size.y));
		Ref<Image> image = Image::create(width, height, false, Image::FORMAT_RGBA8);

		if (!image.is_valid()) {
			return FAILED;
		}

		image->fill(bg_color);

		const int thickness = std::max(1, (int)std::round(line_width));
		auto set_pixel_safe = [&](int p_x, int p_y, const Color& p_color) {
			if (p_x >= 0 && p_x < width && p_y >= 0 && p_y < height) {
				image->set_pixel(p_x, p_y, p_color);
			}
		};
		auto draw_vertical_image_line = [&](int p_x, int p_y_from, int p_y_to, const Color& p_color) {
			const int y_from = std::max(0, std::min(p_y_from, p_y_to));
			const int y_to = std::min(height - 1, std::max(p_y_from, p_y_to));
			const int x_from = p_x - thickness / 2;
			for (int x = x_from; x < x_from + thickness; x++) {
				for (int y = y_from; y <= y_to; y++) {
					set_pixel_safe(x, y, p_color);
				}
			}
		};
		auto draw_horizontal_image_line = [&](int p_y, int p_x_from, int p_x_to, const Color& p_color) {
			const int x_from = std::max(0, std::min(p_x_from, p_x_to));
			const int x_to = std::min(width - 1, std::max(p_x_from, p_x_to));
			const int y_from = p_y - thickness / 2;
			for (int y = y_from; y < y_from + thickness; y++) {
				for (int x = x_from; x <= x_to; x++) {
					set_pixel_safe(x, y, p_color);
				}
			}
		};

		if (audio_samples.is_empty()) {
			return image->save_png(p_path);
		}

		const int64_t sample_count = audio_samples.size();
		if (direction == DIRECTION_LEFT_TO_RIGHT) {
			const int64_t virtual_span = virtual_size.x > 0 ? virtual_size.x : width;
			const float center_y = (float)height * 0.5f;
			const float half_height = std::max(1.0f, (float)height * 0.5f - line_width);
			const double samples_per_unit = (double)sample_count / (double)std::max((int64_t)1, virtual_span);

			if (draw_center_line) {
				draw_horizontal_image_line((int)std::round(center_y), 0, width - 1, center_color);
			}

			for (int x = 0; x < width; x++) {
				const int64_t virtual_x = (int64_t)x + view_offset.x;
				if (virtual_x < 0 || virtual_x >= virtual_span) {
					continue;
				}

				const int64_t sample_start = (int64_t)std::floor((double)virtual_x * samples_per_unit);
				const int64_t sample_end = (int64_t)std::ceil((double)(virtual_x + 1) * samples_per_unit);
				float min_value = 0.0f;
				float max_value = 0.0f;
				if (!get_sample_range(sample_start, sample_end, min_value, max_value)) {
					continue;
				}

				get_smoothed_sample_range(virtual_x, virtual_span, samples_per_unit, min_value, max_value);

				const int y_top = (int)std::round(clamp_float(center_y - max_value * half_height, 0.0f, (float)(height - 1)));
				const int y_bottom = (int)std::round(clamp_float(center_y - min_value * half_height, 0.0f, (float)(height - 1)));
				draw_vertical_image_line(x, y_top, y_bottom, wave_color);
			}
		}
		else {
			const bool invert_axis = direction == DIRECTION_TOP_TO_BOTTOM;
			const int64_t virtual_span = virtual_size.y > 0 ? virtual_size.y : height;
			const float center_x = (float)width * 0.5f;
			const float half_width = std::max(1.0f, (float)width * 0.5f - line_width);
			const double samples_per_unit = (double)sample_count / (double)std::max((int64_t)1, virtual_span);

			if (draw_center_line) {
				draw_vertical_image_line((int)std::round(center_x), 0, height - 1, center_color);
			}

			for (int y = 0; y < height; y++) {
				const int64_t virtual_y = (int64_t)y + view_offset.y;
				const int64_t sample_y = invert_axis ? virtual_span - 1 - virtual_y : virtual_y;
				if (sample_y < 0 || sample_y >= virtual_span) {
					continue;
				}

				const int64_t sample_start = (int64_t)std::floor((double)sample_y * samples_per_unit);
				const int64_t sample_end = (int64_t)std::ceil((double)(sample_y + 1) * samples_per_unit);
				float min_value = 0.0f;
				float max_value = 0.0f;
				if (!get_sample_range(sample_start, sample_end, min_value, max_value)) {
					continue;
				}

				get_smoothed_sample_range(sample_y, virtual_span, samples_per_unit, min_value, max_value);

				const int x_left = (int)std::round(clamp_float(center_x - max_value * half_width, 0.0f, (float)(width - 1)));
				const int x_right = (int)std::round(clamp_float(center_x - min_value * half_width, 0.0f, (float)(width - 1)));
				draw_horizontal_image_line(y, x_left, x_right, wave_color);
			}
		}

		return image->save_png(p_path);
	}

	Vector2i WaveVisualizer::get_image_size() const {
		const Vector2 size = get_size();
		return Vector2i(std::max(1, (int)std::ceil(size.x)), std::max(1, (int)std::ceil(size.y)));
	}

	void WaveVisualizer::set_background_color(const Color& p_color) {
		if (bg_color == p_color) {
			return;
		}

		bg_color = p_color;
		queue_redraw();
	}

	Color WaveVisualizer::get_background_color() const {
		return bg_color;
	}

	void WaveVisualizer::set_wave_color(const Color& p_color) {
		if (wave_color == p_color) {
			return;
		}

		wave_color = p_color;
		queue_redraw();
	}

	Color WaveVisualizer::get_wave_color() const {
		return wave_color;
	}

	void WaveVisualizer::set_accent_color(const Color& p_color) {
		if (accent_color == p_color) {
			return;
		}

		accent_color = p_color;
		queue_redraw();
	}

	Color WaveVisualizer::get_accent_color() const {
		return accent_color;
	}

	void WaveVisualizer::set_fill_color(const Color& p_color) {
		if (fill_color == p_color) {
			return;
		}

		fill_color = p_color;
		queue_redraw();
	}

	Color WaveVisualizer::get_fill_color() const {
		return fill_color;
	}

	void WaveVisualizer::set_glow_color(const Color& p_color) {
		if (glow_color == p_color) {
			return;
		}

		glow_color = p_color;
		queue_redraw();
	}

	Color WaveVisualizer::get_glow_color() const {
		return glow_color;
	}

	void WaveVisualizer::set_center_color(const Color& p_color) {
		if (center_color == p_color) {
			return;
		}

		center_color = p_color;
		queue_redraw();
	}

	Color WaveVisualizer::get_center_color() const {
		return center_color;
	}
}
