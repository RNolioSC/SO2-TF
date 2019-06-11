// EPOS Cortex-A Timer Mediator Initialization

#include <machine/timer.h>
#include <machine/ic.h>

__BEGIN_SYS

//Still the same as LM3S811

void Timer::init() {
    db<Init, Timer>(TRC) << "Timer::init()" << endl;

    Engine::init(FREQUENCY);
    IC::int_vector(IC::INT_TIMER, int_handler);
    IC::enable(IC::INT_TIMER);
    Engine::enable();
}

__END_SYS
