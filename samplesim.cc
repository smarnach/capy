#include "simwrap.hh"

namespace SimWrap
{
    class MySimulation : public Simulation
    {
    public:
        MySimulation(const Mapping& config_)
            : Simulation(config_), f(config.get<Function>("f"))
        {}

        void
        do_time_step(double time_step)
        {
            config.get("name", name);
            config.get("verbose", verbose);
            config.get("x0", x0);
            config.get("x1", x1);
            double x = x0;
            while (x < x1 + time_step*1e-10)
            {
                double y = f.call<double>(x);
                if (verbose)
                    printf("%s(%12g) = %12g\n", name, x, y);
                else
                    printf("%12g\n", y);
                x += time_step;
            }
        }

    private:
        Function f;
        const char *name;
        bool verbose;
        double x0, x1;
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
