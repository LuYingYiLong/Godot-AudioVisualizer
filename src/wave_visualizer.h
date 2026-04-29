#ifndef WAVE_VISUALIZER_H
#define WAVE_VISUALIZER_H

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>

namespace godot {
    class WaveVisualizer : public Control {
        GDCLASS(WaveVisualizer, Control)

    private:
        Ref<Image> waveform_image;
        Ref<ImageTexture> waveform_texture;
        PackedFloat32Array audio_samples;

        Color bg_color = Color(0.1, 0.1, 0.1, 1.0);
        Color wave_color = Color(0.2, 0.8, 0.3, 1.0);
        Color center_color = Color(0.3, 0.3, 0.3, 0.5);

        enum WaveDirection {
            DIRECTION_LEFT_TO_RIGHT = 0,
            DIRECTION_BOTTOM_TO_TOP = 1,
            DIRECTION_TOP_TO_BOTTOM = 2
        };

        int direction = 0;
        Vector2i virtual_size = Vector2i(0, 0);
        Vector2i view_offset = Vector2i(0, 0);

        void _on_size_changed();
        void ensure_image_size();
        void _draw_waveform_region(const Rect2i& p_dirty_rect);
        void _clear_region(const Rect2i& p_dirty_rect);
        void _draw_horizontal_waveform(const Rect2i& p_dirty_rect);
        void _draw_vertical_waveform(const Rect2i& p_dirty_rect, bool p_invert_axis);
        void _draw_vertical_line(int p_x, int p_y_start, int p_y_end, const Color& p_color);
        void _draw_horizontal_line(int p_y, int p_x_start, int p_x_end, const Color& p_color);

    protected:
        static void _bind_methods();
        void _notification(int p_what);

    public:
        WaveVisualizer();
        ~WaveVisualizer();

        void set_audio_samples(const PackedFloat32Array& p_samples);
        PackedFloat32Array get_audio_samples() const;

        void set_wave_direction(int p_direction);
        int get_wave_direction() const;

        void set_background_color(const Color& p_color);
        Color get_background_color() const;

        void set_wave_color(const Color& p_color);
        Color get_wave_color() const;

        void set_center_color(const Color& p_color);
        Color get_center_color() const;

        void set_virtual_size(const Vector2i& p_size);
        Vector2i get_virtual_size() const;

        void set_view_offset(const Vector2i& p_offset);
        Vector2i get_view_offset() const;

        void update_region(const Rect2i& p_region);

        void generate_waveform();
        void update_display();

        void clear();
        Error save_to_file(const String& p_path);
        Vector2i get_image_size() const;
    };
}

#endif // !WAVE_VISUALIZER_H
