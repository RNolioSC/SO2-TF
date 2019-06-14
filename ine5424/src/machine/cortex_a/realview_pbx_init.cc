// EPOS RealView PBX (ARM Cortex) MCU Initialization

#include <machine/main.h>

#ifdef __mmod_realview_pbx__

void _startup_cache();

__BEGIN_SYS

void Realview_PBX::pre_init() {
	//db<Init, Machine>(TRC) << "Pre inicializando a realview" << endl;
}

void Realview_PBX::init() {
	//db<Init, Machine>(TRC) << "Inicializando a realview" << endl;
}

__END_SYS

#endif
