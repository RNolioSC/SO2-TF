// EPOS RealView PBX (ARM Cortex) MCU Initialization

#include <machine/main.h>

#ifdef __mmod_realview_pbx__

void _startup_cache();

__BEGIN_SYS

void Realview_PBX::pre_init() {}

void Realview_PBX::init() {
}

__END_SYS

#endif
