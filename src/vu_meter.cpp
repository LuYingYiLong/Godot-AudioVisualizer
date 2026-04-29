#include "vu_meter.h"

#include <godot_cpp/classes/audio_effect.hpp>
#include <godot_cpp/classes/audio_effect_instance.hpp>
#include <godot_cpp/classes/audio_effect_spectrum_analyzer.hpp>
#include <godot_cpp/classes/audio_effect_spectrum_analyzer_instance.hpp>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/font.hpp>
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

		inline float linear_to_db(float p_linear) {
			if (p_linear <= 1e-20f) {
				return -200.0f;
			}

			return 20.0f * std::log10(p_linear);
		}
	}

	void VUMeter::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_use_bus", "enabled"), &VUMeter::set_use_bus);
		ClassDB::bind_method(D_METHOD("get_use_bus"), &VUMeter::get_use_bus);

		ClassDB::bind_method(D_METHOD("set_bus", "bus"), &VUMeter::set_bus);
		ClassDB::bind_method(D_METHOD("get_bus"), &VUMeter::get_bus);

		ClassDB::bind_method(D_METHOD("set_dbfs", "dbfs"), &VUMeter::set_dbfs);
		ClassDB::bind_method(D_METHOD("get_dbfs"), &VUMeter::get_dbfs);

		ClassDB::bind_method(D_METHOD("set_min_db", "db"), &VUMeter::set_min_db);
		ClassDB::bind_method(D_METHOD("get_min_db"), &VUMeter::get_min_db);

		ClassDB::bind_method(D_METHOD("set_max_db", "db"), &VUMeter::set_max_db);
		ClassDB::bind_method(D_METHOD("get_max_db"), &VUMeter::get_max_db);

		ClassDB::bind_method(D_METHOD("set_warn_db", "db"), &VUMeter::set_warn_db);
		ClassDB::bind_method(D_METHOD("get_warn_db"), &VUMeter::get_warn_db);

		ClassDB::bind_method(D_METHOD("set_dangerous_db", "db"), &VUMeter::set_dangerous_db);
		ClassDB::bind_method(D_METHOD("get_dangerous_db"), &VUMeter::get_dangerous_db);

		ClassDB::bind_method(D_METHOD("set_meter_height", "height"), &VUMeter::set_meter_height);
		ClassDB::bind_method(D_METHOD("get_meter_height"), &VUMeter::get_meter_height);

		ClassDB::bind_method(D_METHOD("set_scale_width", "width"), &VUMeter::set_scale_width);
		ClassDB::bind_method(D_METHOD("get_scale_width"), &VUMeter::get_scale_width);

		ClassDB::bind_method(D_METHOD("set_content_padding", "padding"), &VUMeter::set_content_padding);
		ClassDB::bind_method(D_METHOD("get_content_padding"), &VUMeter::get_content_padding);

		ClassDB::bind_method(D_METHOD("set_major_tick_db", "db"), &VUMeter::set_major_tick_db);
		ClassDB::bind_method(D_METHOD("get_major_tick_db"), &VUMeter::get_major_tick_db);

		ClassDB::bind_method(D_METHOD("set_label_font_size", "size"), &VUMeter::set_label_font_size);
		ClassDB::bind_method(D_METHOD("get_label_font_size"), &VUMeter::get_label_font_size);

		ClassDB::bind_method(D_METHOD("set_draw_ticks", "enabled"), &VUMeter::set_draw_ticks);
		ClassDB::bind_method(D_METHOD("get_draw_ticks"), &VUMeter::get_draw_ticks);

		ClassDB::bind_method(D_METHOD("set_draw_labels", "enabled"), &VUMeter::set_draw_labels);
		ClassDB::bind_method(D_METHOD("get_draw_labels"), &VUMeter::get_draw_labels);

		ClassDB::bind_method(D_METHOD("set_under_color", "color"), &VUMeter::set_under_color);
		ClassDB::bind_method(D_METHOD("get_under_color"), &VUMeter::get_under_color);

		ClassDB::bind_method(D_METHOD("set_safe_color", "color"), &VUMeter::set_safe_color);
		ClassDB::bind_method(D_METHOD("get_safe_color"), &VUMeter::get_safe_color);

		ClassDB::bind_method(D_METHOD("set_warn_color", "color"), &VUMeter::set_warn_color);
		ClassDB::bind_method(D_METHOD("get_warn_color"), &VUMeter::get_warn_color);

		ClassDB::bind_method(D_METHOD("set_dangerous_color", "color"), &VUMeter::set_dangerous_color);
		ClassDB::bind_method(D_METHOD("get_dangerous_color"), &VUMeter::get_dangerous_color);

		ClassDB::bind_method(D_METHOD("set_tick_color", "color"), &VUMeter::set_tick_color);
		ClassDB::bind_method(D_METHOD("get_tick_color"), &VUMeter::get_tick_color);

		ClassDB::bind_method(D_METHOD("set_label_color", "color"), &VUMeter::set_label_color);
		ClassDB::bind_method(D_METHOD("get_label_color"), &VUMeter::get_label_color);

		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_bus"), "set_use_bus", "get_use_bus");
		ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "bus", PROPERTY_HINT_ENUM_SUGGESTION, "Master"), "set_bus", "get_bus");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "dbfs", PROPERTY_HINT_RANGE, "-120.0,12.0,0.1"), "set_dbfs", "get_dbfs");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_db", PROPERTY_HINT_RANGE, "-120.0,-1.0,1.0"), "set_min_db", "get_min_db");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_db", PROPERTY_HINT_RANGE, "-60.0,12.0,1.0"), "set_max_db", "get_max_db");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "warn_db", PROPERTY_HINT_RANGE, "-120.0,12.0,1.0"), "set_warn_db", "get_warn_db");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "dangerous_db", PROPERTY_HINT_RANGE, "-120.0,12.0,1.0"), "set_dangerous_db", "get_dangerous_db");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "meter_height", PROPERTY_HINT_RANGE, "1.0,4096.0,1.0"), "set_meter_height", "get_meter_height");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scale_width", PROPERTY_HINT_RANGE, "0.0,96.0,1.0"), "set_scale_width", "get_scale_width");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "content_padding", PROPERTY_HINT_RANGE, "0.0,32.0,0.5"), "set_content_padding", "get_content_padding");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "major_tick_db", PROPERTY_HINT_RANGE, "1,24,1"), "set_major_tick_db", "get_major_tick_db");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "label_font_size", PROPERTY_HINT_RANGE, "4,64,1"), "set_label_font_size", "get_label_font_size");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "draw_ticks"), "set_draw_ticks", "get_draw_ticks");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "draw_labels"), "set_draw_labels", "get_draw_labels");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "under_color"), "set_under_color", "get_under_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "safe_color"), "set_safe_color", "get_safe_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "warn_color"), "set_warn_color", "get_warn_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "dangerous_color"), "set_dangerous_color", "get_dangerous_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "tick_color"), "set_tick_color", "get_tick_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "label_color"), "set_label_color", "get_label_color");
	}

	VUMeter::VUMeter() {
		set_clip_contents(true);
		set_process(false);
		apply_meter_height();
	}

	VUMeter::~VUMeter() {
	}

	void VUMeter::_notification(int p_what) {
		if (p_what == NOTIFICATION_PROCESS) {
			if (use_bus) {
				queue_redraw();
			}

			return;
		}

		if (p_what != NOTIFICATION_DRAW) {
			return;
		}

		const Vector2 size = get_size();
		if (size.x <= 0.0f || size.y <= 0.0f || max_db <= min_db) {
			return;
		}

		const float padding = std::max(0.0f, content_padding);
		const float right_edge = std::max(1.0f, size.x - padding);
		const float reserved_scale_width = (draw_ticks || draw_labels) ? std::min(std::max(0.0f, scale_width), right_edge) : 0.0f;
		const float meter_width = std::max(1.0f, right_edge - reserved_scale_width);
		const float height = size.y;

		draw_rect(Rect2(Vector2(0.0f, 0.0f), Vector2(meter_width, height)), under_color);

		const float safe_limit = clamp_float(warn_db, min_db, max_db);
		const float warn_limit = clamp_float(std::max(warn_db, dangerous_db), min_db, max_db);
		const float source_db = get_current_dbfs();
		const float current_db = std::isfinite(source_db) ? clamp_float(source_db, min_db, max_db) : min_db;

		draw_db_segment(min_db, std::min(current_db, safe_limit), safe_color, meter_width, height);

		if (current_db > safe_limit) {
			draw_db_segment(safe_limit, std::min(current_db, warn_limit), warn_color, meter_width, height);
		}

		if (current_db > warn_limit) {
			draw_db_segment(warn_limit, current_db, dangerous_color, meter_width, height);
		}

		if (!draw_ticks && !draw_labels) {
			return;
		}

		const int db_range = std::max(1, (int)std::round(max_db - min_db));
		const int major_step = std::max(1, major_tick_db);
		const float major_tick_length = std::min(4.0f, reserved_scale_width);
		const float minor_tick_length = std::min(2.0f, reserved_scale_width);
		const float label_x = meter_width + major_tick_length + 1.0f;
		const float label_width = std::max(1.0f, right_edge - label_x);
		Ref<Font> font = get_theme_default_font();

		for (int tick = 0; tick <= db_range; tick++) {
			const float y = ((float)tick / (float)db_range) * height;
			const bool is_major = tick == 0 || tick % major_step == 0;

			if (draw_ticks) {
				const float tick_length = is_major ? major_tick_length : minor_tick_length;
				draw_line(Vector2(meter_width, y), Vector2(meter_width + tick_length, y), tick_color);
			}

			if (draw_labels && is_major && font.is_valid() && label_width >= 4.0f) {
				const float label_baseline = clamp_float(y + (float)label_font_size * 0.5f, (float)label_font_size, height - 1.0f);
				draw_string(font, Vector2(label_x, label_baseline), String::num_int64(tick), HORIZONTAL_ALIGNMENT_LEFT, label_width, label_font_size, label_color);
			}
		}
	}

	void VUMeter::_validate_property(PropertyInfo& p_property) const {
		const String name = p_property.name;
		if (name == "dbfs" && use_bus) {
			p_property.usage |= PROPERTY_USAGE_READ_ONLY;
		}

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

	void VUMeter::apply_meter_height() {
		Vector2 minimum_size = get_custom_minimum_size();
		minimum_size.y = std::max(1.0f, meter_height);
		set_custom_minimum_size(minimum_size);
	}

	float VUMeter::get_current_dbfs() const {
		if (!use_bus) {
			return dbfs;
		}

		bool found_analyzer = false;
		const float bus_dbfs = get_bus_dbfs(found_analyzer);
		return found_analyzer ? bus_dbfs : dbfs;
	}

	float VUMeter::get_bus_dbfs(bool& r_found_analyzer) const {
		r_found_analyzer = false;

		AudioServer* audio_server = AudioServer::get_singleton();
		if (!audio_server) {
			return dbfs;
		}

		const int bus_index = audio_server->get_bus_index(bus);
		if (bus_index < 0) {
			return dbfs;
		}

		const int effect_count = audio_server->get_bus_effect_count(bus_index);
		for (int effect_index = 0; effect_index < effect_count; effect_index++) {
			if (!audio_server->is_bus_effect_enabled(bus_index, effect_index)) {
				continue;
			}

			Ref<AudioEffect> effect = audio_server->get_bus_effect(bus_index, effect_index);
			Ref<AudioEffectSpectrumAnalyzer> analyzer = effect;
			if (!analyzer.is_valid()) {
				continue;
			}

			Ref<AudioEffectInstance> effect_instance = audio_server->get_bus_effect_instance(bus_index, effect_index);
			Ref<AudioEffectSpectrumAnalyzerInstance> analyzer_instance = effect_instance;
			if (!analyzer_instance.is_valid()) {
				continue;
			}

			const float mix_rate = std::max(1.0f, audio_server->get_mix_rate());
			const Vector2 magnitude = analyzer_instance->get_magnitude_for_frequency_range(20.0f, std::max(20.0f, mix_rate * 0.5f), AudioEffectSpectrumAnalyzerInstance::MAGNITUDE_MAX);
			const float linear_magnitude = std::max(magnitude.x, magnitude.y);
			r_found_analyzer = true;
			return linear_to_db(linear_magnitude);
		}

		return dbfs;
	}

	float VUMeter::db_to_y(float p_db, float p_height) const {
		const float normalized = (clamp_float(p_db, min_db, max_db) - min_db) / (max_db - min_db);
		return p_height - normalized * p_height;
	}

	void VUMeter::draw_db_segment(float p_from_db, float p_to_db, const Color& p_color, float p_meter_width, float p_height) {
		if (p_to_db <= p_from_db) {
			return;
		}

		const float y_top = db_to_y(p_to_db, p_height);
		const float y_bottom = db_to_y(p_from_db, p_height);
		draw_rect(Rect2(Vector2(0.0f, y_top), Vector2(p_meter_width, y_bottom - y_top)), p_color);
	}

	void VUMeter::set_dbfs(float p_dbfs) {
		dbfs = std::isfinite(p_dbfs) ? p_dbfs : min_db;
		queue_redraw();
	}

	float VUMeter::get_dbfs() const {
		return dbfs;
	}

	void VUMeter::set_min_db(float p_db) {
		const float new_min = std::min(p_db, max_db - 1.0f);
		if (min_db == new_min) {
			return;
		}

		min_db = new_min;
		dbfs = clamp_float(dbfs, min_db, max_db);
		queue_redraw();
	}

	float VUMeter::get_min_db() const {
		return min_db;
	}

	void VUMeter::set_max_db(float p_db) {
		const float new_max = std::max(p_db, min_db + 1.0f);
		if (max_db == new_max) {
			return;
		}

		max_db = new_max;
		dbfs = clamp_float(dbfs, min_db, max_db);
		queue_redraw();
	}

	float VUMeter::get_max_db() const {
		return max_db;
	}

	void VUMeter::set_warn_db(float p_db) {
		warn_db = p_db;
		queue_redraw();
	}

	float VUMeter::get_warn_db() const {
		return warn_db;
	}

	void VUMeter::set_dangerous_db(float p_db) {
		dangerous_db = p_db;
		queue_redraw();
	}

	float VUMeter::get_dangerous_db() const {
		return dangerous_db;
	}

	void VUMeter::set_meter_height(float p_height) {
		const float new_height = std::max(1.0f, p_height);
		if (meter_height == new_height) {
			return;
		}

		meter_height = new_height;
		apply_meter_height();
		queue_redraw();
	}

	float VUMeter::get_meter_height() const {
		return meter_height;
	}

	void VUMeter::set_scale_width(float p_width) {
		const float new_width = std::max(0.0f, p_width);
		if (scale_width == new_width) {
			return;
		}

		scale_width = new_width;
		queue_redraw();
	}

	float VUMeter::get_scale_width() const {
		return scale_width;
	}

	void VUMeter::set_content_padding(float p_padding) {
		const float new_padding = std::max(0.0f, p_padding);
		if (content_padding == new_padding) {
			return;
		}

		content_padding = new_padding;
		queue_redraw();
	}

	float VUMeter::get_content_padding() const {
		return content_padding;
	}

	void VUMeter::set_use_bus(bool p_enabled) {
		if (use_bus == p_enabled) {
			return;
		}

		use_bus = p_enabled;
		set_process(use_bus);
		notify_property_list_changed();
		queue_redraw();
	}

	bool VUMeter::get_use_bus() const {
		return use_bus;
	}

	void VUMeter::set_bus(const StringName& p_bus) {
		if (bus == p_bus) {
			return;
		}

		bus = p_bus;
		queue_redraw();
	}

	StringName VUMeter::get_bus() const {
		return bus;
	}

	void VUMeter::set_major_tick_db(int p_db) {
		const int new_db = std::max(1, p_db);
		if (major_tick_db == new_db) {
			return;
		}

		major_tick_db = new_db;
		queue_redraw();
	}

	int VUMeter::get_major_tick_db() const {
		return major_tick_db;
	}

	void VUMeter::set_label_font_size(int p_size) {
		const int new_size = std::max(1, p_size);
		if (label_font_size == new_size) {
			return;
		}

		label_font_size = new_size;
		queue_redraw();
	}

	int VUMeter::get_label_font_size() const {
		return label_font_size;
	}

	void VUMeter::set_draw_ticks(bool p_enabled) {
		if (draw_ticks == p_enabled) {
			return;
		}

		draw_ticks = p_enabled;
		queue_redraw();
	}

	bool VUMeter::get_draw_ticks() const {
		return draw_ticks;
	}

	void VUMeter::set_draw_labels(bool p_enabled) {
		if (draw_labels == p_enabled) {
			return;
		}

		draw_labels = p_enabled;
		queue_redraw();
	}

	bool VUMeter::get_draw_labels() const {
		return draw_labels;
	}

	void VUMeter::set_under_color(const Color& p_color) {
		under_color = p_color;
		queue_redraw();
	}

	Color VUMeter::get_under_color() const {
		return under_color;
	}

	void VUMeter::set_safe_color(const Color& p_color) {
		safe_color = p_color;
		queue_redraw();
	}

	Color VUMeter::get_safe_color() const {
		return safe_color;
	}

	void VUMeter::set_warn_color(const Color& p_color) {
		warn_color = p_color;
		queue_redraw();
	}

	Color VUMeter::get_warn_color() const {
		return warn_color;
	}

	void VUMeter::set_dangerous_color(const Color& p_color) {
		dangerous_color = p_color;
		queue_redraw();
	}

	Color VUMeter::get_dangerous_color() const {
		return dangerous_color;
	}

	void VUMeter::set_tick_color(const Color& p_color) {
		tick_color = p_color;
		queue_redraw();
	}

	Color VUMeter::get_tick_color() const {
		return tick_color;
	}

	void VUMeter::set_label_color(const Color& p_color) {
		label_color = p_color;
		queue_redraw();
	}

	Color VUMeter::get_label_color() const {
		return label_color;
	}
}
