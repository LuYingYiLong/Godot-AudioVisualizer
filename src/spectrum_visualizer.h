#ifndef SPECTRUM_VISUALIZER_H
#define SPECTRUM_VISUALIZER_H

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include <cstdint>
#include <vector>

namespace godot {
	class SpectrumVisualizer : public Control {
		GDCLASS(SpectrumVisualizer, Control)

	private:
		class FFTImpl;
		FFTImpl* fft_impl = nullptr;

		// 音频数据
		PackedFloat32Array audio_samples;
		int sample_rate = 44100;

		// 显示参数
		Ref<Image> spectrum_image;
		Ref<ImageTexture> spectrum_texture;

		// 颜色配置
		Color color_bottom = Color(0, 0, 0.1f, 1);
		Color color_lower = Color(0, 0, 1, 1);
		Color color_mid = Color(0, 1, 0, 1);
		Color color_upper = Color(1, 1, 0, 1);
		Color color_top = Color(1, 0, 0, 1);
		Color default_min_color = Color(0, 0, 0, 1);
		Color default_max_color = Color(1, 1, 1, 1);

		// FFT参数
		int fft_size = 1024;                                        // FFT窗口大小
		int fft_overlap = 4;                                        // 窗口重叠次数
		float fft_window_strength = 0.5f;                           // 窗口函数强度

		// 频谱图参数
		bool use_log_frequency = true;                              // 使用对数频率轴
		bool use_db_scale = true;                                   // 使用分贝刻度
		float dynamic_range_db = 60.0f;                             // 动态范围（分贝）
		float min_db = -90.0f;                                      // 最小分贝值

		// 时间轴参数
		bool show_full_audio = true;                                // 显示完整音频
		bool flip_time_axis = false;                                // 反转时间轴
		float time_window = 60.0f;                                  // 显示的时间窗口（秒）
		float time_resolution = 0.02f;                              // 时间分辨率（秒）

		// LOD和性能
		bool auto_adjust_lod = true;                                // 自动调整细节层次
		int max_time_slices = 500;                                  // 最大时间片数量
		int max_frequency_bins = 512;                               // 最大频率bin数量

		// 缓冲区
		std::vector<float> window_function;
		std::vector<float> fft_buffer;
		std::vector<float> magnitude_spectrum;
		std::vector<uint8_t> color_ramp_rgba;

		// 内部状态
		bool needs_update = false;
		bool data_valid = false;
		bool color_ramp_dirty = true;

		// 私有方法
		bool ensure_image_size();
		void generate_spectrogram();
		void compute_window_function();
		void rebuild_color_ramp();
		void apply_lod_adjustments(int& time_slices, int& freq_bins);
		void regenerate_if_ready();

		Color get_color_for_magnitude(float normalized_magnitude) const;

	protected:
		static void _bind_methods();
		void _notification(int p_what);

	public:
		SpectrumVisualizer();
		~SpectrumVisualizer();

		// 数据设置
		void set_audio_samples(const PackedFloat32Array& p_samples, int p_sample_rate = 44100);
		PackedFloat32Array get_audio_samples() const;
		void set_sample_rate(int p_sample_rate);
		int get_sample_rate() const;

		// 颜色配置
		void set_color_bottom(const Color& p_color);
		Color get_color_bottom() const;

		void set_color_lower(const Color& p_color);
		Color get_color_lower() const;

		void set_color_mid(const Color& p_color);
		Color get_color_mid() const;

		void set_color_upper(const Color& p_color);
		Color get_color_upper() const;

		void set_color_top(const Color& p_color);
		Color get_color_top() const;

		void set_default_min_color(const Color& p_color);
		Color get_default_min_color() const;

		void set_default_max_color(const Color& p_color);
		Color get_default_max_color() const;

		// FFT参数
		void set_fft_size(int p_size);
		int get_fft_size() const;

		void set_fft_overlap(int p_overlap);
		int get_fft_overlap() const;

		void set_fft_window_strength(float p_strength);
		float get_fft_window_strength() const;

		// 频谱图参数
		void set_use_log_frequency(bool p_enabled);
		bool get_use_log_frequency() const;

		void set_use_db_scale(bool p_enabled);
		bool get_use_db_scale() const;

		void set_dynamic_range_db(float p_range);
		float get_dynamic_range_db() const;

		void set_min_db(float p_min_db);
		float get_min_db() const;

		// 时间轴参数
		void set_show_full_audio(bool p_enabled);
		bool get_show_full_audio() const;

		void set_time_window(float p_window);
		float get_time_window() const;

		void set_flip_time_axis(bool p_enabled);
		bool get_flip_time_axis() const;

		void set_time_resolution(float p_resolution);
		float get_time_resolution() const;

		// LOD参数
		void set_auto_adjust_lod(bool p_enabled);
		bool get_auto_adjust_lod() const;

		void set_max_time_slices(int p_slices);
		int get_max_time_slices() const;

		void set_max_frequency_bins(int p_bins);
		int get_max_frequency_bins() const;

		// 操作函数
		void generate_spectrum();
		void update_display();
		void clear();
		Error save_to_file(const String& p_path);

		// 工具函数
		Vector2i get_image_size() const;
		bool has_audio_data() const;
		float get_audio_duration() const;
	};
}

#endif // SPECTRUM_VISUALIZER_H
