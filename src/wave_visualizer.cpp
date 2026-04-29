#include "wave_visualizer.h"

#include <godot_cpp/classes/texture2d.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace godot {
	void WaveVisualizer::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_audio_samples", "samples"), &WaveVisualizer::set_audio_samples);
		ClassDB::bind_method(D_METHOD("get_audio_samples"), &WaveVisualizer::get_audio_samples);

		ClassDB::bind_method(D_METHOD("set_wave_direction", "direction"), &WaveVisualizer::set_wave_direction);
		ClassDB::bind_method(D_METHOD("get_wave_direction"), &WaveVisualizer::get_wave_direction);

		ClassDB::bind_method(D_METHOD("set_background_color", "color"), &WaveVisualizer::set_background_color);
		ClassDB::bind_method(D_METHOD("get_background_color"), &WaveVisualizer::get_background_color);

		ClassDB::bind_method(D_METHOD("set_wave_color", "color"), &WaveVisualizer::set_wave_color);
		ClassDB::bind_method(D_METHOD("get_wave_color"), &WaveVisualizer::get_wave_color);

		ClassDB::bind_method(D_METHOD("set_center_color", "color"), &WaveVisualizer::set_center_color);
		ClassDB::bind_method(D_METHOD("get_center_color"), &WaveVisualizer::get_center_color);

		ClassDB::bind_method(D_METHOD("set_virtual_size", "size"), &WaveVisualizer::set_virtual_size);
		ClassDB::bind_method(D_METHOD("get_virtual_size"), &WaveVisualizer::get_virtual_size);

		ClassDB::bind_method(D_METHOD("set_view_offset", "offset"), &WaveVisualizer::set_view_offset);
		ClassDB::bind_method(D_METHOD("get_view_offset"), &WaveVisualizer::get_view_offset);

		ClassDB::bind_method(D_METHOD("update_region", "region"), &WaveVisualizer::update_region);
		ClassDB::bind_method(D_METHOD("generate_waveform"), &WaveVisualizer::generate_waveform);
		ClassDB::bind_method(D_METHOD("update_display"), &WaveVisualizer::update_display);
		ClassDB::bind_method(D_METHOD("clear"), &WaveVisualizer::clear);
		ClassDB::bind_method(D_METHOD("save_to_file", "path"), &WaveVisualizer::save_to_file);
		ClassDB::bind_method(D_METHOD("get_image_size"), &WaveVisualizer::get_image_size);

		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "background_color"), "set_background_color", "get_background_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "wave_color"), "set_wave_color", "get_wave_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "center_color"), "set_center_color", "get_center_color");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "wave_direction",
			PROPERTY_HINT_ENUM, "Left to Right,Bottom to Top,Top to Bottom"),
			"set_wave_direction", "get_wave_direction");
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "virtual_size"), "set_virtual_size", "get_virtual_size");
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "view_offset"), "set_view_offset", "get_view_offset");
	}

	WaveVisualizer::WaveVisualizer() {
		direction = DIRECTION_LEFT_TO_RIGHT;
		set_clip_contents(true);
	}

	WaveVisualizer::~WaveVisualizer() {
		audio_samples.clear();
	}

	void WaveVisualizer::_notification(int p_what) {
		switch (p_what) {
		case NOTIFICATION_READY:
			if (audio_samples.is_empty()) {
				clear();
			}
			else {
				generate_waveform();
			}
			break;
		case NOTIFICATION_RESIZED:
			_on_size_changed();
			break;
		case NOTIFICATION_DRAW:
			if (waveform_texture.is_valid()) {
				draw_texture_rect(waveform_texture, Rect2(Vector2(), get_size()), false);
			}
			break;
		}
	}

	void WaveVisualizer::_on_size_changed() {
		Vector2i previous_size = get_image_size();
		ensure_image_size();

		if (!waveform_image.is_valid()) {
			return;
		}

		if (previous_size == get_image_size() && waveform_texture.is_valid()) {
			queue_redraw();
			return;
		}

		if (audio_samples.is_empty()) {
			clear();
		}
		else {
			generate_waveform();
		}
	}

	void WaveVisualizer::ensure_image_size() {
		Vector2 current_size = get_size();
		int width = std::max(1, (int)std::ceil(std::max(0.0, (double)current_size.x)));
		int height = std::max(1, (int)std::ceil(std::max(0.0, (double)current_size.y)));

		if (!waveform_image.is_valid() ||
			waveform_image->get_width() != width ||
			waveform_image->get_height() != height) {
			waveform_image = Image::create(width, height, false, Image::FORMAT_RGBA8);

			if (waveform_image.is_valid()) {
				waveform_image->fill(bg_color);
			}
		}
	}

	Vector2i WaveVisualizer::get_image_size() const {
		if (waveform_image.is_valid()) {
			return Vector2i(waveform_image->get_width(), waveform_image->get_height());
		}

		return Vector2i(0, 0);
	}

	void WaveVisualizer::set_audio_samples(const PackedFloat32Array& p_samples) {
		audio_samples = p_samples;
		ensure_image_size();

		if (audio_samples.is_empty()) {
			clear();
		}
		else {
			generate_waveform();
		}
	}

	PackedFloat32Array WaveVisualizer::get_audio_samples() const {
		return audio_samples;
	}

	void WaveVisualizer::set_wave_direction(int p_direction) {
		int new_direction = std::max(0, std::min(2, p_direction));

		if (direction == new_direction) {
			return;
		}

		direction = new_direction;

		if (waveform_image.is_valid() && !audio_samples.is_empty()) {
			generate_waveform();
		}
	}

	int WaveVisualizer::get_wave_direction() const {
		return direction;
	}

	void WaveVisualizer::set_virtual_size(const Vector2i& p_size) {
		if (virtual_size == p_size) {
			return;
		}

		virtual_size = p_size;

		if (waveform_image.is_valid() && !audio_samples.is_empty()) {
			generate_waveform();
		}
	}

	Vector2i WaveVisualizer::get_virtual_size() const {
		return virtual_size;
	}

	void WaveVisualizer::set_view_offset(const Vector2i& p_offset) {
		if (view_offset == p_offset) {
			return;
		}

		view_offset = p_offset;

		if (waveform_image.is_valid() && !audio_samples.is_empty()) {
			generate_waveform();
		}
	}

	Vector2i WaveVisualizer::get_view_offset() const {
		return view_offset;
	}

	void WaveVisualizer::generate_waveform() {
		ensure_image_size();

		if (!waveform_image.is_valid()) {
			return;
		}

		Rect2i full_rect(0, 0, waveform_image->get_width(), waveform_image->get_height());
		_draw_waveform_region(full_rect);
		update_display();
	}

	void WaveVisualizer::update_region(const Rect2i& p_region) {
		if (!waveform_image.is_valid()) {
			return;
		}

		Rect2i image_rect(0, 0, waveform_image->get_width(), waveform_image->get_height());
		Rect2i dirty_rect = image_rect.intersection(p_region);

		if (!dirty_rect.has_area()) {
			return;
		}

		_draw_waveform_region(dirty_rect);
		update_display();
	}

	void WaveVisualizer::_draw_waveform_region(const Rect2i& p_dirty_rect) {
		if (!waveform_image.is_valid() || !p_dirty_rect.has_area()) {
			return;
		}

		if (audio_samples.is_empty()) {
			waveform_image->fill_rect(p_dirty_rect, bg_color);
			return;
		}

		_clear_region(p_dirty_rect);

		switch (direction) {
		case DIRECTION_LEFT_TO_RIGHT:
			_draw_horizontal_waveform(p_dirty_rect);
			break;
		case DIRECTION_BOTTOM_TO_TOP:
			_draw_vertical_waveform(p_dirty_rect, false);
			break;
		case DIRECTION_TOP_TO_BOTTOM:
			_draw_vertical_waveform(p_dirty_rect, true);
			break;
		}
	}

	void WaveVisualizer::_clear_region(const Rect2i& p_dirty_rect) {
		waveform_image->fill_rect(p_dirty_rect, bg_color);

		int img_w = waveform_image->get_width();
		int img_h = waveform_image->get_height();
		int start_x = p_dirty_rect.position.x;
		int start_y = p_dirty_rect.position.y;
		int end_x = start_x + p_dirty_rect.size.x - 1;
		int end_y = start_y + p_dirty_rect.size.y - 1;

		if (direction == DIRECTION_LEFT_TO_RIGHT) {
			int center_y = img_h / 2;

			if (center_y >= start_y && center_y <= end_y) {
				_draw_horizontal_line(center_y, start_x, end_x, center_color);
			}
		}
		else {
			int center_x = img_w / 2;

			if (center_x >= start_x && center_x <= end_x) {
				_draw_vertical_line(center_x, start_y, end_y, center_color);
			}
		}
	}

	void WaveVisualizer::_draw_horizontal_waveform(const Rect2i& p_dirty_rect) {
		int img_w = waveform_image->get_width();
		int img_h = waveform_image->get_height();
		int64_t sample_count = audio_samples.size();
		int64_t virtual_span = virtual_size.x > 0 ? virtual_size.x : img_w;

		if (virtual_span <= 0 || sample_count <= 0) {
			return;
		}

		int center_y = img_h / 2;
		int half_height = std::max(1, (img_h / 2) - 2);
		int start_x = p_dirty_rect.position.x;
		int start_y = p_dirty_rect.position.y;
		int end_x = start_x + p_dirty_rect.size.x;
		int end_y = start_y + p_dirty_rect.size.y;
		double samples_per_pixel = (double)sample_count / (double)virtual_span;
		const float* samples = audio_samples.ptr();

		for (int x = start_x; x < end_x; x++) {
			int64_t virtual_x = (int64_t)x + view_offset.x;

			if (virtual_x < 0 || virtual_x >= virtual_span) {
				continue;
			}

			int64_t sample_start_idx = (int64_t)(virtual_x * samples_per_pixel);
			int64_t sample_end_idx = (int64_t)((virtual_x + 1) * samples_per_pixel);

			if (sample_start_idx >= sample_count) {
				continue;
			}

			sample_end_idx = std::min(sample_end_idx, sample_count);

			if (sample_end_idx <= sample_start_idx) {
				sample_end_idx = std::min(sample_start_idx + 1, sample_count);
			}

			float min_val = 1.0f;
			float max_val = -1.0f;
			bool has_data = false;

			for (int64_t i = sample_start_idx; i < sample_end_idx; i++) {
				float val = samples[i];

				if (!std::isfinite(val)) {
					continue;
				}

				val = std::max(-1.0f, std::min(1.0f, val));
				min_val = std::min(min_val, val);
				max_val = std::max(max_val, val);
				has_data = true;
			}

			if (!has_data) {
				continue;
			}

			int y_min = center_y - (int)(min_val * half_height);
			int y_max = center_y - (int)(max_val * half_height);

			if (y_min > y_max) {
				std::swap(y_min, y_max);
			}

			y_min = std::max(0, std::min(img_h - 1, y_min));
			y_max = std::max(0, std::min(img_h - 1, y_max));

			int draw_y_start = std::max(y_min, start_y);
			int draw_y_end = std::min(y_max, end_y - 1);

			if (draw_y_start <= draw_y_end) {
				_draw_vertical_line(x, draw_y_start, draw_y_end, wave_color);
			}
		}
	}

	void WaveVisualizer::_draw_vertical_waveform(const Rect2i& p_dirty_rect, bool p_invert_axis) {
		int img_w = waveform_image->get_width();
		int img_h = waveform_image->get_height();
		int64_t sample_count = audio_samples.size();
		int64_t virtual_span = virtual_size.y > 0 ? virtual_size.y : img_h;

		if (virtual_span <= 0 || sample_count <= 0) {
			return;
		}

		int center_x = img_w / 2;
		int half_width = std::max(1, (img_w / 2) - 2);
		int start_x = p_dirty_rect.position.x;
		int start_y = p_dirty_rect.position.y;
		int end_x = start_x + p_dirty_rect.size.x;
		int end_y = start_y + p_dirty_rect.size.y;
		double samples_per_pixel = (double)sample_count / (double)virtual_span;
		const float* samples = audio_samples.ptr();

		for (int y = start_y; y < end_y; y++) {
			int64_t virtual_y = (int64_t)y + view_offset.y;
			int64_t sample_y = p_invert_axis ? virtual_span - 1 - virtual_y : virtual_y;

			if (sample_y < 0 || sample_y >= virtual_span) {
				continue;
			}

			int64_t sample_start_idx = (int64_t)(sample_y * samples_per_pixel);
			int64_t sample_end_idx = (int64_t)((sample_y + 1) * samples_per_pixel);

			if (sample_start_idx >= sample_count) {
				continue;
			}

			sample_end_idx = std::min(sample_end_idx, sample_count);

			if (sample_end_idx <= sample_start_idx) {
				sample_end_idx = std::min(sample_start_idx + 1, sample_count);
			}

			float min_val = 1.0f;
			float max_val = -1.0f;
			bool has_data = false;

			for (int64_t i = sample_start_idx; i < sample_end_idx; i++) {
				float val = samples[i];

				if (!std::isfinite(val)) {
					continue;
				}

				val = std::max(-1.0f, std::min(1.0f, val));
				min_val = std::min(min_val, val);
				max_val = std::max(max_val, val);
				has_data = true;
			}

			if (!has_data) {
				continue;
			}

			int x_min = center_x - (int)(min_val * half_width);
			int x_max = center_x - (int)(max_val * half_width);

			if (x_min > x_max) {
				std::swap(x_min, x_max);
			}

			x_min = std::max(0, std::min(img_w - 1, x_min));
			x_max = std::max(0, std::min(img_w - 1, x_max));

			int draw_x_start = std::max(x_min, start_x);
			int draw_x_end = std::min(x_max, end_x - 1);

			if (draw_x_start <= draw_x_end) {
				_draw_horizontal_line(y, draw_x_start, draw_x_end, wave_color);
			}
		}
	}

	void WaveVisualizer::_draw_vertical_line(int p_x, int p_y_start, int p_y_end, const Color& p_color) {
		if (!waveform_image.is_valid()) {
			return;
		}

		int img_w = waveform_image->get_width();
		int img_h = waveform_image->get_height();

		if (p_x < 0 || p_x >= img_w) {
			return;
		}

		int y_start = std::max(0, std::min(p_y_start, p_y_end));
		int y_end = std::min(img_h - 1, std::max(p_y_start, p_y_end));

		for (int y = y_start; y <= y_end; y++) {
			waveform_image->set_pixel(p_x, y, p_color);
		}
	}

	void WaveVisualizer::_draw_horizontal_line(int p_y, int p_x_start, int p_x_end, const Color& p_color) {
		if (!waveform_image.is_valid()) {
			return;
		}

		int img_w = waveform_image->get_width();
		int img_h = waveform_image->get_height();

		if (p_y < 0 || p_y >= img_h) {
			return;
		}

		int x_start = std::max(0, std::min(p_x_start, p_x_end));
		int x_end = std::min(img_w - 1, std::max(p_x_start, p_x_end));

		for (int x = x_start; x <= x_end; x++) {
			waveform_image->set_pixel(x, p_y, p_color);
		}
	}

	void WaveVisualizer::update_display() {
		if (!waveform_image.is_valid()) {
			return;
		}

		if (!waveform_texture.is_valid() ||
			waveform_image->get_width() != waveform_texture->get_width() ||
			waveform_image->get_height() != waveform_texture->get_height()) {
			waveform_texture = ImageTexture::create_from_image(waveform_image);
		}
		else {
			waveform_texture->update(waveform_image);
		}

		queue_redraw();
	}

	void WaveVisualizer::clear() {
		ensure_image_size();

		if (!waveform_image.is_valid()) {
			return;
		}

		waveform_image->fill(bg_color);
		update_display();
	}

	Error WaveVisualizer::save_to_file(const String& p_path) {
		if (waveform_image.is_valid()) {
			return waveform_image->save_png(p_path);
		}

		return FAILED;
	}

	void WaveVisualizer::set_background_color(const Color& p_color) {
		if (bg_color == p_color) {
			return;
		}

		bg_color = p_color;

		if (!waveform_image.is_valid()) {
			return;
		}

		if (audio_samples.is_empty()) {
			clear();
		}
		else {
			generate_waveform();
		}
	}

	Color WaveVisualizer::get_background_color() const {
		return bg_color;
	}

	void WaveVisualizer::set_wave_color(const Color& p_color) {
		if (wave_color == p_color) {
			return;
		}

		wave_color = p_color;

		if (waveform_image.is_valid() && !audio_samples.is_empty()) {
			generate_waveform();
		}
	}

	Color WaveVisualizer::get_wave_color() const {
		return wave_color;
	}

	void WaveVisualizer::set_center_color(const Color& p_color) {
		if (center_color == p_color) {
			return;
		}

		center_color = p_color;

		if (waveform_image.is_valid() && !audio_samples.is_empty()) {
			generate_waveform();
		}
	}

	Color WaveVisualizer::get_center_color() const {
		return center_color;
	}
}
