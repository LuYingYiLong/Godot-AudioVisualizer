#include "phase_meter.h"

#include "audio_visualizer_bus_utils.h"

#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <algorithm>
#include <cmath>

namespace godot {
	namespace {
		float move_toward_phase(float p_from, float p_to, float p_weight) {
			const float weight = AudioVisualizerBusUtils::clamp_float(p_weight, 0.0f, 1.0f);
			return p_from + (p_to - p_from) * weight;
		}

		float phase_to_gauge_angle(float p_phase) {
			const float start_angle = -2.61799388f;
			const float end_angle = -0.523598776f;
			const float value = (AudioVisualizerBusUtils::clamp_float(p_phase, -1.0f, 1.0f) + 1.0f) * 0.5f;
			return start_angle + (end_angle - start_angle) * value;
		}
	}

	void PhaseMeter::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_use_bus", "enabled"), &PhaseMeter::set_use_bus);
		ClassDB::bind_method(D_METHOD("get_use_bus"), &PhaseMeter::get_use_bus);
		ClassDB::bind_method(D_METHOD("set_bus", "bus"), &PhaseMeter::set_bus);
		ClassDB::bind_method(D_METHOD("get_bus"), &PhaseMeter::get_bus);
		ClassDB::bind_method(D_METHOD("set_bus_backend", "backend"), &PhaseMeter::set_bus_backend);
		ClassDB::bind_method(D_METHOD("get_bus_backend"), &PhaseMeter::get_bus_backend);
		ClassDB::bind_method(D_METHOD("set_samples", "samples"), &PhaseMeter::set_samples);
		ClassDB::bind_method(D_METHOD("get_samples"), &PhaseMeter::get_samples);
		ClassDB::bind_method(D_METHOD("set_sample_count", "count"), &PhaseMeter::set_sample_count);
		ClassDB::bind_method(D_METHOD("get_sample_count"), &PhaseMeter::get_sample_count);
		ClassDB::bind_method(D_METHOD("set_orientation", "orientation"), &PhaseMeter::set_orientation);
		ClassDB::bind_method(D_METHOD("get_orientation"), &PhaseMeter::get_orientation);
		ClassDB::bind_method(D_METHOD("set_meter_style", "style"), &PhaseMeter::set_meter_style);
		ClassDB::bind_method(D_METHOD("get_meter_style"), &PhaseMeter::get_meter_style);
		ClassDB::bind_method(D_METHOD("set_phase", "phase"), &PhaseMeter::set_phase);
		ClassDB::bind_method(D_METHOD("get_phase"), &PhaseMeter::get_phase);
		ClassDB::bind_method(D_METHOD("set_smoothing", "smoothing"), &PhaseMeter::set_smoothing);
		ClassDB::bind_method(D_METHOD("get_smoothing"), &PhaseMeter::get_smoothing);
		ClassDB::bind_method(D_METHOD("set_animation_enabled", "enabled"), &PhaseMeter::set_animation_enabled);
		ClassDB::bind_method(D_METHOD("get_animation_enabled"), &PhaseMeter::get_animation_enabled);
		ClassDB::bind_method(D_METHOD("set_content_padding", "padding"), &PhaseMeter::set_content_padding);
		ClassDB::bind_method(D_METHOD("get_content_padding"), &PhaseMeter::get_content_padding);
		ClassDB::bind_method(D_METHOD("set_marker_width", "width"), &PhaseMeter::set_marker_width);
		ClassDB::bind_method(D_METHOD("get_marker_width"), &PhaseMeter::get_marker_width);
		ClassDB::bind_method(D_METHOD("set_tick_count", "count"), &PhaseMeter::set_tick_count);
		ClassDB::bind_method(D_METHOD("get_tick_count"), &PhaseMeter::get_tick_count);
		ClassDB::bind_method(D_METHOD("set_segment_count", "count"), &PhaseMeter::set_segment_count);
		ClassDB::bind_method(D_METHOD("get_segment_count"), &PhaseMeter::get_segment_count);
		ClassDB::bind_method(D_METHOD("set_segment_gap", "gap"), &PhaseMeter::set_segment_gap);
		ClassDB::bind_method(D_METHOD("get_segment_gap"), &PhaseMeter::get_segment_gap);
		ClassDB::bind_method(D_METHOD("set_gauge_arc_width", "width"), &PhaseMeter::set_gauge_arc_width);
		ClassDB::bind_method(D_METHOD("get_gauge_arc_width"), &PhaseMeter::get_gauge_arc_width);
		ClassDB::bind_method(D_METHOD("set_gauge_needle_length", "length"), &PhaseMeter::set_gauge_needle_length);
		ClassDB::bind_method(D_METHOD("get_gauge_needle_length"), &PhaseMeter::get_gauge_needle_length);
		ClassDB::bind_method(D_METHOD("set_gauge_hub_radius", "radius"), &PhaseMeter::set_gauge_hub_radius);
		ClassDB::bind_method(D_METHOD("get_gauge_hub_radius"), &PhaseMeter::get_gauge_hub_radius);
		ClassDB::bind_method(D_METHOD("set_label_font_size", "size"), &PhaseMeter::set_label_font_size);
		ClassDB::bind_method(D_METHOD("get_label_font_size"), &PhaseMeter::get_label_font_size);
		ClassDB::bind_method(D_METHOD("set_draw_ticks", "enabled"), &PhaseMeter::set_draw_ticks);
		ClassDB::bind_method(D_METHOD("get_draw_ticks"), &PhaseMeter::get_draw_ticks);
		ClassDB::bind_method(D_METHOD("set_draw_labels", "enabled"), &PhaseMeter::set_draw_labels);
		ClassDB::bind_method(D_METHOD("get_draw_labels"), &PhaseMeter::get_draw_labels);
		ClassDB::bind_method(D_METHOD("set_background_color", "color"), &PhaseMeter::set_background_color);
		ClassDB::bind_method(D_METHOD("get_background_color"), &PhaseMeter::get_background_color);
		ClassDB::bind_method(D_METHOD("set_track_color", "color"), &PhaseMeter::set_track_color);
		ClassDB::bind_method(D_METHOD("get_track_color"), &PhaseMeter::get_track_color);
		ClassDB::bind_method(D_METHOD("set_negative_color", "color"), &PhaseMeter::set_negative_color);
		ClassDB::bind_method(D_METHOD("get_negative_color"), &PhaseMeter::get_negative_color);
		ClassDB::bind_method(D_METHOD("set_neutral_color", "color"), &PhaseMeter::set_neutral_color);
		ClassDB::bind_method(D_METHOD("get_neutral_color"), &PhaseMeter::get_neutral_color);
		ClassDB::bind_method(D_METHOD("set_positive_color", "color"), &PhaseMeter::set_positive_color);
		ClassDB::bind_method(D_METHOD("get_positive_color"), &PhaseMeter::get_positive_color);
		ClassDB::bind_method(D_METHOD("set_marker_color", "color"), &PhaseMeter::set_marker_color);
		ClassDB::bind_method(D_METHOD("get_marker_color"), &PhaseMeter::get_marker_color);
		ClassDB::bind_method(D_METHOD("set_tick_color", "color"), &PhaseMeter::set_tick_color);
		ClassDB::bind_method(D_METHOD("get_tick_color"), &PhaseMeter::get_tick_color);
		ClassDB::bind_method(D_METHOD("set_label_color", "color"), &PhaseMeter::set_label_color);
		ClassDB::bind_method(D_METHOD("get_label_color"), &PhaseMeter::get_label_color);

		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_bus"), "set_use_bus", "get_use_bus");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "bus_backend", PROPERTY_HINT_ENUM, "Godot,FmodPlayer"), "set_bus_backend", "get_bus_backend");
		ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "bus", PROPERTY_HINT_ENUM_SUGGESTION, "Master"), "set_bus", "get_bus");
		ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR2_ARRAY, "samples"), "set_samples", "get_samples");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_count", PROPERTY_HINT_RANGE, "16,8192,1"), "set_sample_count", "get_sample_count");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "orientation", PROPERTY_HINT_ENUM, "Horizontal,Vertical"), "set_orientation", "get_orientation");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "meter_style", PROPERTY_HINT_ENUM, "Bar,Segments,Gauge"), "set_meter_style", "get_meter_style");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "phase", PROPERTY_HINT_RANGE, "-1.0,1.0,0.001"), "set_phase", "get_phase");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "animation_enabled"), "set_animation_enabled", "get_animation_enabled");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "smoothing", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_smoothing", "get_smoothing");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "content_padding", PROPERTY_HINT_RANGE, "0.0,64.0,0.5"), "set_content_padding", "get_content_padding");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "marker_width", PROPERTY_HINT_RANGE, "1.0,24.0,0.5"), "set_marker_width", "get_marker_width");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "tick_count", PROPERTY_HINT_RANGE, "2,16,1"), "set_tick_count", "get_tick_count");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "segment_count", PROPERTY_HINT_RANGE, "3,96,1"), "set_segment_count", "get_segment_count");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "segment_gap", PROPERTY_HINT_RANGE, "0.0,16.0,0.5"), "set_segment_gap", "get_segment_gap");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gauge_arc_width", PROPERTY_HINT_RANGE, "1.0,48.0,0.5"), "set_gauge_arc_width", "get_gauge_arc_width");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gauge_needle_length", PROPERTY_HINT_RANGE, "0.1,1.2,0.01"), "set_gauge_needle_length", "get_gauge_needle_length");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gauge_hub_radius", PROPERTY_HINT_RANGE, "0.0,48.0,0.5"), "set_gauge_hub_radius", "get_gauge_hub_radius");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "label_font_size", PROPERTY_HINT_RANGE, "4,64,1"), "set_label_font_size", "get_label_font_size");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "draw_ticks"), "set_draw_ticks", "get_draw_ticks");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "draw_labels"), "set_draw_labels", "get_draw_labels");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "background_color"), "set_background_color", "get_background_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "track_color"), "set_track_color", "get_track_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "negative_color"), "set_negative_color", "get_negative_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "neutral_color"), "set_neutral_color", "get_neutral_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "positive_color"), "set_positive_color", "get_positive_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "marker_color"), "set_marker_color", "get_marker_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "tick_color"), "set_tick_color", "get_tick_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "label_color"), "set_label_color", "get_label_color");
	}

	PhaseMeter::PhaseMeter() {
		set_clip_contents(true);
		set_custom_minimum_size(Vector2(160.0f, 28.0f));
		live_samples.reserve(static_cast<size_t>(sample_count));
		update_processing();
	}

	PhaseMeter::~PhaseMeter() {
	}

	void PhaseMeter::_notification(int p_what) {
		if (p_what == NOTIFICATION_PROCESS) {
			update_meter(static_cast<float>(get_process_delta_time()));
			queue_redraw();
			return;
		}

		if (p_what != NOTIFICATION_DRAW) {
			return;
		}

		draw_meter();
	}

	void PhaseMeter::_validate_property(PropertyInfo& p_property) const {
		const String name = p_property.name;
		if (name == StringName("bus")) {
			p_property.hint = PROPERTY_HINT_ENUM;
			p_property.hint_string = AudioVisualizerBusUtils::get_bus_hint_string(bus_backend);
		}
		else if (name == StringName("orientation") && meter_style == METER_STYLE_GAUGE) {
			p_property.usage = PROPERTY_USAGE_NO_EDITOR;
		}
		else if (name == StringName("phase") && (use_bus || samples.size() > 0)) {
			p_property.usage |= PROPERTY_USAGE_READ_ONLY;
		}
		else if (name == StringName("label_font_size") && !draw_labels) {
			p_property.usage = PROPERTY_USAGE_NO_EDITOR;
		}
		else if (name == StringName("tick_count") && !draw_ticks) {
			p_property.usage = PROPERTY_USAGE_NO_EDITOR;
		}
		else if ((name == StringName("segment_count") || name == StringName("segment_gap")) && meter_style != METER_STYLE_SEGMENTS) {
			p_property.usage = PROPERTY_USAGE_NO_EDITOR;
		}
		else if ((name == StringName("gauge_arc_width") || name == StringName("gauge_needle_length") || name == StringName("gauge_hub_radius")) && meter_style != METER_STYLE_GAUGE) {
			p_property.usage = PROPERTY_USAGE_NO_EDITOR;
		}
	}

	void PhaseMeter::update_processing() {
		set_process(use_bus || animation_enabled);
	}

	void PhaseMeter::update_meter(float p_delta) {
		const float next_phase = get_current_phase();
		phase = next_phase;

		const float delta = std::max(0.0f, p_delta);
		if (!animation_enabled || delta <= 0.0f || smoothing <= 0.0f) {
			displayed_phase = phase;
		}
		else {
			displayed_phase = move_toward_phase(displayed_phase, phase, smoothing * delta * 60.0f);
		}

		displayed_phase = AudioVisualizerBusUtils::clamp_float(displayed_phase, -1.0f, 1.0f);
	}

	float PhaseMeter::get_current_phase() {
		if (use_bus) {
			PackedVector2Array frames;
			if (AudioVisualizerBusUtils::get_capture_frames(bus, bus_backend, std::max(1, sample_count), frames)) {
				if (frames.size() > 0) {
					append_samples(frames);
					return calculate_phase(live_samples);
				}

				live_samples.clear();
				return 0.0f;
			}
		}

		live_samples.clear();
		if (samples.size() > 0) {
			return calculate_phase(samples);
		}

		return AudioVisualizerBusUtils::clamp_float(phase, -1.0f, 1.0f);
	}

	float PhaseMeter::calculate_phase(const PackedVector2Array& p_samples) const {
		const int count = p_samples.size();
		if (count <= 0) {
			return 0.0f;
		}

		double sum_l2 = 0.0;
		double sum_r2 = 0.0;
		double sum_lr = 0.0;
		const Vector2* sample_ptr = p_samples.ptr();
		for (int i = 0; i < count; i++) {
			const double left = sample_ptr[i].x;
			const double right = sample_ptr[i].y;
			sum_l2 += left * left;
			sum_r2 += right * right;
			sum_lr += left * right;
		}

		const double denom = std::sqrt(std::max(1e-12, sum_l2 * sum_r2));
		if (denom <= 1e-9) {
			return 0.0f;
		}

		return AudioVisualizerBusUtils::clamp_float(static_cast<float>(sum_lr / denom), -1.0f, 1.0f);
	}

	float PhaseMeter::calculate_phase(const std::vector<Vector2>& p_samples) const {
		if (p_samples.empty()) {
			return 0.0f;
		}

		double sum_l2 = 0.0;
		double sum_r2 = 0.0;
		double sum_lr = 0.0;
		for (const Vector2& sample : p_samples) {
			const double left = sample.x;
			const double right = sample.y;
			sum_l2 += left * left;
			sum_r2 += right * right;
			sum_lr += left * right;
		}

		const double denom = std::sqrt(std::max(1e-12, sum_l2 * sum_r2));
		if (denom <= 1e-9) {
			return 0.0f;
		}

		return AudioVisualizerBusUtils::clamp_float(static_cast<float>(sum_lr / denom), -1.0f, 1.0f);
	}

	void PhaseMeter::append_samples(const PackedVector2Array& p_frames) {
		const int frame_count = p_frames.size();
		if (frame_count <= 0) {
			return;
		}

		const Vector2* frame_ptr = p_frames.ptr();
		for (int i = 0; i < frame_count; i++) {
			live_samples.push_back(frame_ptr[i]);
		}

		const int overflow = static_cast<int>(live_samples.size()) - std::max(1, sample_count);
		if (overflow > 0) {
			live_samples.erase(live_samples.begin(), live_samples.begin() + overflow);
		}
	}

	Rect2 PhaseMeter::get_track_rect() const {
		const Vector2 size = get_size();
		const float pad = std::max(0.0f, content_padding);
		const float label_space = draw_labels ? static_cast<float>(label_font_size) + 4.0f : 0.0f;

		if (orientation == ORIENTATION_VERTICAL) {
			return Rect2(
				Vector2(pad, pad),
				Vector2(std::max(1.0f, size.x - pad * 2.0f - label_space), std::max(1.0f, size.y - pad * 2.0f)));
		}

		return Rect2(
			Vector2(pad, pad),
			Vector2(std::max(1.0f, size.x - pad * 2.0f), std::max(1.0f, size.y - pad * 2.0f - label_space)));
	}

	Color PhaseMeter::get_phase_color(float p_phase) const {
		const float value = AudioVisualizerBusUtils::clamp_float(p_phase, -1.0f, 1.0f);
		if (value < 0.0f) {
			return neutral_color.lerp(negative_color, -value);
		}

		return neutral_color.lerp(positive_color, value);
	}

	void PhaseMeter::draw_meter() {
		const Vector2 size = get_size();
		if (size.x <= 0.0f || size.y <= 0.0f) {
			return;
		}

		draw_rect(Rect2(Vector2(0.0f, 0.0f), size), background_color);

		const Rect2 track_rect = get_track_rect();
		if (track_rect.size.x <= 0.0f || track_rect.size.y <= 0.0f) {
			return;
		}

		if (!is_processing()) {
			update_meter(0.0f);
		}

		if (meter_style == METER_STYLE_SEGMENTS) {
			draw_segment_meter(track_rect);
		}
		else if (meter_style == METER_STYLE_GAUGE) {
			draw_gauge_meter(track_rect);
		}
		else if (orientation == ORIENTATION_VERTICAL) {
			draw_vertical_meter(track_rect);
		}
		else {
			draw_horizontal_meter(track_rect);
		}
	}

	void PhaseMeter::draw_horizontal_meter(const Rect2& p_rect) {
		const float center_x = p_rect.position.x + p_rect.size.x * 0.5f;
		const float value = AudioVisualizerBusUtils::clamp_float(displayed_phase, -1.0f, 1.0f);
		const float marker_x = p_rect.position.x + (value + 1.0f) * 0.5f * p_rect.size.x;

		draw_rect(p_rect, track_color);

		Color negative = negative_color;
		negative.a *= 0.32f;
		Color positive = positive_color;
		positive.a *= 0.32f;
		draw_rect(Rect2(p_rect.position, Vector2(p_rect.size.x * 0.5f, p_rect.size.y)), negative);
		draw_rect(Rect2(Vector2(center_x, p_rect.position.y), Vector2(p_rect.size.x * 0.5f, p_rect.size.y)), positive);

		Color fill = get_phase_color(value);
		fill.a *= 0.72f;
		if (value >= 0.0f) {
			draw_rect(Rect2(Vector2(center_x, p_rect.position.y), Vector2(std::max(0.0f, marker_x - center_x), p_rect.size.y)), fill);
		}
		else {
			draw_rect(Rect2(Vector2(marker_x, p_rect.position.y), Vector2(std::max(0.0f, center_x - marker_x), p_rect.size.y)), fill);
		}

		if (draw_ticks) {
			const int ticks = std::max(2, tick_count);
			for (int i = 0; i <= ticks; i++) {
				const float x = p_rect.position.x + static_cast<float>(i) / static_cast<float>(ticks) * p_rect.size.x;
				draw_line(Vector2(x, p_rect.position.y), Vector2(x, p_rect.position.y + p_rect.size.y), tick_color, i == ticks / 2 ? 2.0f : 1.0f);
			}
		}

		const float width = std::max(1.0f, marker_width);
		draw_rect(Rect2(Vector2(marker_x - width * 0.5f, p_rect.position.y), Vector2(width, p_rect.size.y)), marker_color);

		if (draw_labels) {
			draw_horizontal_labels(p_rect);
		}
	}

	void PhaseMeter::draw_vertical_meter(const Rect2& p_rect) {
		const float center_y = p_rect.position.y + p_rect.size.y * 0.5f;
		const float value = AudioVisualizerBusUtils::clamp_float(displayed_phase, -1.0f, 1.0f);
		const float marker_y = p_rect.position.y + (1.0f - (value + 1.0f) * 0.5f) * p_rect.size.y;

		draw_rect(p_rect, track_color);

		Color positive = positive_color;
		positive.a *= 0.32f;
		Color negative = negative_color;
		negative.a *= 0.32f;
		draw_rect(Rect2(p_rect.position, Vector2(p_rect.size.x, p_rect.size.y * 0.5f)), positive);
		draw_rect(Rect2(Vector2(p_rect.position.x, center_y), Vector2(p_rect.size.x, p_rect.size.y * 0.5f)), negative);

		Color fill = get_phase_color(value);
		fill.a *= 0.72f;
		if (value >= 0.0f) {
			draw_rect(Rect2(Vector2(p_rect.position.x, marker_y), Vector2(p_rect.size.x, std::max(0.0f, center_y - marker_y))), fill);
		}
		else {
			draw_rect(Rect2(Vector2(p_rect.position.x, center_y), Vector2(p_rect.size.x, std::max(0.0f, marker_y - center_y))), fill);
		}

		if (draw_ticks) {
			const int ticks = std::max(2, tick_count);
			for (int i = 0; i <= ticks; i++) {
				const float y = p_rect.position.y + static_cast<float>(i) / static_cast<float>(ticks) * p_rect.size.y;
				draw_line(Vector2(p_rect.position.x, y), Vector2(p_rect.position.x + p_rect.size.x, y), tick_color, i == ticks / 2 ? 2.0f : 1.0f);
			}
		}

		const float width = std::max(1.0f, marker_width);
		draw_rect(Rect2(Vector2(p_rect.position.x, marker_y - width * 0.5f), Vector2(p_rect.size.x, width)), marker_color);

		if (draw_labels) {
			draw_vertical_labels(p_rect);
		}
	}

	void PhaseMeter::draw_segment_meter(const Rect2& p_rect) {
		const float value = AudioVisualizerBusUtils::clamp_float(displayed_phase, -1.0f, 1.0f);
		const int count = std::max(3, segment_count);
		const float gap = std::max(0.0f, segment_gap);
		const bool vertical = orientation == ORIENTATION_VERTICAL;
		const float full_length = vertical ? p_rect.size.y : p_rect.size.x;
		const float cross_length = vertical ? p_rect.size.x : p_rect.size.y;
		const float cell_length = std::max(1.0f, (full_length - gap * static_cast<float>(count - 1)) / static_cast<float>(count));

		for (int i = 0; i < count; i++) {
			const float normalized = count > 1 ? static_cast<float>(i) / static_cast<float>(count - 1) : 0.0f;
			const float cell_phase = vertical ? 1.0f - normalized * 2.0f : normalized * 2.0f - 1.0f;
			const bool active = (value >= 0.0f && cell_phase >= -0.0001f && cell_phase <= value) || (value < 0.0f && cell_phase <= 0.0001f && cell_phase >= value);
			Color color = active ? get_phase_color(cell_phase) : track_color;
			if (!active) {
				color.a *= 0.62f;
			}

			const float offset = static_cast<float>(i) * (cell_length + gap);
			if (vertical) {
				draw_rect(Rect2(Vector2(p_rect.position.x, p_rect.position.y + offset), Vector2(cross_length, cell_length)), color);
			}
			else {
				draw_rect(Rect2(Vector2(p_rect.position.x + offset, p_rect.position.y), Vector2(cell_length, cross_length)), color);
			}
		}

		if (draw_ticks) {
			if (vertical) {
				const float center_y = p_rect.position.y + p_rect.size.y * 0.5f;
				draw_line(Vector2(p_rect.position.x, center_y), Vector2(p_rect.position.x + p_rect.size.x, center_y), tick_color, 2.0f);
			}
			else {
				const float center_x = p_rect.position.x + p_rect.size.x * 0.5f;
				draw_line(Vector2(center_x, p_rect.position.y), Vector2(center_x, p_rect.position.y + p_rect.size.y), tick_color, 2.0f);
			}
		}

		if (draw_labels) {
			if (vertical) {
				draw_vertical_labels(p_rect);
			}
			else {
				draw_horizontal_labels(p_rect);
			}
		}
	}

	void PhaseMeter::draw_gauge_meter(const Rect2& p_rect) {
		const float value = AudioVisualizerBusUtils::clamp_float(displayed_phase, -1.0f, 1.0f);
		const float radius = std::max(1.0f, std::min(p_rect.size.x * 0.46f, p_rect.size.y * 0.82f));
		const Vector2 center(p_rect.position.x + p_rect.size.x * 0.5f, p_rect.position.y + std::min(p_rect.size.y - 1.0f, radius + p_rect.size.y * 0.42f));
		const float arc_width = std::max(1.0f, gauge_arc_width);

		draw_gauge_arc(center, radius, -1.0f, -0.05f, negative_color, arc_width);
		draw_gauge_arc(center, radius, -0.05f, 0.05f, neutral_color, arc_width);
		draw_gauge_arc(center, radius, 0.05f, 1.0f, positive_color, arc_width);

		if (draw_ticks) {
			const int ticks = std::max(2, tick_count);
			for (int i = 0; i <= ticks; i++) {
				const float tick_phase = -1.0f + 2.0f * static_cast<float>(i) / static_cast<float>(ticks);
				const float angle = phase_to_gauge_angle(tick_phase);
				const Vector2 outer(center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius);
				const float inner_radius = radius - arc_width - (i == ticks / 2 ? 7.0f : 4.0f);
				const Vector2 inner(center.x + std::cos(angle) * inner_radius, center.y + std::sin(angle) * inner_radius);
				draw_line(inner, outer, tick_color, i == ticks / 2 ? 2.0f : 1.0f, true);
			}
		}

		const float needle_angle = phase_to_gauge_angle(value);
		const float needle_length = radius * AudioVisualizerBusUtils::clamp_float(gauge_needle_length, 0.1f, 1.2f);
		const Vector2 needle_end(center.x + std::cos(needle_angle) * needle_length, center.y + std::sin(needle_angle) * needle_length);
		draw_line(center, needle_end, marker_color, std::max(1.0f, marker_width), true);

		const float hub = std::max(0.0f, gauge_hub_radius);
		if (hub > 0.0f) {
			draw_circle(center, hub, marker_color);
			draw_circle(center, std::max(1.0f, hub * 0.45f), background_color);
		}

		if (!draw_labels) {
			return;
		}

		Ref<Font> font = get_theme_default_font();
		if (!font.is_valid()) {
			return;
		}

		const float font_size = static_cast<float>(std::max(1, label_font_size));
		const float label_width = std::max(18.0f, font_size * 3.0f);
		const float label_radius = radius + font_size * 0.85f;
		const float left_angle = phase_to_gauge_angle(-1.0f);
		const float zero_angle = phase_to_gauge_angle(0.0f);
		const float right_angle = phase_to_gauge_angle(1.0f);
		draw_string(font, Vector2(center.x + std::cos(left_angle) * label_radius - label_width * 0.5f, center.y + std::sin(left_angle) * label_radius + font_size * 0.35f), String("-1"), HORIZONTAL_ALIGNMENT_CENTER, label_width, label_font_size, label_color);
		draw_string(font, Vector2(center.x + std::cos(zero_angle) * label_radius - label_width * 0.5f, center.y + std::sin(zero_angle) * label_radius + font_size * 0.35f), String("0"), HORIZONTAL_ALIGNMENT_CENTER, label_width, label_font_size, label_color);
		draw_string(font, Vector2(center.x + std::cos(right_angle) * label_radius - label_width * 0.5f, center.y + std::sin(right_angle) * label_radius + font_size * 0.35f), String("+1"), HORIZONTAL_ALIGNMENT_CENTER, label_width, label_font_size, label_color);
	}

	void PhaseMeter::draw_gauge_arc(const Vector2& p_center, float p_radius, float p_from_value, float p_to_value, const Color& p_color, float p_width) {
		const int point_count = 24;
		PackedVector2Array points;
		points.resize(point_count);
		const float from_angle = phase_to_gauge_angle(p_from_value);
		const float to_angle = phase_to_gauge_angle(p_to_value);

		for (int i = 0; i < point_count; i++) {
			const float t = static_cast<float>(i) / static_cast<float>(point_count - 1);
			const float angle = from_angle + (to_angle - from_angle) * t;
			points.set(i, Vector2(p_center.x + std::cos(angle) * p_radius, p_center.y + std::sin(angle) * p_radius));
		}

		draw_polyline(points, p_color, p_width, true);
	}

	void PhaseMeter::draw_horizontal_labels(const Rect2& p_rect) {
		Ref<Font> font = get_theme_default_font();
		if (!font.is_valid()) {
			return;
		}

		const float font_size = static_cast<float>(std::max(1, label_font_size));
		const float y = std::min(get_size().y - 1.0f, p_rect.position.y + p_rect.size.y + font_size + 1.0f);
		const float label_width = std::max(18.0f, font_size * 3.0f);
		const float center_x = p_rect.position.x + p_rect.size.x * 0.5f;
		const float right_x = p_rect.position.x + p_rect.size.x - label_width;

		draw_string(font, Vector2(p_rect.position.x, y), String("-1"), HORIZONTAL_ALIGNMENT_LEFT, label_width, label_font_size, label_color);
		draw_string(font, Vector2(center_x - label_width * 0.5f, y), String("0"), HORIZONTAL_ALIGNMENT_CENTER, label_width, label_font_size, label_color);
		draw_string(font, Vector2(right_x, y), String("+1"), HORIZONTAL_ALIGNMENT_RIGHT, label_width, label_font_size, label_color);
	}

	void PhaseMeter::draw_vertical_labels(const Rect2& p_rect) {
		Ref<Font> font = get_theme_default_font();
		if (!font.is_valid()) {
			return;
		}

		const float label_x = p_rect.position.x + p_rect.size.x + 3.0f;
		const float label_width = std::max(1.0f, get_size().x - label_x - std::max(0.0f, content_padding));
		const float font_size = static_cast<float>(std::max(1, label_font_size));
		const float top_y = std::min(p_rect.position.y + font_size, p_rect.position.y + p_rect.size.y);
		const float center_y = p_rect.position.y + p_rect.size.y * 0.5f + font_size * 0.5f;
		const float bottom_y = p_rect.position.y + p_rect.size.y - 1.0f;

		draw_string(font, Vector2(label_x, top_y), String("+1"), HORIZONTAL_ALIGNMENT_LEFT, label_width, label_font_size, label_color);
		draw_string(font, Vector2(label_x, center_y), String("0"), HORIZONTAL_ALIGNMENT_LEFT, label_width, label_font_size, label_color);
		draw_string(font, Vector2(label_x, bottom_y), String("-1"), HORIZONTAL_ALIGNMENT_LEFT, label_width, label_font_size, label_color);
	}

	void PhaseMeter::set_use_bus(bool p_enabled) {
		if (use_bus == p_enabled) {
			return;
		}

		use_bus = p_enabled;
		live_samples.clear();
		update_processing();
		notify_property_list_changed();
		queue_redraw();
	}

	bool PhaseMeter::get_use_bus() const {
		return use_bus;
	}

	void PhaseMeter::set_bus(const StringName& p_bus) {
		if (bus == p_bus) {
			return;
		}

		bus = p_bus;
		live_samples.clear();
		queue_redraw();
	}

	StringName PhaseMeter::get_bus() const {
		return bus;
	}

	void PhaseMeter::set_bus_backend(int p_backend) {
		const int new_backend = std::min(std::max(static_cast<int>(AudioVisualizerBusUtils::BUS_BACKEND_GODOT), p_backend), static_cast<int>(AudioVisualizerBusUtils::BUS_BACKEND_FMOD_PLAYER));
		if (bus_backend == new_backend) {
			return;
		}

		bus_backend = new_backend;
		live_samples.clear();
		notify_property_list_changed();
		queue_redraw();
	}

	int PhaseMeter::get_bus_backend() const {
		return bus_backend;
	}

	void PhaseMeter::set_samples(const PackedVector2Array& p_samples) {
		samples = p_samples;
		notify_property_list_changed();
		queue_redraw();
	}

	PackedVector2Array PhaseMeter::get_samples() const {
		return samples;
	}

	void PhaseMeter::set_sample_count(int p_count) {
		const int new_count = std::max(16, p_count);
		if (sample_count == new_count) {
			return;
		}

		sample_count = new_count;
		live_samples.reserve(static_cast<size_t>(sample_count));
		if (static_cast<int>(live_samples.size()) > sample_count) {
			live_samples.erase(live_samples.begin(), live_samples.begin() + (static_cast<int>(live_samples.size()) - sample_count));
		}
		queue_redraw();
	}

	int PhaseMeter::get_sample_count() const {
		return sample_count;
	}

	void PhaseMeter::set_orientation(int p_orientation) {
		const int new_orientation = std::min(std::max(static_cast<int>(ORIENTATION_HORIZONTAL), p_orientation), static_cast<int>(ORIENTATION_VERTICAL));
		if (orientation == new_orientation) {
			return;
		}

		orientation = new_orientation;
		queue_redraw();
	}

	int PhaseMeter::get_orientation() const {
		return orientation;
	}

	void PhaseMeter::set_meter_style(int p_style) {
		const int new_style = std::min(std::max(static_cast<int>(METER_STYLE_BAR), p_style), static_cast<int>(METER_STYLE_GAUGE));
		if (meter_style == new_style) {
			return;
		}

		meter_style = new_style;
		notify_property_list_changed();
		queue_redraw();
	}

	int PhaseMeter::get_meter_style() const {
		return meter_style;
	}

	void PhaseMeter::set_phase(float p_phase) {
		phase = AudioVisualizerBusUtils::clamp_float(p_phase, -1.0f, 1.0f);
		if (!animation_enabled) {
			displayed_phase = phase;
		}
		queue_redraw();
	}

	float PhaseMeter::get_phase() const {
		return phase;
	}

	void PhaseMeter::set_smoothing(float p_smoothing) {
		smoothing = AudioVisualizerBusUtils::clamp_float(p_smoothing, 0.0f, 1.0f);
	}

	float PhaseMeter::get_smoothing() const {
		return smoothing;
	}

	void PhaseMeter::set_animation_enabled(bool p_enabled) {
		if (animation_enabled == p_enabled) {
			return;
		}

		animation_enabled = p_enabled;
		update_processing();
		queue_redraw();
	}

	bool PhaseMeter::get_animation_enabled() const {
		return animation_enabled;
	}

	void PhaseMeter::set_content_padding(float p_padding) {
		content_padding = std::max(0.0f, p_padding);
		queue_redraw();
	}

	float PhaseMeter::get_content_padding() const {
		return content_padding;
	}

	void PhaseMeter::set_marker_width(float p_width) {
		marker_width = std::max(1.0f, p_width);
		queue_redraw();
	}

	float PhaseMeter::get_marker_width() const {
		return marker_width;
	}

	void PhaseMeter::set_tick_count(int p_count) {
		tick_count = std::max(2, p_count);
		queue_redraw();
	}

	int PhaseMeter::get_tick_count() const {
		return tick_count;
	}

	void PhaseMeter::set_segment_count(int p_count) {
		segment_count = std::max(3, p_count);
		queue_redraw();
	}

	int PhaseMeter::get_segment_count() const {
		return segment_count;
	}

	void PhaseMeter::set_segment_gap(float p_gap) {
		segment_gap = std::max(0.0f, p_gap);
		queue_redraw();
	}

	float PhaseMeter::get_segment_gap() const {
		return segment_gap;
	}

	void PhaseMeter::set_gauge_arc_width(float p_width) {
		gauge_arc_width = std::max(1.0f, p_width);
		queue_redraw();
	}

	float PhaseMeter::get_gauge_arc_width() const {
		return gauge_arc_width;
	}

	void PhaseMeter::set_gauge_needle_length(float p_length) {
		gauge_needle_length = AudioVisualizerBusUtils::clamp_float(p_length, 0.1f, 1.2f);
		queue_redraw();
	}

	float PhaseMeter::get_gauge_needle_length() const {
		return gauge_needle_length;
	}

	void PhaseMeter::set_gauge_hub_radius(float p_radius) {
		gauge_hub_radius = std::max(0.0f, p_radius);
		queue_redraw();
	}

	float PhaseMeter::get_gauge_hub_radius() const {
		return gauge_hub_radius;
	}

	void PhaseMeter::set_label_font_size(int p_size) {
		label_font_size = std::max(1, p_size);
		queue_redraw();
	}

	int PhaseMeter::get_label_font_size() const {
		return label_font_size;
	}

	void PhaseMeter::set_draw_ticks(bool p_enabled) {
		if (draw_ticks == p_enabled) {
			return;
		}

		draw_ticks = p_enabled;
		notify_property_list_changed();
		queue_redraw();
	}

	bool PhaseMeter::get_draw_ticks() const {
		return draw_ticks;
	}

	void PhaseMeter::set_draw_labels(bool p_enabled) {
		if (draw_labels == p_enabled) {
			return;
		}

		draw_labels = p_enabled;
		notify_property_list_changed();
		queue_redraw();
	}

	bool PhaseMeter::get_draw_labels() const {
		return draw_labels;
	}

	void PhaseMeter::set_background_color(const Color& p_color) {
		background_color = p_color;
		queue_redraw();
	}

	Color PhaseMeter::get_background_color() const {
		return background_color;
	}

	void PhaseMeter::set_track_color(const Color& p_color) {
		track_color = p_color;
		queue_redraw();
	}

	Color PhaseMeter::get_track_color() const {
		return track_color;
	}

	void PhaseMeter::set_negative_color(const Color& p_color) {
		negative_color = p_color;
		queue_redraw();
	}

	Color PhaseMeter::get_negative_color() const {
		return negative_color;
	}

	void PhaseMeter::set_neutral_color(const Color& p_color) {
		neutral_color = p_color;
		queue_redraw();
	}

	Color PhaseMeter::get_neutral_color() const {
		return neutral_color;
	}

	void PhaseMeter::set_positive_color(const Color& p_color) {
		positive_color = p_color;
		queue_redraw();
	}

	Color PhaseMeter::get_positive_color() const {
		return positive_color;
	}

	void PhaseMeter::set_marker_color(const Color& p_color) {
		marker_color = p_color;
		queue_redraw();
	}

	Color PhaseMeter::get_marker_color() const {
		return marker_color;
	}

	void PhaseMeter::set_tick_color(const Color& p_color) {
		tick_color = p_color;
		queue_redraw();
	}

	Color PhaseMeter::get_tick_color() const {
		return tick_color;
	}

	void PhaseMeter::set_label_color(const Color& p_color) {
		label_color = p_color;
		queue_redraw();
	}

	Color PhaseMeter::get_label_color() const {
		return label_color;
	}
}
