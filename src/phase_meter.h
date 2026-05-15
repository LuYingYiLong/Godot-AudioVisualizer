#ifndef PHASE_METER_H
#define PHASE_METER_H

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include <vector>

namespace godot {
	class PhaseMeter : public Control {
		GDCLASS(PhaseMeter, Control)

	public:
		enum Orientation {
			ORIENTATION_HORIZONTAL = 0,
			ORIENTATION_VERTICAL = 1
		};

		enum MeterStyle {
			METER_STYLE_BAR = 0,
			METER_STYLE_SEGMENTS = 1,
			METER_STYLE_GAUGE = 2
		};

	private:
		bool use_bus = true;
		StringName bus = StringName("Master");
		int bus_backend = 0;
		PackedVector2Array samples;

		int sample_count = 1024;
		int orientation = ORIENTATION_HORIZONTAL;
		int meter_style = METER_STYLE_BAR;
		float phase = 0.0f;
		float displayed_phase = 0.0f;
		float smoothing = 0.25f;
		bool animation_enabled = true;
		float content_padding = 4.0f;
		float marker_width = 2.0f;
		int tick_count = 4;
		int segment_count = 24;
		float segment_gap = 2.0f;
		float gauge_arc_width = 6.0f;
		float gauge_needle_length = 0.82f;
		float gauge_hub_radius = 5.0f;
		int label_font_size = 9;
		bool draw_ticks = true;
		bool draw_labels = true;

		Color background_color = Color("#000000");
		Color track_color = Color("#1a1a1a");
		Color negative_color = Color(1.0f, 0.18f, 0.28f, 1.0f);
		Color neutral_color = Color(1.0f, 0.82f, 0.24f, 1.0f);
		Color positive_color = Color(0.35f, 1.0f, 0.72f, 1.0f);
		Color marker_color = Color("#ffffffff");
		Color tick_color = Color("#ffffff99");
		Color label_color = Color("#ffffffff");

		std::vector<Vector2> live_samples;

		void update_processing();
		void update_meter(float p_delta);
		float get_current_phase();
		float calculate_phase(const PackedVector2Array& p_samples) const;
		float calculate_phase(const std::vector<Vector2>& p_samples) const;
		void append_samples(const PackedVector2Array& p_frames);
		Rect2 get_track_rect() const;
		Color get_phase_color(float p_phase) const;
		void draw_meter();
		void draw_horizontal_meter(const Rect2& p_rect);
		void draw_vertical_meter(const Rect2& p_rect);
		void draw_segment_meter(const Rect2& p_rect);
		void draw_gauge_meter(const Rect2& p_rect);
		void draw_gauge_arc(const Vector2& p_center, float p_radius, float p_from_value, float p_to_value, const Color& p_color, float p_width);
		void draw_horizontal_labels(const Rect2& p_rect);
		void draw_vertical_labels(const Rect2& p_rect);

	protected:
		static void _bind_methods();
		void _notification(int p_what);
		void _validate_property(PropertyInfo& p_property) const;

	public:
		PhaseMeter();
		~PhaseMeter();

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

		void set_orientation(int p_orientation);
		int get_orientation() const;

		void set_meter_style(int p_style);
		int get_meter_style() const;

		void set_phase(float p_phase);
		float get_phase() const;

		void set_smoothing(float p_smoothing);
		float get_smoothing() const;

		void set_animation_enabled(bool p_enabled);
		bool get_animation_enabled() const;

		void set_content_padding(float p_padding);
		float get_content_padding() const;

		void set_marker_width(float p_width);
		float get_marker_width() const;

		void set_tick_count(int p_count);
		int get_tick_count() const;

		void set_segment_count(int p_count);
		int get_segment_count() const;

		void set_segment_gap(float p_gap);
		float get_segment_gap() const;

		void set_gauge_arc_width(float p_width);
		float get_gauge_arc_width() const;

		void set_gauge_needle_length(float p_length);
		float get_gauge_needle_length() const;

		void set_gauge_hub_radius(float p_radius);
		float get_gauge_hub_radius() const;

		void set_label_font_size(int p_size);
		int get_label_font_size() const;

		void set_draw_ticks(bool p_enabled);
		bool get_draw_ticks() const;

		void set_draw_labels(bool p_enabled);
		bool get_draw_labels() const;

		void set_background_color(const Color& p_color);
		Color get_background_color() const;

		void set_track_color(const Color& p_color);
		Color get_track_color() const;

		void set_negative_color(const Color& p_color);
		Color get_negative_color() const;

		void set_neutral_color(const Color& p_color);
		Color get_neutral_color() const;

		void set_positive_color(const Color& p_color);
		Color get_positive_color() const;

		void set_marker_color(const Color& p_color);
		Color get_marker_color() const;

		void set_tick_color(const Color& p_color);
		Color get_tick_color() const;

		void set_label_color(const Color& p_color);
		Color get_label_color() const;
	};
}

VARIANT_ENUM_CAST(PhaseMeter::Orientation);
VARIANT_ENUM_CAST(PhaseMeter::MeterStyle);

#endif // PHASE_METER_H
