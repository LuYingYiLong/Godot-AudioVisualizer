#ifndef VU_METER_H
#define VU_METER_H

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {
	class VUMeter : public Control {
		GDCLASS(VUMeter, Control)

	private:
		float dbfs = -60.0f;
		float min_db = -60.0f;
		float max_db = 0.0f;
		float warn_db = -12.0f;
		float dangerous_db = -6.0f;
		float meter_height = 120.0f;
		float scale_width = 18.0f;
		float content_padding = 3.0f;
		bool use_bus = true;
		StringName bus = StringName("Master");
		bool animation_enabled = true;
		float displayed_dbfs = -60.0f;
		float rise_db_per_second = 180.0f;
		float fall_db_per_second = 90.0f;
		bool peak_hold_enabled = true;
		float peak_dbfs = -60.0f;
		float peak_age = 0.0f;
		float peak_hold_time = 0.8f;
		float peak_fall_db_per_second = 24.0f;
		float peak_bar_height = 2.0f;
		int major_tick_db = 6;
		int label_font_size = 8;
		bool draw_ticks = true;
		bool draw_labels = true;

		Color under_color = Color(0.129f, 0.129f, 0.129f, 1.0f);
		Color safe_color = Color(0.04f, 0.8f, 0.0f, 1.0f);
		Color warn_color = Color(1.0f, 0.75f, 0.0f, 1.0f);
		Color dangerous_color = Color(1.0f, 0.175f, 0.175f, 1.0f);
		Color peak_color = Color(1.0f, 1.0f, 1.0f, 0.9f);
		Color tick_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
		Color label_color = Color(1.0f, 1.0f, 1.0f, 1.0f);

		void apply_meter_height();
		void update_processing();
		void update_meter(float p_delta);
		float move_toward(float p_from, float p_to, float p_delta) const;
		float get_current_dbfs() const;
		float get_bus_dbfs(bool& r_found_analyzer) const;
		float db_to_y(float p_db, float p_height) const;
		void draw_db_segment(float p_from_db, float p_to_db, const Color& p_color, float p_meter_width, float p_height);
		void draw_peak_bar(float p_meter_width, float p_height);

	protected:
		static void _bind_methods();
		void _notification(int p_what);
		void _validate_property(PropertyInfo& p_property) const;

	public:
		VUMeter();
		~VUMeter();

		void set_dbfs(float p_dbfs);
		float get_dbfs() const;

		void set_min_db(float p_db);
		float get_min_db() const;

		void set_max_db(float p_db);
		float get_max_db() const;

		void set_warn_db(float p_db);
		float get_warn_db() const;

		void set_dangerous_db(float p_db);
		float get_dangerous_db() const;

		void set_meter_height(float p_height);
		float get_meter_height() const;

		void set_scale_width(float p_width);
		float get_scale_width() const;

		void set_content_padding(float p_padding);
		float get_content_padding() const;

		void set_use_bus(bool p_enabled);
		bool get_use_bus() const;

		void set_bus(const StringName& p_bus);
		StringName get_bus() const;

		void set_animation_enabled(bool p_enabled);
		bool get_animation_enabled() const;

		void set_rise_db_per_second(float p_speed);
		float get_rise_db_per_second() const;

		void set_fall_db_per_second(float p_speed);
		float get_fall_db_per_second() const;

		void set_peak_hold_enabled(bool p_enabled);
		bool get_peak_hold_enabled() const;

		void set_peak_hold_time(float p_time);
		float get_peak_hold_time() const;

		void set_peak_fall_db_per_second(float p_speed);
		float get_peak_fall_db_per_second() const;

		void set_peak_bar_height(float p_height);
		float get_peak_bar_height() const;

		void set_major_tick_db(int p_db);
		int get_major_tick_db() const;

		void set_label_font_size(int p_size);
		int get_label_font_size() const;

		void set_draw_ticks(bool p_enabled);
		bool get_draw_ticks() const;

		void set_draw_labels(bool p_enabled);
		bool get_draw_labels() const;

		void set_under_color(const Color& p_color);
		Color get_under_color() const;

		void set_safe_color(const Color& p_color);
		Color get_safe_color() const;

		void set_warn_color(const Color& p_color);
		Color get_warn_color() const;

		void set_dangerous_color(const Color& p_color);
		Color get_dangerous_color() const;

		void set_peak_color(const Color& p_color);
		Color get_peak_color() const;

		void set_tick_color(const Color& p_color);
		Color get_tick_color() const;

		void set_label_color(const Color& p_color);
		Color get_label_color() const;
	};
}

#endif // VU_METER_H
