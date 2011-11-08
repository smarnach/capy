#include "simwrap.hh"

namespace SimWrap
{
    class MySimulation : public Simulation
    {
    public:
        MySimulation(const Mapping& config_)
            : Simulation(config_)
        {
            config.get("b", b);
            config.get("i", i);
            config.get("d", d);
            config.get("s", s);
        }

        void
        do_time_step(double time_step)
        {
            config.set("b", true);
            config.set("i", 42L);
            config.set("d", 4.2);
            config.set("s", "Huhu!");
        }

    private:
        bool b;
        long i;
        double d;
        const char *s;
    };

    PyMODINIT_FUNC
    initsamplesim()
    {
        PyObject *m = init_simulation_module(
            "samplesim", "An example of a simulation wrapped with SimWrap");
        add_simulation_type<MySimulation>(
            m, "MySimulation", "A stupid simulation examples class");
    }
}
