#include "oscilloscope_visualizer.h"

#include "audio_visualizer_bus_utils.h"

#include <godot_cpp/classes/audio_effect_capture.hpp>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>

#include <algorithm>
#include <cmath>

namespace godot {
	namespace {
		Vector2 clamp_sample(const Vector2& p_sample, float p_gain) {
			const float gain = std::max(0.0f, p_gain);
			return Vector2(
				AudioVisualizerBusUtils::clamp_float(p_sample.x * gain, -1.0f, 1.0f),
				AudioVisualizerBusUtils::clamp_float(p_sample.y * gain, -1.0f, 1.0f));
		}
	}

	void OscilloscopeVisualizer::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_use_bus", "enabled"), &OscilloscopeVisualizer::set_use_bus);
		ClassDB::bind_method(D_METHOD("get_use_bus"), &OscilloscopeVisualizer::get_use_bus);
		ClassDB::bind_method(D_METHOD("set_bus", "bus"), &OscilloscopeVisualizer::set_bus);
		ClassDB::bind_method(D_METHOD("get_bus"), &OscilloscopeVisualizer::get_bus);
		ClassDB::bind_method(D_METHOD("set_samples", "samples"), &OscilloscopeVisualizer::set_samples);
		ClassDB::bind_method(D_METHOD("get_samples"), &OscilloscopeVisualizer::get_samples);
		ClassDB::bind_method(D_METHOD("set_sample_count", "count"), &OscilloscopeVisualizer::set_sample_count);
		ClassDB::bind_method(D_METHOD("get_sample_count"), &OscilloscopeVisualizer::get_sample_count);
		ClassDB::bind_method(D_METHOD("set_display_mode", "mode"), &OscilloscopeVisualizer::set_display_mode);
		ClassDB::bind_method(D_METHOD("get_display_mode"), &OscilloscopeVisualizer::get_display_mode);
		ClassDB::bind_method(D_METHOD("set_frozen", "enabled"), &OscilloscopeVisualizer::set_frozen);
		ClassDB::bind_method(D_METHOD("get_frozen"), &OscilloscopeVisualizer::get_frozen);
		ClassDB::bind_method(D_METHOD("set_draw_grid", "enabled"), &OscilloscopeVisualizer::set_draw_grid);
		ClassDB::bind_method(D_METHOD("get_draw_grid"), &OscilloscopeVisualizer::get_draw_grid);
		ClassDB::bind_method(D_METHOD("set_draw_center", "enabled"), &OscilloscopeVisualizer::set_draw_center);
		ClassDB::bind_method(D_METHOD("get_draw_center"), &OscilloscopeVisualizer::get_draw_center);
		ClassDB::bind_method(D_METHOD("set_gain", "gain"), &OscilloscopeVisualizer::set_gain);
		ClassDB::bind_method(D_METHOD("get_gain"), &OscilloscopeVisualizer::get_gain);
		ClassDB::bind_method(D_METHOD("set_line_width", "width"), &OscilloscopeVisualizer::set_line_width);
		ClassDB::bind_method(D_METHOD("get_line_width"), &OscilloscopeVisualizer::get_line_width);
		ClassDB::bind_method(D_METHOD("set_point_size", "size"), &OscilloscopeVisualizer::set_point_size);
		ClassDB::bind_method(D_METHOD("get_point_size"), &OscilloscopeVisualizer::get_point_size);
		ClassDB::bind_method(D_METHOD("set_padding", "padding"), &OscilloscopeVisualizer::set_padding);
		ClassDB::bind_method(D_METHOD("get_padding"), &OscilloscopeVisualizer::get_padding);
		ClassDB::bind_method(D_METHOD("set_stereo_gap", "gap"), &OscilloscopeVisualizer::set_stereo_gap);
		ClassDB::bind_method(D_METHOD("get_stereo_gap"), &OscilloscopeVisualizer::get_stereo_gap);
		ClassDB::bind_method(D_METHOD("set_background_color", "color"), &OscilloscopeVisualizer::set_background_color);
		ClassDB::bind_method(D_METHOD("get_background_color"), &OscilloscopeVisualizer::get_background_color);
		ClassDB::bind_method(D_METHOD("set_grid_color", "color"), &OscilloscopeVisualizer::set_grid_color);
		ClassDB::bind_method(D_METHOD("get_grid_color"), &OscilloscopeVisualizer::get_grid_color);
		ClassDB::bind_method(D_METHOD("set_center_color", "color"), &OscilloscopeVisualizer::set_center_color);
		ClassDB::bind_method(D_METHOD("get_center_color"), &OscilloscopeVisualizer::get_center_color);
		ClassDB::bind_method(D_METHOD("set_left_color", "color"), &OscilloscopeVisualizer::set_left_color);
		ClassDB::bind_method(D_METHOD("get_left_color"), &OscilloscopeVisualizer::get_left_color);
		ClassDB::bind_method(D_METHOD("set_right_color", "color"), &OscilloscopeVisualizer::set_right_color);
		ClassDB::bind_method(D_METHOD("get_right_color"), &OscilloscopeVisualizer::get_right_color);
		ClassDB::bind_method(D_METHOD("set_xy_color", "color"), &OscilloscopeVisualizer::set_xy_color);
		ClassDB::bind_method(D_METHOD("get_xy_color"), &OscilloscopeVisualizer::get_xy_color);

		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_bus"), "set_use_bus", "get_use_bus");
		ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "bus", PROPERTY_HINT_ENUM_SUGGESTION, "Master"), "set_bus", "get_bus");
		ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR2_ARRAY, "samples"), "set_samples", "get_samples");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_count", PROPERTY_HINT_RANGE, "16,8192,1"), "set_sample_count", "get_sample_count");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "display_mode", PROPERTY_HINT_ENUM, "Waveform,XY,Lissajous"), "set_display_mode", "get_display_mode");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "frozen"), "set_frozen", "get_frozen");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "draw_grid"), "set_draw_grid", "get_draw_grid");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "draw_center"), "set_draw_center", "get_draw_center");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gain", PROPERTY_HINT_RANGE, "0.0,8.0,0.01"), "set_gain", "get_gain");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "line_width", PROPERTY_HINT_RANGE, "1.0,16.0,0.5"), "set_line_width", "get_line_width");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "point_size", PROPERTY_HINT_RANGE, "1.0,12.0,0.5"), "set_point_size", "get_point_size");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "padding", PROPERTY_HINT_RANGE, "0.0,64.0,0.5"), "set_padding", "get_padding");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "stereo_gap", PROPERTY_HINT_RANGE, "0.0,64.0,0.5"), "set_stereo_gap", "get_stereo_gap");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "background_color"), "set_background_color", "get_background_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "grid_color"), "set_grid_color", "get_grid_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "center_color"), "set_center_color", "get_center_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "left_color"), "set_left_color", "get_left_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "right_color"), "set_right_color", "get_right_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "xy_color"), "set_xy_color", "get_xy_color");
	}

	OscilloscopeVisualizer::OscilloscopeVisualizer() {
		set_clip_contents(true);
		set_process(true);
		live_samples.reserve((size_t)sample_count);
	}

	OscilloscopeVisualizer::~OscilloscopeVisualizer() {
	}

	void OscilloscopeVisualizer::_notification(int p_what) {
		if (p_what == NOTIFICATION_PROCESS) {
			if (!frozen) {
				update_samples();
			}
			queue_redraw();
			return;
		}

		if (p_what != NOTIFICATION_DRAW) {
			return;
		}

		const Vector2 size = get_size();
		if (size.x <= 0.0f || size.y <= 0.0f) {
			return;
		}

		draw_rect(Rect2(Vector2(0.0f, 0.0f), size), background_color);

		const Rect2 canvas = get_canvas_rect();
		if (draw_grid || draw_center) {
			draw_scope_grid(canvas);
		}

		if (display_mode == DISPLAY_MODE_WAVEFORM) {
			draw_waveform(canvas);
		}
		else {
			draw_xy(canvas, display_mode == DISPLAY_MODE_LISSAJOUS);
		}
	}

	void OscilloscopeVisualizer::_validate_property(PropertyInfo& p_property) const {
		const String name = p_property.name;
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

	void OscilloscopeVisualizer::update_samples() {
		bool found_capture = false;
		if (use_bus) {
			PackedVector2Array frames;
			if (AudioVisualizerBusUtils::get_capture_frames(bus, std::max(1, sample_count), frames)) {
				append_samples(frames);
				found_capture = true;
			}
		}

		if (!use_bus || !found_capture) {
			live_samples.clear();
			const int source_count = samples.size();
			const Vector2* source = samples.ptr();
			const int start = std::max(0, source_count - sample_count);
			for (int i = start; i < source_count; i++) {
				live_samples.push_back(source[i]);
			}
		}
	}

	void OscilloscopeVisualizer::append_samples(const PackedVector2Array& p_frames) {
		const int frame_count = p_frames.size();
		if (frame_count <= 0) {
			return;
		}

		const Vector2* frame_ptr = p_frames.ptr();
		for (int i = 0; i < frame_count; i++) {
			live_samples.push_back(frame_ptr[i]);
		}

		const int overflow = (int)live_samples.size() - std::max(1, sample_count);
		if (overflow > 0) {
			live_samples.erase(live_samples.begin(), live_samples.begin() + overflow);
		}
	}

	const std::vector<Vector2>& OscilloscopeVisualizer::get_display_samples() const {
		return live_samples;
	}

	Rect2 OscilloscopeVisualizer::get_canvas_rect() const {
		const Vector2 size = get_size();
		const float pad = std::max(0.0f, padding);
		return Rect2(Vector2(pad, pad), Vector2(std::max(1.0f, size.x - pad * 2.0f), std::max(1.0f, size.y - pad * 2.0f)));
	}

	void OscilloscopeVisualizer::draw_scope_grid(const Rect2& p_rect) {
		if (draw_grid) {
			const int steps = 4;
			for (int i = 0; i <= steps; i++) {
				const float x = p_rect.position.x + p_rect.size.x * (float)i / (float)steps;
				const float y = p_rect.position.y + p_rect.size.y * (float)i / (float)steps;
				draw_line(Vector2(x, p_rect.position.y), Vector2(x, p_rect.position.y + p_rect.size.y), grid_color, 1.0f);
				draw_line(Vector2(p_rect.position.x, y), Vector2(p_rect.position.x + p_rect.size.x, y), grid_color, 1.0f);
			}
		}

		if (draw_center) {
			const float cx = p_rect.position.x + p_rect.size.x * 0.5f;
			const float cy = p_rect.position.y + p_rect.size.y * 0.5f;
			draw_line(Vector2(cx, p_rect.position.y), Vector2(cx, p_rect.position.y + p_rect.size.y), center_color, 1.0f);
			draw_line(Vector2(p_rect.position.x, cy), Vector2(p_rect.position.x + p_rect.size.x, cy), center_color, 1.0f);
		}
	}

	void OscilloscopeVisualizer::draw_waveform(const Rect2& p_rect) {
		const std::vector<Vector2>& display_samples = get_display_samples();
		const int count = (int)display_samples.size();
		if (count <= 1) {
			return;
		}

		const float gap = std::min(std::max(0.0f, stereo_gap), std::max(0.0f, p_rect.size.y * 0.25f));
		const float lane_height = (p_rect.size.y - gap) * 0.5f;
		const float left_center = p_rect.position.y + lane_height * 0.5f;
		const float right_center = p_rect.position.y + lane_height + gap + lane_height * 0.5f;
		const float half_height = std::max(1.0f, lane_height * 0.45f);

		PackedVector2Array left_points;
		PackedVector2Array right_points;
		left_points.resize(count);
		right_points.resize(count);

		for (int i = 0; i < count; i++) {
			const float x = p_rect.position.x + (float)i / (float)(count - 1) * p_rect.size.x;
			const Vector2 sample = clamp_sample(display_samples[(size_t)i], gain);
			left_points.set(i, Vector2(x, left_center - sample.x * half_height));
			right_points.set(i, Vector2(x, right_center - sample.y * half_height));
		}

		draw_polyline(left_points, left_color, line_width, true);
		draw_polyline(right_points, right_color, line_width, true);
	}

	void OscilloscopeVisualizer::draw_xy(const Rect2& p_rect, bool p_lissajous) {
		const std::vector<Vector2>& display_samples = get_display_samples();
		const int count = (int)display_samples.size();
		if (count <= 1) {
			return;
		}

		const float cx = p_rect.position.x + p_rect.size.x * 0.5f;
		const float cy = p_rect.position.y + p_rect.size.y * 0.5f;
		const float radius = std::max(1.0f, std::min(p_rect.size.x, p_rect.size.y) * 0.46f);
		const int stride = std::max(1, count / 1024);
		const float point = std::max(1.0f, point_size);

		for (int i = 0; i < count; i += stride) {
			const Vector2 sample = clamp_sample(display_samples[(size_t)i], gain);
			float x_value = sample.x;
			float y_value = sample.y;

			if (p_lissajous) {
				x_value = (sample.x - sample.y) * 0.70710678f;
				y_value = (sample.x + sample.y) * 0.70710678f;
			}

			const float x = cx + x_value * radius;
			const float y = cy - y_value * radius;
			const float age = count > 1 ? (float)i / (float)(count - 1) : 1.0f;
			Color color = xy_color;
			color.a *= 0.25f + 0.75f * age;
			draw_rect(Rect2(Vector2(x - point * 0.5f, y - point * 0.5f), Vector2(point, point)), color);
		}
	}

	void OscilloscopeVisualizer::set_use_bus(bool p_enabled) {
		use_bus = p_enabled;
		queue_redraw();
	}

	bool OscilloscopeVisualizer::get_use_bus() const {
		return use_bus;
	}

	void OscilloscopeVisualizer::set_bus(const StringName& p_bus) {
		bus = p_bus;
		queue_redraw();
	}

	StringName OscilloscopeVisualizer::get_bus() const {
		return bus;
	}

	void OscilloscopeVisualizer::set_samples(const PackedVector2Array& p_samples) {
		samples = p_samples;
		update_samples();
		queue_redraw();
	}

	PackedVector2Array OscilloscopeVisualizer::get_samples() const {
		return samples;
	}

	void OscilloscopeVisualizer::set_sample_count(int p_count) {
		sample_count = std::max(16, p_count);
		live_samples.reserve((size_t)sample_count);
		if ((int)live_samples.size() > sample_count) {
			live_samples.erase(live_samples.begin(), live_samples.begin() + ((int)live_samples.size() - sample_count));
		}
		queue_redraw();
	}

	int OscilloscopeVisualizer::get_sample_count() const {
		return sample_count;
	}

	void OscilloscopeVisualizer::set_display_mode(int p_mode) {
		display_mode = std::max(0, std::min(2, p_mode));
		queue_redraw();
	}

	int OscilloscopeVisualizer::get_display_mode() const {
		return display_mode;
	}

	void OscilloscopeVisualizer::set_frozen(bool p_enabled) {
		frozen = p_enabled;
		queue_redraw();
	}

	bool OscilloscopeVisualizer::get_frozen() const {
		return frozen;
	}

	void OscilloscopeVisualizer::set_draw_grid(bool p_enabled) {
		draw_grid = p_enabled;
		queue_redraw();
	}

	bool OscilloscopeVisualizer::get_draw_grid() const {
		return draw_grid;
	}

	void OscilloscopeVisualizer::set_draw_center(bool p_enabled) {
		draw_center = p_enabled;
		queue_redraw();
	}

	bool OscilloscopeVisualizer::get_draw_center() const {
		return draw_center;
	}

	void OscilloscopeVisualizer::set_gain(float p_gain) {
		gain = std::max(0.0f, p_gain);
		queue_redraw();
	}

	float OscilloscopeVisualizer::get_gain() const {
		return gain;
	}

	void OscilloscopeVisualizer::set_line_width(float p_width) {
		line_width = std::max(1.0f, p_width);
		queue_redraw();
	}

	float OscilloscopeVisualizer::get_line_width() const {
		return line_width;
	}

	void OscilloscopeVisualizer::set_point_size(float p_size) {
		point_size = std::max(1.0f, p_size);
		queue_redraw();
	}

	float OscilloscopeVisualizer::get_point_size() const {
		return point_size;
	}

	void OscilloscopeVisualizer::set_padding(float p_padding) {
		padding = std::max(0.0f, p_padding);
		queue_redraw();
	}

	float OscilloscopeVisualizer::get_padding() const {
		return padding;
	}

	void OscilloscopeVisualizer::set_stereo_gap(float p_gap) {
		stereo_gap = std::max(0.0f, p_gap);
		queue_redraw();
	}

	float OscilloscopeVisualizer::get_stereo_gap() const {
		return stereo_gap;
	}

	void OscilloscopeVisualizer::set_background_color(const Color& p_color) {
		background_color = p_color;
		queue_redraw();
	}

	Color OscilloscopeVisualizer::get_background_color() const {
		return background_color;
	}

	void OscilloscopeVisualizer::set_grid_color(const Color& p_color) {
		grid_color = p_color;
		queue_redraw();
	}

	Color OscilloscopeVisualizer::get_grid_color() const {
		return grid_color;
	}

	void OscilloscopeVisualizer::set_center_color(const Color& p_color) {
		center_color = p_color;
		queue_redraw();
	}

	Color OscilloscopeVisualizer::get_center_color() const {
		return center_color;
	}

	void OscilloscopeVisualizer::set_left_color(const Color& p_color) {
		left_color = p_color;
		queue_redraw();
	}

	Color OscilloscopeVisualizer::get_left_color() const {
		return left_color;
	}

	void OscilloscopeVisualizer::set_right_color(const Color& p_color) {
		right_color = p_color;
		queue_redraw();
	}

	Color OscilloscopeVisualizer::get_right_color() const {
		return right_color;
	}

	void OscilloscopeVisualizer::set_xy_color(const Color& p_color) {
		xy_color = p_color;
		queue_redraw();
	}

	Color OscilloscopeVisualizer::get_xy_color() const {
		return xy_color;
	}
}
