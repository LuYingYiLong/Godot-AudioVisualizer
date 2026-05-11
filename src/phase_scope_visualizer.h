#ifndef PHASE_SCOPE_VISUALIZER_H
#define PHASE_SCOPE_VISUALIZER_H

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include <vector>

namespace godot {
	class PhaseScopeVisualizer : public Control {
		GDCLASS(PhaseScopeVisualizer, Control)

	private:
		bool use_bus = true;
		StringName bus = StringName("Master");
		PackedVector2Array samples;

		int sample_count = 1024;
		bool frozen = false;
		bool draw_grid = true;
		bool draw_meter = true;
		float gain = 1.0f;
		float point_size = 2.0f;
		float padding = 8.0f;
		float meter_width = 44.0f;
		float smoothing = 0.18f;

		Color background_color = Color(0.052f, 0.052f, 0.075f, 1.0f);
		Color grid_color = Color(0.95f, 0.65f, 0.95f, 0.25f);
		Color point_color = Color(1.0f, 0.56f, 0.88f, 0.85f);
		Color meter_negative_color = Color(1.0f, 0.2f, 0.38f, 1.0f);
		Color meter_positive_color = Color(0.35f, 1.0f, 0.72f, 1.0f);
		Color meter_neutral_color = Color(1.0f, 0.82f, 0.24f, 1.0f);

		std::vector<Vector2> live_samples;
		float correlation = 0.0f;
		float width = 0.0f;
		float balance = 0.0f;

		void update_samples();
		void append_samples(const PackedVector2Array& p_frames);
		void update_phase_metrics();
		Rect2 get_scope_rect() const;
		Rect2 get_meter_rect() const;
		void draw_scope_grid(const Rect2& p_rect);
		void draw_goniometer(const Rect2& p_rect);
		void draw_correlation_meter(const Rect2& p_rect);

	protected:
		static void _bind_methods();
		void _notification(int p_what);
		void _validate_property(PropertyInfo& p_property) const;

	public:
		PhaseScopeVisualizer();
		~PhaseScopeVisualizer();

		void set_use_bus(bool p_enabled);
		bool get_use_bus() const;

		void set_bus(const StringName& p_bus);
		StringName get_bus() const;

		void set_samples(const PackedVector2Array& p_samples);
		PackedVector2Array get_samples() const;

		void set_sample_count(int p_count);
		int get_sample_count() const;

		void set_frozen(bool p_enabled);
		bool get_frozen() const;

		void set_draw_grid(bool p_enabled);
		bool get_draw_grid() const;

		void set_draw_meter(bool p_enabled);
		bool get_draw_meter() const;

		void set_gain(float p_gain);
		float get_gain() const;

		void set_point_size(float p_size);
		float get_point_size() const;

		void set_padding(float p_padding);
		float get_padding() const;

		void set_meter_width(float p_width);
		float get_meter_width() const;

		void set_smoothing(float p_smoothing);
		float get_smoothing() const;

		void set_background_color(const Color& p_color);
		Color get_background_color() const;

		void set_grid_color(const Color& p_color);
		Color get_grid_color() const;

		void set_point_color(const Color& p_color);
		Color get_point_color() const;

		void set_meter_negative_color(const Color& p_color);
		Color get_meter_negative_color() const;

		void set_meter_positive_color(const Color& p_color);
		Color get_meter_positive_color() const;

		void set_meter_neutral_color(const Color& p_color);
		Color get_meter_neutral_color() const;

		float get_correlation() const;
		float get_width() const;
		float get_balance() const;
	};
}

#endif // PHASE_SCOPE_VISUALIZER_H
