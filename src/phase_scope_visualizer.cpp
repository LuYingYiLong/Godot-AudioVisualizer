#include "phase_scope_visualizer.h"

#include "audio_visualizer_bus_utils.h"

#include <godot_cpp/classes/audio_effect_capture.hpp>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <algorithm>
#include <cmath>

namespace godot {
	namespace {
		Vector2 sanitize_sample(const Vector2& p_sample, float p_gain) {
			const float gain = std::max(0.0f, p_gain);
			return Vector2(
				AudioVisualizerBusUtils::clamp_float(p_sample.x * gain, -1.0f, 1.0f),
				AudioVisualizerBusUtils::clamp_float(p_sample.y * gain, -1.0f, 1.0f));
		}

		float mix_float(float p_from, float p_to, float p_weight) {
			return p_from + (p_to - p_from) * AudioVisualizerBusUtils::clamp_float(p_weight, 0.0f, 1.0f);
		}

		Color correlation_color(float p_value, const Color& p_negative, const Color& p_neutral, const Color& p_positive) {
			if (p_value < 0.0f) {
				return p_neutral.lerp(p_negative, -p_value);
			}

			return p_neutral.lerp(p_positive, p_value);
		}
	}

	void PhaseScopeVisualizer::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_use_bus", "enabled"), &PhaseScopeVisualizer::set_use_bus);
		ClassDB::bind_method(D_METHOD("get_use_bus"), &PhaseScopeVisualizer::get_use_bus);
		ClassDB::bind_method(D_METHOD("set_bus", "bus"), &PhaseScopeVisualizer::set_bus);
		ClassDB::bind_method(D_METHOD("get_bus"), &PhaseScopeVisualizer::get_bus);
		ClassDB::bind_method(D_METHOD("set_samples", "samples"), &PhaseScopeVisualizer::set_samples);
		ClassDB::bind_method(D_METHOD("get_samples"), &PhaseScopeVisualizer::get_samples);
		ClassDB::bind_method(D_METHOD("set_sample_count", "count"), &PhaseScopeVisualizer::set_sample_count);
		ClassDB::bind_method(D_METHOD("get_sample_count"), &PhaseScopeVisualizer::get_sample_count);
		ClassDB::bind_method(D_METHOD("set_frozen", "enabled"), &PhaseScopeVisualizer::set_frozen);
		ClassDB::bind_method(D_METHOD("get_frozen"), &PhaseScopeVisualizer::get_frozen);
		ClassDB::bind_method(D_METHOD("set_draw_grid", "enabled"), &PhaseScopeVisualizer::set_draw_grid);
		ClassDB::bind_method(D_METHOD("get_draw_grid"), &PhaseScopeVisualizer::get_draw_grid);
		ClassDB::bind_method(D_METHOD("set_draw_meter", "enabled"), &PhaseScopeVisualizer::set_draw_meter);
		ClassDB::bind_method(D_METHOD("get_draw_meter"), &PhaseScopeVisualizer::get_draw_meter);
		ClassDB::bind_method(D_METHOD("set_gain", "gain"), &PhaseScopeVisualizer::set_gain);
		ClassDB::bind_method(D_METHOD("get_gain"), &PhaseScopeVisualizer::get_gain);
		ClassDB::bind_method(D_METHOD("set_point_size", "size"), &PhaseScopeVisualizer::set_point_size);
		ClassDB::bind_method(D_METHOD("get_point_size"), &PhaseScopeVisualizer::get_point_size);
		ClassDB::bind_method(D_METHOD("set_padding", "padding"), &PhaseScopeVisualizer::set_padding);
		ClassDB::bind_method(D_METHOD("get_padding"), &PhaseScopeVisualizer::get_padding);
		ClassDB::bind_method(D_METHOD("set_meter_width", "width"), &PhaseScopeVisualizer::set_meter_width);
		ClassDB::bind_method(D_METHOD("get_meter_width"), &PhaseScopeVisualizer::get_meter_width);
		ClassDB::bind_method(D_METHOD("set_smoothing", "smoothing"), &PhaseScopeVisualizer::set_smoothing);
		ClassDB::bind_method(D_METHOD("get_smoothing"), &PhaseScopeVisualizer::get_smoothing);
		ClassDB::bind_method(D_METHOD("set_background_color", "color"), &PhaseScopeVisualizer::set_background_color);
		ClassDB::bind_method(D_METHOD("get_background_color"), &PhaseScopeVisualizer::get_background_color);
		ClassDB::bind_method(D_METHOD("set_grid_color", "color"), &PhaseScopeVisualizer::set_grid_color);
		ClassDB::bind_method(D_METHOD("get_grid_color"), &PhaseScopeVisualizer::get_grid_color);
		ClassDB::bind_method(D_METHOD("set_point_color", "color"), &PhaseScopeVisualizer::set_point_color);
		ClassDB::bind_method(D_METHOD("get_point_color"), &PhaseScopeVisualizer::get_point_color);
		ClassDB::bind_method(D_METHOD("set_meter_negative_color", "color"), &PhaseScopeVisualizer::set_meter_negative_color);
		ClassDB::bind_method(D_METHOD("get_meter_negative_color"), &PhaseScopeVisualizer::get_meter_negative_color);
		ClassDB::bind_method(D_METHOD("set_meter_positive_color", "color"), &PhaseScopeVisualizer::set_meter_positive_color);
		ClassDB::bind_method(D_METHOD("get_meter_positive_color"), &PhaseScopeVisualizer::get_meter_positive_color);
		ClassDB::bind_method(D_METHOD("set_meter_neutral_color", "color"), &PhaseScopeVisualizer::set_meter_neutral_color);
		ClassDB::bind_method(D_METHOD("get_meter_neutral_color"), &PhaseScopeVisualizer::get_meter_neutral_color);
		ClassDB::bind_method(D_METHOD("get_correlation"), &PhaseScopeVisualizer::get_correlation);
		ClassDB::bind_method(D_METHOD("get_width"), &PhaseScopeVisualizer::get_width);
		ClassDB::bind_method(D_METHOD("get_balance"), &PhaseScopeVisualizer::get_balance);

		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_bus"), "set_use_bus", "get_use_bus");
		ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "bus", PROPERTY_HINT_ENUM_SUGGESTION, "Master"), "set_bus", "get_bus");
		ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR2_ARRAY, "samples"), "set_samples", "get_samples");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_count", PROPERTY_HINT_RANGE, "16,8192,1"), "set_sample_count", "get_sample_count");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "frozen"), "set_frozen", "get_frozen");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "draw_grid"), "set_draw_grid", "get_draw_grid");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "draw_meter"), "set_draw_meter", "get_draw_meter");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gain", PROPERTY_HINT_RANGE, "0.0,8.0,0.01"), "set_gain", "get_gain");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "point_size", PROPERTY_HINT_RANGE, "1.0,12.0,0.5"), "set_point_size", "get_point_size");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "padding", PROPERTY_HINT_RANGE, "0.0,64.0,0.5"), "set_padding", "get_padding");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "meter_width", PROPERTY_HINT_RANGE, "0.0,160.0,1.0"), "set_meter_width", "get_meter_width");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "smoothing", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_smoothing", "get_smoothing");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "background_color"), "set_background_color", "get_background_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "grid_color"), "set_grid_color", "get_grid_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "point_color"), "set_point_color", "get_point_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "meter_negative_color"), "set_meter_negative_color", "get_meter_negative_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "meter_positive_color"), "set_meter_positive_color", "get_meter_positive_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "meter_neutral_color"), "set_meter_neutral_color", "get_meter_neutral_color");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "correlation", PROPERTY_HINT_RANGE, "-1.0,1.0,0.001", PROPERTY_USAGE_READ_ONLY), "", "get_correlation");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "width", PROPERTY_HINT_RANGE, "0.0,1.0,0.001", PROPERTY_USAGE_READ_ONLY), "", "get_width");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "balance", PROPERTY_HINT_RANGE, "-1.0,1.0,0.001", PROPERTY_USAGE_READ_ONLY), "", "get_balance");
	}

	PhaseScopeVisualizer::PhaseScopeVisualizer() {
		set_clip_contents(true);
		set_process(true);
		live_samples.reserve((size_t)sample_count);
	}

	PhaseScopeVisualizer::~PhaseScopeVisualizer() {
	}

	void PhaseScopeVisualizer::_notification(int p_what) {
		if (p_what == NOTIFICATION_PROCESS) {
			if (!frozen) {
				update_samples();
				update_phase_metrics();
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

		const Rect2 scope_rect = get_scope_rect();
		if (draw_grid) {
			draw_scope_grid(scope_rect);
		}
		draw_goniometer(scope_rect);

		if (draw_meter) {
			draw_correlation_meter(get_meter_rect());
		}
	}

	void PhaseScopeVisualizer::_validate_property(PropertyInfo& p_property) const {
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

	void PhaseScopeVisualizer::update_samples() {
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

	void PhaseScopeVisualizer::append_samples(const PackedVector2Array& p_frames) {
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

	void PhaseScopeVisualizer::update_phase_metrics() {
		if (live_samples.empty()) {
			correlation = mix_float(correlation, 0.0f, smoothing);
			width = mix_float(width, 0.0f, smoothing);
			balance = mix_float(balance, 0.0f, smoothing);
			return;
		}

		double sum_l2 = 0.0;
		double sum_r2 = 0.0;
		double sum_lr = 0.0;
		double sum_m2 = 0.0;
		double sum_s2 = 0.0;

		for (const Vector2& raw_sample : live_samples) {
			const Vector2 sample = sanitize_sample(raw_sample, gain);
			const double l = sample.x;
			const double r = sample.y;
			const double m = (l + r) * 0.7071067811865475;
			const double s = (l - r) * 0.7071067811865475;
			sum_l2 += l * l;
			sum_r2 += r * r;
			sum_lr += l * r;
			sum_m2 += m * m;
			sum_s2 += s * s;
		}

		const double denom = std::sqrt(std::max(1e-12, sum_l2 * sum_r2));
		const float next_corr = AudioVisualizerBusUtils::clamp_float((float)(sum_lr / denom), -1.0f, 1.0f);
		const float next_width = AudioVisualizerBusUtils::clamp_float((float)std::sqrt(sum_s2 / std::max(1e-12, sum_m2 + sum_s2)), 0.0f, 1.0f);
		const float next_balance = AudioVisualizerBusUtils::clamp_float((float)((sum_l2 - sum_r2) / std::max(1e-12, sum_l2 + sum_r2)), -1.0f, 1.0f);

		correlation = mix_float(correlation, next_corr, smoothing);
		width = mix_float(width, next_width, smoothing);
		balance = mix_float(balance, next_balance, smoothing);
	}

	Rect2 PhaseScopeVisualizer::get_scope_rect() const {
		const Vector2 size = get_size();
		const float pad = std::max(0.0f, padding);
		const float meter = draw_meter ? std::max(0.0f, meter_width) : 0.0f;
		const float gap = draw_meter && meter > 0.0f ? 6.0f : 0.0f;
		return Rect2(Vector2(pad, pad), Vector2(std::max(1.0f, size.x - pad * 2.0f - meter - gap), std::max(1.0f, size.y - pad * 2.0f)));
	}

	Rect2 PhaseScopeVisualizer::get_meter_rect() const {
		const Vector2 size = get_size();
		const float pad = std::max(0.0f, padding);
		const float width_value = std::max(1.0f, meter_width);
		return Rect2(Vector2(std::max(pad, size.x - pad - width_value), pad), Vector2(width_value, std::max(1.0f, size.y - pad * 2.0f)));
	}

	void PhaseScopeVisualizer::draw_scope_grid(const Rect2& p_rect) {
		const float cx = p_rect.position.x + p_rect.size.x * 0.5f;
		const float cy = p_rect.position.y + p_rect.size.y * 0.5f;
		const float radius = std::min(p_rect.size.x, p_rect.size.y) * 0.46f;

		draw_line(Vector2(cx - radius, cy), Vector2(cx + radius, cy), grid_color, 1.0f);
		draw_line(Vector2(cx, cy - radius), Vector2(cx, cy + radius), grid_color, 1.0f);
		draw_line(Vector2(cx, cy - radius), Vector2(cx + radius, cy), grid_color, 1.0f);
		draw_line(Vector2(cx + radius, cy), Vector2(cx, cy + radius), grid_color, 1.0f);
		draw_line(Vector2(cx, cy + radius), Vector2(cx - radius, cy), grid_color, 1.0f);
		draw_line(Vector2(cx - radius, cy), Vector2(cx, cy - radius), grid_color, 1.0f);
	}

	void PhaseScopeVisualizer::draw_goniometer(const Rect2& p_rect) {
		if (live_samples.empty()) {
			return;
		}

		const float cx = p_rect.position.x + p_rect.size.x * 0.5f;
		const float cy = p_rect.position.y + p_rect.size.y * 0.5f;
		const float radius = std::max(1.0f, std::min(p_rect.size.x, p_rect.size.y) * 0.46f);
		const int count = (int)live_samples.size();
		const int stride = std::max(1, count / 1200);
		const float point = std::max(1.0f, point_size);

		for (int i = 0; i < count; i += stride) {
			const Vector2 sample = sanitize_sample(live_samples[(size_t)i], gain);
			const float mid = (sample.x + sample.y) * 0.70710678f;
			const float side = (sample.x - sample.y) * 0.70710678f;
			const float x = cx + side * radius;
			const float y = cy - mid * radius;
			const float age = count > 1 ? (float)i / (float)(count - 1) : 1.0f;
			Color color = point_color;
			color.a *= 0.2f + 0.8f * age;
			draw_rect(Rect2(Vector2(x - point * 0.5f, y - point * 0.5f), Vector2(point, point)), color);
		}
	}

	void PhaseScopeVisualizer::draw_correlation_meter(const Rect2& p_rect) {
		draw_rect(p_rect, Color(0.0f, 0.0f, 0.0f, 0.2f));

		const float bar_gap = 5.0f;
		const float third = (p_rect.size.y - bar_gap * 2.0f) / 3.0f;
		const Rect2 corr_rect(p_rect.position, Vector2(p_rect.size.x, third));
		const Rect2 width_rect(Vector2(p_rect.position.x, p_rect.position.y + third + bar_gap), Vector2(p_rect.size.x, third));
		const Rect2 balance_rect(Vector2(p_rect.position.x, p_rect.position.y + (third + bar_gap) * 2.0f), Vector2(p_rect.size.x, third));

		const float corr_t = (correlation + 1.0f) * 0.5f;
		const Color corr_color = correlation_color(correlation, meter_negative_color, meter_neutral_color, meter_positive_color);
		draw_rect(corr_rect, Color(0.0f, 0.0f, 0.0f, 0.3f));
		draw_rect(Rect2(corr_rect.position, Vector2(corr_rect.size.x * corr_t, corr_rect.size.y)), corr_color);

		draw_rect(width_rect, Color(0.0f, 0.0f, 0.0f, 0.3f));
		draw_rect(Rect2(width_rect.position, Vector2(width_rect.size.x * AudioVisualizerBusUtils::clamp_float(width, 0.0f, 1.0f), width_rect.size.y)), meter_positive_color);

		const float balance_center = balance_rect.position.x + balance_rect.size.x * 0.5f;
		const float balance_extent = balance_rect.size.x * 0.5f * std::abs(balance);
		draw_rect(balance_rect, Color(0.0f, 0.0f, 0.0f, 0.3f));
		if (balance >= 0.0f) {
			draw_rect(Rect2(Vector2(balance_center, balance_rect.position.y), Vector2(balance_extent, balance_rect.size.y)), meter_positive_color);
		}
		else {
			draw_rect(Rect2(Vector2(balance_center - balance_extent, balance_rect.position.y), Vector2(balance_extent, balance_rect.size.y)), meter_negative_color);
		}
	}

	void PhaseScopeVisualizer::set_use_bus(bool p_enabled) {
		use_bus = p_enabled;
		queue_redraw();
	}

	bool PhaseScopeVisualizer::get_use_bus() const {
		return use_bus;
	}

	void PhaseScopeVisualizer::set_bus(const StringName& p_bus) {
		bus = p_bus;
		queue_redraw();
	}

	StringName PhaseScopeVisualizer::get_bus() const {
		return bus;
	}

	void PhaseScopeVisualizer::set_samples(const PackedVector2Array& p_samples) {
		samples = p_samples;
		update_samples();
		update_phase_metrics();
		queue_redraw();
	}

	PackedVector2Array PhaseScopeVisualizer::get_samples() const {
		return samples;
	}

	void PhaseScopeVisualizer::set_sample_count(int p_count) {
		sample_count = std::max(16, p_count);
		live_samples.reserve((size_t)sample_count);
		if ((int)live_samples.size() > sample_count) {
			live_samples.erase(live_samples.begin(), live_samples.begin() + ((int)live_samples.size() - sample_count));
		}
		queue_redraw();
	}

	int PhaseScopeVisualizer::get_sample_count() const {
		return sample_count;
	}

	void PhaseScopeVisualizer::set_frozen(bool p_enabled) {
		frozen = p_enabled;
		queue_redraw();
	}

	bool PhaseScopeVisualizer::get_frozen() const {
		return frozen;
	}

	void PhaseScopeVisualizer::set_draw_grid(bool p_enabled) {
		draw_grid = p_enabled;
		queue_redraw();
	}

	bool PhaseScopeVisualizer::get_draw_grid() const {
		return draw_grid;
	}

	void PhaseScopeVisualizer::set_draw_meter(bool p_enabled) {
		draw_meter = p_enabled;
		queue_redraw();
	}

	bool PhaseScopeVisualizer::get_draw_meter() const {
		return draw_meter;
	}

	void PhaseScopeVisualizer::set_gain(float p_gain) {
		gain = std::max(0.0f, p_gain);
		queue_redraw();
	}

	float PhaseScopeVisualizer::get_gain() const {
		return gain;
	}

	void PhaseScopeVisualizer::set_point_size(float p_size) {
		point_size = std::max(1.0f, p_size);
		queue_redraw();
	}

	float PhaseScopeVisualizer::get_point_size() const {
		return point_size;
	}

	void PhaseScopeVisualizer::set_padding(float p_padding) {
		padding = std::max(0.0f, p_padding);
		queue_redraw();
	}

	float PhaseScopeVisualizer::get_padding() const {
		return padding;
	}

	void PhaseScopeVisualizer::set_meter_width(float p_width) {
		meter_width = std::max(0.0f, p_width);
		queue_redraw();
	}

	float PhaseScopeVisualizer::get_meter_width() const {
		return meter_width;
	}

	void PhaseScopeVisualizer::set_smoothing(float p_smoothing) {
		smoothing = AudioVisualizerBusUtils::clamp_float(p_smoothing, 0.0f, 1.0f);
	}

	float PhaseScopeVisualizer::get_smoothing() const {
		return smoothing;
	}

	void PhaseScopeVisualizer::set_background_color(const Color& p_color) {
		background_color = p_color;
		queue_redraw();
	}

	Color PhaseScopeVisualizer::get_background_color() const {
		return background_color;
	}

	void PhaseScopeVisualizer::set_grid_color(const Color& p_color) {
		grid_color = p_color;
		queue_redraw();
	}

	Color PhaseScopeVisualizer::get_grid_color() const {
		return grid_color;
	}

	void PhaseScopeVisualizer::set_point_color(const Color& p_color) {
		point_color = p_color;
		queue_redraw();
	}

	Color PhaseScopeVisualizer::get_point_color() const {
		return point_color;
	}

	void PhaseScopeVisualizer::set_meter_negative_color(const Color& p_color) {
		meter_negative_color = p_color;
		queue_redraw();
	}

	Color PhaseScopeVisualizer::get_meter_negative_color() const {
		return meter_negative_color;
	}

	void PhaseScopeVisualizer::set_meter_positive_color(const Color& p_color) {
		meter_positive_color = p_color;
		queue_redraw();
	}

	Color PhaseScopeVisualizer::get_meter_positive_color() const {
		return meter_positive_color;
	}

	void PhaseScopeVisualizer::set_meter_neutral_color(const Color& p_color) {
		meter_neutral_color = p_color;
		queue_redraw();
	}

	Color PhaseScopeVisualizer::get_meter_neutral_color() const {
		return meter_neutral_color;
	}

	float PhaseScopeVisualizer::get_correlation() const {
		return correlation;
	}

	float PhaseScopeVisualizer::get_width() const {
		return width;
	}

	float PhaseScopeVisualizer::get_balance() const {
		return balance;
	}
}
