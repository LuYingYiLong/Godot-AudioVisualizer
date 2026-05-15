#include "register_types.h"

#include "oscilloscope_visualizer.h"
#include "phase_meter.h"
#include "phase_scope_visualizer.h"
#include "realtime_spectrum_visualizer.h"
#include "spectrum_visualizer.h"
#include "vu_meter.h"
#include "wave_visualizer.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_audio_visualizer_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		return;
	}

	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	GDREGISTER_CLASS(RealtimeSpectrumVisualizer);
	GDREGISTER_CLASS(OscilloscopeVisualizer);
	GDREGISTER_CLASS(PhaseMeter);
	GDREGISTER_CLASS(PhaseScopeVisualizer);
	GDREGISTER_CLASS(SpectrumVisualizer);
	GDREGISTER_CLASS(VUMeter);
	GDREGISTER_CLASS(WaveVisualizer);
}

void uninitialize_audio_visualizer_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C" {
	// Initialization.
	GDExtensionBool GDE_EXPORT audio_visualizer_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization) {
		godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

		init_obj.register_initializer(initialize_audio_visualizer_module);
		init_obj.register_terminator(uninitialize_audio_visualizer_module);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_EDITOR);

		return init_obj.init();
	}
}
