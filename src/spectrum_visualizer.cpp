#include "spectrum_visualizer.h"

#include "thirdparty/kissfft/kiss_fft.h"

#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>

typedef ::kiss_fft_cfg LocalFFTCfg;
typedef ::kiss_fft_cpx LocalFFTCpx;

namespace godot {
	class SpectrumVisualizer::FFTImpl {
	private:
		LocalFFTCfg cfg = nullptr;
		std::vector<LocalFFTCpx> input;
		std::vector<LocalFFTCpx> output;
		int size = 0;

	public:
		FFTImpl(int p_fft_size) {
			resize(p_fft_size);
		}

		~FFTImpl() {
			cleanup();
		}

		void cleanup() {
			if (cfg) {
				::free(cfg);
				cfg = nullptr;
			}

			input.clear();
			output.clear();
			size = 0;
		}

		bool is_valid() const {
			return cfg && (int)input.size() == size && (int)output.size() == size && size > 0;
		}

		void resize(int p_new_size) {
			if (p_new_size == size && is_valid()) {
				return;
			}

			cleanup();

			if (p_new_size <= 0) {
				return;
			}

			cfg = kiss_fft_alloc(p_new_size, 0, nullptr, nullptr);
			if (!cfg) {
				UtilityFunctions::push_error("Failed to allocate kissfft config");
				return;
			}

			size = p_new_size;
			input.resize(size);
			output.resize(size);
		}

		bool compute(const float* p_samples, const std::vector<float>& p_window, std::vector<float>& r_magnitude_spectrum) {
			const int bin_count = size / 2 + 1;

			if (!is_valid() || !p_samples || (int)p_window.size() != size || (int)r_magnitude_spectrum.size() != bin_count) {
				return false;
			}

			float energy = 0.0f;
			for (int i = 0; i < size; i++) {
				float sample = p_samples[i];
				if (!std::isfinite(sample)) {
					sample = 0.0f;
				}

				const float windowed_sample = sample * p_window[i];
				input[i].r = windowed_sample;
				input[i].i = 0.0f;
				energy += windowed_sample * windowed_sample;
			}

			if (energy <= 1e-12f) {
				std::fill(r_magnitude_spectrum.begin(), r_magnitude_spectrum.end(), 0.0f);
				return false;
			}

			kiss_fft(cfg, input.data(), output.data());

			for (int i = 0; i < bin_count; i++) {
				const float real = output[i].r;
				const float imag = output[i].i;
				r_magnitude_spectrum[i] = std::sqrt(real * real + imag * imag);
			}

			return true;
		}
	};

	namespace {
		constexpr float TAU = 6.28318530717958647692f;
		constexpr int COLOR_RAMP_STEPS = 256;
		constexpr int RGBA_CHANNELS = 4;

		inline float linear_to_db(float p_linear) {
			if (p_linear <= 1e-20f) {
				return -200.0f;
			}

			return 20.0f * std::log10(p_linear);
		}

		inline float clamp_float(float p_value, float p_min, float p_max) {
			if (p_value < p_min) {
				return p_min;
			}

			if (p_value > p_max) {
				return p_max;
			}

			return p_value;
		}

		inline int clamp_int(int p_value, int p_min, int p_max) {
			if (p_value < p_min) {
				return p_min;
			}

			if (p_value > p_max) {
				return p_max;
			}

			return p_value;
		}

		inline int next_power_of_two(int p_value) {
			p_value = std::max(1, p_value);

			int power = 1;
			while (power < p_value && power < (1 << 30)) {
				power <<= 1;
			}

			return power;
		}

		inline uint8_t color_channel_to_u8(float p_value) {
			return (uint8_t)clamp_int((int)std::lround(clamp_float(p_value, 0.0f, 1.0f) * 255.0f), 0, 255);
		}

		inline void color_to_rgba8(const Color& p_color, uint8_t* r_rgba) {
			r_rgba[0] = color_channel_to_u8(p_color.r);
			r_rgba[1] = color_channel_to_u8(p_color.g);
			r_rgba[2] = color_channel_to_u8(p_color.b);
			r_rgba[3] = color_channel_to_u8(p_color.a);
		}

		inline const uint8_t* ramp_color_ptr(const std::vector<uint8_t>& p_ramp, int p_index) {
			return &p_ramp[clamp_int(p_index, 0, COLOR_RAMP_STEPS - 1) * RGBA_CHANNELS];
		}

		void fill_pixels(uint8_t* p_pixels, int p_width, int p_height, const uint8_t* p_rgba) {
			if (!p_pixels || p_width <= 0 || p_height <= 0) {
				return;
			}

			const int pixel_count = p_width * p_height;

			if (p_rgba[0] == p_rgba[1] && p_rgba[1] == p_rgba[2] && p_rgba[2] == p_rgba[3]) {
				std::memset(p_pixels, p_rgba[0], pixel_count * RGBA_CHANNELS);
				return;
			}

			for (int i = 0; i < pixel_count; i++) {
				uint8_t* pixel = p_pixels + i * RGBA_CHANNELS;
				pixel[0] = p_rgba[0];
				pixel[1] = p_rgba[1];
				pixel[2] = p_rgba[2];
				pixel[3] = p_rgba[3];
			}
		}

		inline void set_rgba_pixel(uint8_t* p_pixels, int p_width, int p_x, int p_y, const uint8_t* p_rgba) {
			uint8_t* pixel = p_pixels + ((p_y * p_width + p_x) * RGBA_CHANNELS);
			pixel[0] = p_rgba[0];
			pixel[1] = p_rgba[1];
			pixel[2] = p_rgba[2];
			pixel[3] = p_rgba[3];
		}
	}

	void SpectrumVisualizer::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_audio_samples", "samples", "sample_rate"), &SpectrumVisualizer::set_audio_samples, DEFVAL(44100));
		ClassDB::bind_method(D_METHOD("get_audio_samples"), &SpectrumVisualizer::get_audio_samples);

		ClassDB::bind_method(D_METHOD("set_sample_rate", "sample_rate"), &SpectrumVisualizer::set_sample_rate);
		ClassDB::bind_method(D_METHOD("get_sample_rate"), &SpectrumVisualizer::get_sample_rate);

		ClassDB::bind_method(D_METHOD("set_color_bottom", "color"), &SpectrumVisualizer::set_color_bottom);
		ClassDB::bind_method(D_METHOD("get_color_bottom"), &SpectrumVisualizer::get_color_bottom);

		ClassDB::bind_method(D_METHOD("set_color_lower", "color"), &SpectrumVisualizer::set_color_lower);
		ClassDB::bind_method(D_METHOD("get_color_lower"), &SpectrumVisualizer::get_color_lower);

		ClassDB::bind_method(D_METHOD("set_color_mid", "color"), &SpectrumVisualizer::set_color_mid);
		ClassDB::bind_method(D_METHOD("get_color_mid"), &SpectrumVisualizer::get_color_mid);

		ClassDB::bind_method(D_METHOD("set_color_upper", "color"), &SpectrumVisualizer::set_color_upper);
		ClassDB::bind_method(D_METHOD("get_color_upper"), &SpectrumVisualizer::get_color_upper);

		ClassDB::bind_method(D_METHOD("set_color_top", "color"), &SpectrumVisualizer::set_color_top);
		ClassDB::bind_method(D_METHOD("get_color_top"), &SpectrumVisualizer::get_color_top);

		ClassDB::bind_method(D_METHOD("set_default_min_color", "color"), &SpectrumVisualizer::set_default_min_color);
		ClassDB::bind_method(D_METHOD("get_default_min_color"), &SpectrumVisualizer::get_default_min_color);

		ClassDB::bind_method(D_METHOD("set_default_max_color", "color"), &SpectrumVisualizer::set_default_max_color);
		ClassDB::bind_method(D_METHOD("get_default_max_color"), &SpectrumVisualizer::get_default_max_color);

		ClassDB::bind_method(D_METHOD("set_fft_size", "size"), &SpectrumVisualizer::set_fft_size);
		ClassDB::bind_method(D_METHOD("get_fft_size"), &SpectrumVisualizer::get_fft_size);

		ClassDB::bind_method(D_METHOD("set_fft_overlap", "overlap"), &SpectrumVisualizer::set_fft_overlap);
		ClassDB::bind_method(D_METHOD("get_fft_overlap"), &SpectrumVisualizer::get_fft_overlap);

		ClassDB::bind_method(D_METHOD("set_fft_window_strength", "strength"), &SpectrumVisualizer::set_fft_window_strength);
		ClassDB::bind_method(D_METHOD("get_fft_window_strength"), &SpectrumVisualizer::get_fft_window_strength);

		ClassDB::bind_method(D_METHOD("set_use_log_frequency", "enabled"), &SpectrumVisualizer::set_use_log_frequency);
		ClassDB::bind_method(D_METHOD("get_use_log_frequency"), &SpectrumVisualizer::get_use_log_frequency);

		ClassDB::bind_method(D_METHOD("set_use_db_scale", "enabled"), &SpectrumVisualizer::set_use_db_scale);
		ClassDB::bind_method(D_METHOD("get_use_db_scale"), &SpectrumVisualizer::get_use_db_scale);

		ClassDB::bind_method(D_METHOD("set_dynamic_range_db", "range"), &SpectrumVisualizer::set_dynamic_range_db);
		ClassDB::bind_method(D_METHOD("get_dynamic_range_db"), &SpectrumVisualizer::get_dynamic_range_db);

		ClassDB::bind_method(D_METHOD("set_min_db", "min_db"), &SpectrumVisualizer::set_min_db);
		ClassDB::bind_method(D_METHOD("get_min_db"), &SpectrumVisualizer::get_min_db);

		ClassDB::bind_method(D_METHOD("set_show_full_audio", "enabled"), &SpectrumVisualizer::set_show_full_audio);
		ClassDB::bind_method(D_METHOD("get_show_full_audio"), &SpectrumVisualizer::get_show_full_audio);

		ClassDB::bind_method(D_METHOD("set_time_window", "window"), &SpectrumVisualizer::set_time_window);
		ClassDB::bind_method(D_METHOD("get_time_window"), &SpectrumVisualizer::get_time_window);

		ClassDB::bind_method(D_METHOD("set_flip_time_axis", "enabled"), &SpectrumVisualizer::set_flip_time_axis);
		ClassDB::bind_method(D_METHOD("get_flip_time_axis"), &SpectrumVisualizer::get_flip_time_axis);

		ClassDB::bind_method(D_METHOD("set_time_resolution", "resolution"), &SpectrumVisualizer::set_time_resolution);
		ClassDB::bind_method(D_METHOD("get_time_resolution"), &SpectrumVisualizer::get_time_resolution);

		ClassDB::bind_method(D_METHOD("set_auto_adjust_lod", "enabled"), &SpectrumVisualizer::set_auto_adjust_lod);
		ClassDB::bind_method(D_METHOD("get_auto_adjust_lod"), &SpectrumVisualizer::get_auto_adjust_lod);

		ClassDB::bind_method(D_METHOD("set_max_time_slices", "slices"), &SpectrumVisualizer::set_max_time_slices);
		ClassDB::bind_method(D_METHOD("get_max_time_slices"), &SpectrumVisualizer::get_max_time_slices);

		ClassDB::bind_method(D_METHOD("set_max_frequency_bins", "bins"), &SpectrumVisualizer::set_max_frequency_bins);
		ClassDB::bind_method(D_METHOD("get_max_frequency_bins"), &SpectrumVisualizer::get_max_frequency_bins);

		ClassDB::bind_method(D_METHOD("generate_spectrum"), &SpectrumVisualizer::generate_spectrum);
		ClassDB::bind_method(D_METHOD("update_display"), &SpectrumVisualizer::update_display);
		ClassDB::bind_method(D_METHOD("clear"), &SpectrumVisualizer::clear);
		ClassDB::bind_method(D_METHOD("save_to_file", "path"), &SpectrumVisualizer::save_to_file);

		ClassDB::bind_method(D_METHOD("get_image_size"), &SpectrumVisualizer::get_image_size);
		ClassDB::bind_method(D_METHOD("has_audio_data"), &SpectrumVisualizer::has_audio_data);
		ClassDB::bind_method(D_METHOD("get_audio_duration"), &SpectrumVisualizer::get_audio_duration);

		ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_rate"), "set_sample_rate", "get_sample_rate");

		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color_bottom"), "set_color_bottom", "get_color_bottom");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color_lower"), "set_color_lower", "get_color_lower");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color_mid"), "set_color_mid", "get_color_mid");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color_upper"), "set_color_upper", "get_color_upper");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color_top"), "set_color_top", "get_color_top");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "default_min_color"), "set_default_min_color", "get_default_min_color");
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "default_max_color"), "set_default_max_color", "get_default_max_color");

		ADD_PROPERTY(PropertyInfo(Variant::INT, "fft_size", PROPERTY_HINT_RANGE, "64,4096,2"), "set_fft_size", "get_fft_size");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "fft_overlap", PROPERTY_HINT_RANGE, "1,16,1"), "set_fft_overlap", "get_fft_overlap");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fft_window_strength", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_fft_window_strength", "get_fft_window_strength");

		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_log_frequency"), "set_use_log_frequency", "get_use_log_frequency");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_db_scale"), "set_use_db_scale", "get_use_db_scale");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "dynamic_range_db", PROPERTY_HINT_RANGE, "20.0,120.0,1.0"), "set_dynamic_range_db", "get_dynamic_range_db");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_db", PROPERTY_HINT_RANGE, "-120.0,0.0,1.0"), "set_min_db", "get_min_db");

		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_full_audio"), "set_show_full_audio", "get_show_full_audio");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "time_window", PROPERTY_HINT_RANGE, "0.1,3600.0,0.1"), "set_time_window", "get_time_window");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flip_time_axis"), "set_flip_time_axis", "get_flip_time_axis");
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "time_resolution", PROPERTY_HINT_RANGE, "0.001,0.5,0.001"), "set_time_resolution", "get_time_resolution");

		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_adjust_lod"), "set_auto_adjust_lod", "get_auto_adjust_lod");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "max_time_slices", PROPERTY_HINT_RANGE, "10,2000,1"), "set_max_time_slices", "get_max_time_slices");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "max_frequency_bins", PROPERTY_HINT_RANGE, "16,2048,1"), "set_max_frequency_bins", "get_max_frequency_bins");
	}

	SpectrumVisualizer::SpectrumVisualizer() {
		fft_impl = new FFTImpl(fft_size);
		compute_window_function();
		rebuild_color_ramp();
	}

	SpectrumVisualizer::~SpectrumVisualizer() {
		if (fft_impl) {
			delete fft_impl;
			fft_impl = nullptr;
		}

		audio_samples.clear();
	}

	void SpectrumVisualizer::_notification(int p_what) {
		switch (p_what) {
		case NOTIFICATION_READY:
			ensure_image_size();
			if (data_valid) {
				generate_spectrum();
			}
			else {
				clear();
			}
			break;

		case NOTIFICATION_RESIZED:
			if (ensure_image_size()) {
				if (data_valid) {
					generate_spectrum();
				}
				else {
					clear();
				}
			}
			break;

		case NOTIFICATION_DRAW:
			if (spectrum_texture.is_valid()) {
				draw_texture_rect(spectrum_texture, Rect2(Vector2(), get_size()), false);
			}
			break;
		}
	}

	void SpectrumVisualizer::compute_window_function() {
		window_function.resize(fft_size);

		if (fft_size <= 1) {
			std::fill(window_function.begin(), window_function.end(), 1.0f);
			return;
		}

		const float alpha = fft_window_strength;

		for (int i = 0; i < fft_size; i++) {
			const float hanning = 0.5f * (1.0f - std::cos(TAU * (float)i / (float)(fft_size - 1)));
			const float window_value = alpha * hanning + (1.0f - alpha);
			window_function[i] = window_value;
		}
	}

	bool SpectrumVisualizer::ensure_image_size() {
		const Vector2 current_size = get_size();
		const int width = std::max(1, (int)std::ceil(std::max(0.0f, current_size.x)));
		const int height = std::max(1, (int)std::ceil(std::max(0.0f, current_size.y)));

		if (spectrum_image.is_valid() &&
			spectrum_image->get_width() == width &&
			spectrum_image->get_height() == height) {
			return false;
		}

		spectrum_image = Image::create(width, height, false, Image::FORMAT_RGBA8);
		if (!spectrum_image.is_valid()) {
			UtilityFunctions::push_error("Failed to create spectrum image");
			spectrum_texture.unref();
			return false;
		}

		spectrum_image->fill(default_min_color);
		spectrum_texture.unref();
		return true;
	}

	void SpectrumVisualizer::apply_lod_adjustments(int& time_slices, int& freq_bins) {
		const Vector2i image_size = get_image_size();
		if (image_size.x <= 0 || image_size.y <= 0) {
			time_slices = std::max(1, time_slices);
			freq_bins = std::max(1, freq_bins);
			return;
		}

		time_slices = clamp_int(time_slices, 1, image_size.x);
		freq_bins = clamp_int(freq_bins, 1, image_size.y);

		if (!auto_adjust_lod) {
			return;
		}

		const float lod_factor_x = clamp_float(static_cast<float>(image_size.x) / 800.0f, 0.1f, 4.0f);
		const float lod_factor_y = clamp_float(static_cast<float>(image_size.y) / 400.0f, 0.1f, 4.0f);
		const int time_limit = clamp_int((int)std::ceil(static_cast<float>(max_time_slices) * lod_factor_x), 1, max_time_slices);
		const int freq_limit = clamp_int((int)std::ceil(static_cast<float>(max_frequency_bins) * lod_factor_y), 1, max_frequency_bins);

		time_slices = std::min(time_slices, std::min(time_limit, image_size.x));
		freq_bins = std::min(freq_bins, std::min(freq_limit, image_size.y));
	}

	Color SpectrumVisualizer::get_color_for_magnitude(float p_normalized_magnitude) const {
		const float n = clamp_float(p_normalized_magnitude, 0.0f, 1.0f);

		if (n < 0.25f) {
			return color_bottom.lerp(color_lower, n * 4.0f);
		}

		if (n < 0.50f) {
			return color_lower.lerp(color_mid, (n - 0.25f) * 4.0f);
		}

		if (n < 0.75f) {
			return color_mid.lerp(color_upper, (n - 0.50f) * 4.0f);
		}

		return color_upper.lerp(color_top, (n - 0.75f) * 4.0f);
	}

	void SpectrumVisualizer::rebuild_color_ramp() {
		color_ramp_rgba.resize(COLOR_RAMP_STEPS * RGBA_CHANNELS);

		for (int i = 0; i < COLOR_RAMP_STEPS; i++) {
			const float magnitude = (float)i / (float)(COLOR_RAMP_STEPS - 1);
			color_to_rgba8(get_color_for_magnitude(magnitude), &color_ramp_rgba[i * RGBA_CHANNELS]);
		}

		color_ramp_dirty = false;
	}

	void SpectrumVisualizer::generate_spectrogram() {
		if (audio_samples.is_empty() || sample_rate <= 0) {
			return;
		}

		if (!spectrum_image.is_valid() || !fft_impl || !fft_impl->is_valid()) {
			UtilityFunctions::push_error("Spectrum image or FFT implementation is not valid");
			return;
		}

		const int image_width = spectrum_image->get_width();
		const int image_height = spectrum_image->get_height();
		if (image_width <= 0 || image_height <= 0) {
			return;
		}

		uint8_t* pixels = spectrum_image->ptrw();
		if (!pixels) {
			UtilityFunctions::push_error("Failed to access spectrum image data");
			return;
		}

		uint8_t background_rgba[RGBA_CHANNELS];
		color_to_rgba8(default_min_color, background_rgba);
		fill_pixels(pixels, image_width, image_height, background_rgba);

		if (color_ramp_dirty || (int)color_ramp_rgba.size() != COLOR_RAMP_STEPS * RGBA_CHANNELS) {
			rebuild_color_ramp();
		}

		if ((int)window_function.size() != fft_size) {
			compute_window_function();
		}

		const int total_samples = (int)audio_samples.size();
		const int window_samples = clamp_int((int)std::ceil(time_window * (float)sample_rate), 1, total_samples);
		const int visible_samples = show_full_audio ? total_samples : window_samples;
		const int visible_start_sample = std::max(0, total_samples - visible_samples);

		int time_slices = image_width;
		int freq_bins = fft_size / 2 + 1;
		apply_lod_adjustments(time_slices, freq_bins);
		freq_bins = clamp_int(freq_bins, 1, fft_size / 2 + 1);

		fft_buffer.resize(fft_size);
		magnitude_spectrum.resize(fft_size / 2 + 1);

		const float* samples = audio_samples.ptr();
		const int target_cols = image_width;
		const int target_rows = image_height;

		for (int time_idx = 0; time_idx < target_cols; time_idx++) {
			const float t = target_cols <= 1 ? 0.0f : (float)time_idx / (float)(target_cols - 1);
			const int sample_center = visible_start_sample + (int)std::lround(t * (float)(visible_samples - 1));
			const int sample_offset = sample_center - fft_size / 2;

			std::fill(fft_buffer.begin(), fft_buffer.end(), 0.0f);

			const int source_start = std::max(0, sample_offset);
			const int source_end = std::min(total_samples, sample_offset + fft_size);
			if (source_end > source_start) {
				const int destination_start = source_start - sample_offset;
				std::copy(samples + source_start, samples + source_end, fft_buffer.begin() + destination_start);
			}

			if (!fft_impl->compute(fft_buffer.data(), window_function, magnitude_spectrum)) {
				continue;
			}

			const int x_pos = flip_time_axis ? target_cols - 1 - time_idx : time_idx;
			const int max_freq_idx = std::min(freq_bins - 1, (int)magnitude_spectrum.size() - 1);

			for (int y = 0; y < target_rows; y++) {
				const float fy = target_rows <= 1 ? 0.0f : (float)y / (float)(target_rows - 1);
				int freq_idx = 0;

				if (use_log_frequency && max_freq_idx > 1) {
					const float log_max = std::log10((float)max_freq_idx);
					freq_idx = clamp_int((int)std::pow(10.0f, fy * log_max), 1, max_freq_idx);
				}
				else {
					freq_idx = clamp_int((int)(fy * (float)max_freq_idx), 0, max_freq_idx);
				}

				float normalized_magnitude = magnitude_spectrum[freq_idx];

				if (use_db_scale) {
					const float db_value = linear_to_db(normalized_magnitude);
					normalized_magnitude = (db_value - min_db) / dynamic_range_db;
				}

				const int color_index = clamp_int((int)std::lround(clamp_float(normalized_magnitude, 0.0f, 1.0f) * (float)(COLOR_RAMP_STEPS - 1)), 0, COLOR_RAMP_STEPS - 1);
				const uint8_t* rgba = ramp_color_ptr(color_ramp_rgba, color_index);
				const int y_pos = target_rows - 1 - y;

				set_rgba_pixel(pixels, image_width, x_pos, y_pos, rgba);
			}
		}
	}

	void SpectrumVisualizer::regenerate_if_ready() {
		needs_update = true;

		if (data_valid) {
			generate_spectrum();
		}
	}

	void SpectrumVisualizer::set_audio_samples(const PackedFloat32Array& p_samples, int p_sample_rate) {
		audio_samples = p_samples;
		sample_rate = std::max(1, p_sample_rate);
		data_valid = !audio_samples.is_empty();
		needs_update = data_valid;

		if (!fft_impl) {
			fft_impl = new FFTImpl(fft_size);
		}
		else {
			fft_impl->resize(fft_size);
		}

		compute_window_function();
		ensure_image_size();

		if (data_valid) {
			generate_spectrum();
		}
		else {
			clear();
		}
	}

	PackedFloat32Array SpectrumVisualizer::get_audio_samples() const {
		return audio_samples;
	}

	void SpectrumVisualizer::set_sample_rate(int p_sample_rate) {
		const int new_sample_rate = std::max(1, p_sample_rate);
		if (sample_rate == new_sample_rate) {
			return;
		}

		sample_rate = new_sample_rate;
		regenerate_if_ready();
	}

	int SpectrumVisualizer::get_sample_rate() const {
		return sample_rate;
	}

	void SpectrumVisualizer::set_color_bottom(const Color& p_color) {
		if (color_bottom == p_color) {
			return;
		}

		color_bottom = p_color;
		color_ramp_dirty = true;
		regenerate_if_ready();
	}

	Color SpectrumVisualizer::get_color_bottom() const {
		return color_bottom;
	}

	void SpectrumVisualizer::set_color_lower(const Color& p_color) {
		if (color_lower == p_color) {
			return;
		}

		color_lower = p_color;
		color_ramp_dirty = true;
		regenerate_if_ready();
	}

	Color SpectrumVisualizer::get_color_lower() const {
		return color_lower;
	}

	void SpectrumVisualizer::set_color_mid(const Color& p_color) {
		if (color_mid == p_color) {
			return;
		}

		color_mid = p_color;
		color_ramp_dirty = true;
		regenerate_if_ready();
	}

	Color SpectrumVisualizer::get_color_mid() const {
		return color_mid;
	}

	void SpectrumVisualizer::set_color_upper(const Color& p_color) {
		if (color_upper == p_color) {
			return;
		}

		color_upper = p_color;
		color_ramp_dirty = true;
		regenerate_if_ready();
	}

	Color SpectrumVisualizer::get_color_upper() const {
		return color_upper;
	}

	void SpectrumVisualizer::set_color_top(const Color& p_color) {
		if (color_top == p_color) {
			return;
		}

		color_top = p_color;
		color_ramp_dirty = true;
		regenerate_if_ready();
	}

	Color SpectrumVisualizer::get_color_top() const {
		return color_top;
	}

	void SpectrumVisualizer::set_default_min_color(const Color& p_color) {
		if (default_min_color == p_color) {
			return;
		}

		default_min_color = p_color;

		if (data_valid) {
			generate_spectrum();
		}
		else {
			clear();
		}
	}

	Color SpectrumVisualizer::get_default_min_color() const {
		return default_min_color;
	}

	void SpectrumVisualizer::set_default_max_color(const Color& p_color) {
		if (default_max_color == p_color) {
			return;
		}

		default_max_color = p_color;
	}

	Color SpectrumVisualizer::get_default_max_color() const {
		return default_max_color;
	}

	void SpectrumVisualizer::set_fft_size(int p_size) {
		const int new_fft_size = next_power_of_two(std::max(64, p_size));
		if (fft_size == new_fft_size) {
			return;
		}

		fft_size = new_fft_size;

		if (!fft_impl) {
			fft_impl = new FFTImpl(fft_size);
		}
		else {
			fft_impl->resize(fft_size);
		}

		compute_window_function();
		magnitude_spectrum.clear();
		fft_buffer.clear();
		regenerate_if_ready();
	}

	int SpectrumVisualizer::get_fft_size() const {
		return fft_size;
	}

	void SpectrumVisualizer::set_fft_overlap(int p_overlap) {
		const int new_overlap = clamp_int(p_overlap, 1, 16);
		if (fft_overlap == new_overlap) {
			return;
		}

		fft_overlap = new_overlap;
		regenerate_if_ready();
	}

	int SpectrumVisualizer::get_fft_overlap() const {
		return fft_overlap;
	}

	void SpectrumVisualizer::set_fft_window_strength(float p_strength) {
		const float new_strength = clamp_float(p_strength, 0.0f, 1.0f);
		if (fft_window_strength == new_strength) {
			return;
		}

		fft_window_strength = new_strength;
		compute_window_function();
		regenerate_if_ready();
	}

	float SpectrumVisualizer::get_fft_window_strength() const {
		return fft_window_strength;
	}

	void SpectrumVisualizer::set_use_log_frequency(bool p_enabled) {
		if (use_log_frequency == p_enabled) {
			return;
		}

		use_log_frequency = p_enabled;
		regenerate_if_ready();
	}

	bool SpectrumVisualizer::get_use_log_frequency() const {
		return use_log_frequency;
	}

	void SpectrumVisualizer::set_use_db_scale(bool p_enabled) {
		if (use_db_scale == p_enabled) {
			return;
		}

		use_db_scale = p_enabled;
		regenerate_if_ready();
	}

	bool SpectrumVisualizer::get_use_db_scale() const {
		return use_db_scale;
	}

	void SpectrumVisualizer::set_dynamic_range_db(float p_range) {
		const float new_range = clamp_float(p_range, 20.0f, 120.0f);
		if (dynamic_range_db == new_range) {
			return;
		}

		dynamic_range_db = new_range;
		regenerate_if_ready();
	}

	float SpectrumVisualizer::get_dynamic_range_db() const {
		return dynamic_range_db;
	}

	void SpectrumVisualizer::set_min_db(float p_min_db) {
		const float new_min_db = clamp_float(p_min_db, -120.0f, 0.0f);
		if (min_db == new_min_db) {
			return;
		}

		min_db = new_min_db;
		regenerate_if_ready();
	}

	float SpectrumVisualizer::get_min_db() const {
		return min_db;
	}

	void SpectrumVisualizer::set_show_full_audio(bool p_enabled) {
		if (show_full_audio == p_enabled) {
			return;
		}

		show_full_audio = p_enabled;
		regenerate_if_ready();
	}

	bool SpectrumVisualizer::get_show_full_audio() const {
		return show_full_audio;
	}

	void SpectrumVisualizer::set_time_window(float p_window) {
		const float new_window = std::max(0.1f, p_window);
		if (time_window == new_window) {
			return;
		}

		time_window = new_window;
		regenerate_if_ready();
	}

	float SpectrumVisualizer::get_time_window() const {
		return time_window;
	}

	void SpectrumVisualizer::set_flip_time_axis(bool p_enabled) {
		if (flip_time_axis == p_enabled) {
			return;
		}

		flip_time_axis = p_enabled;
		regenerate_if_ready();
	}

	bool SpectrumVisualizer::get_flip_time_axis() const {
		return flip_time_axis;
	}

	void SpectrumVisualizer::set_time_resolution(float p_resolution) {
		const float new_resolution = clamp_float(p_resolution, 0.001f, 0.5f);
		if (time_resolution == new_resolution) {
			return;
		}

		time_resolution = new_resolution;
		regenerate_if_ready();
	}

	float SpectrumVisualizer::get_time_resolution() const {
		return time_resolution;
	}

	void SpectrumVisualizer::set_auto_adjust_lod(bool p_enabled) {
		if (auto_adjust_lod == p_enabled) {
			return;
		}

		auto_adjust_lod = p_enabled;
		regenerate_if_ready();
	}

	bool SpectrumVisualizer::get_auto_adjust_lod() const {
		return auto_adjust_lod;
	}

	void SpectrumVisualizer::set_max_time_slices(int p_slices) {
		const int new_slices = std::max(10, p_slices);
		if (max_time_slices == new_slices) {
			return;
		}

		max_time_slices = new_slices;
		if (auto_adjust_lod) {
			regenerate_if_ready();
		}
	}

	int SpectrumVisualizer::get_max_time_slices() const {
		return max_time_slices;
	}

	void SpectrumVisualizer::set_max_frequency_bins(int p_bins) {
		const int new_bins = std::max(16, p_bins);
		if (max_frequency_bins == new_bins) {
			return;
		}

		max_frequency_bins = new_bins;
		if (auto_adjust_lod) {
			regenerate_if_ready();
		}
	}

	int SpectrumVisualizer::get_max_frequency_bins() const {
		return max_frequency_bins;
	}

	void SpectrumVisualizer::generate_spectrum() {
		ensure_image_size();

		if (!data_valid || !spectrum_image.is_valid()) {
			return;
		}

		if (!fft_impl) {
			fft_impl = new FFTImpl(fft_size);
		}
		else if (!fft_impl->is_valid()) {
			fft_impl->resize(fft_size);
		}

		generate_spectrogram();
		update_display();
		needs_update = false;
	}

	void SpectrumVisualizer::update_display() {
		if (!spectrum_image.is_valid()) {
			return;
		}

		if (!spectrum_texture.is_valid() ||
			spectrum_texture->get_width() != spectrum_image->get_width() ||
			spectrum_texture->get_height() != spectrum_image->get_height()) {
			spectrum_texture = ImageTexture::create_from_image(spectrum_image);
		}
		else {
			spectrum_texture->update(spectrum_image);
		}

		queue_redraw();
	}

	void SpectrumVisualizer::clear() {
		ensure_image_size();

		if (spectrum_image.is_valid()) {
			spectrum_image->fill(default_min_color);
			update_display();
		}

		audio_samples.clear();
		data_valid = false;
		needs_update = false;
	}

	Error SpectrumVisualizer::save_to_file(const String& p_path) {
		if (spectrum_image.is_valid()) {
			return spectrum_image->save_png(p_path);
		}

		return FAILED;
	}

	Vector2i SpectrumVisualizer::get_image_size() const {
		if (spectrum_image.is_valid()) {
			return Vector2i(spectrum_image->get_width(), spectrum_image->get_height());
		}

		return Vector2i(0, 0);
	}

	bool SpectrumVisualizer::has_audio_data() const {
		return data_valid;
	}

	float SpectrumVisualizer::get_audio_duration() const {
		if (sample_rate > 0 && !audio_samples.is_empty()) {
			return (float)audio_samples.size() / (float)sample_rate;
		}

		return 0.0f;
	}
}
