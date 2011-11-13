#include "capy.hh"

#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>

class MySimulation : public Capy::Simulation
{
public:
    MySimulation(const Capy::Mapping& config_)
        : Simulation(config_),
          f(config.get("f", Capy::eval("lambda x: x * x")))
    {}

    virtual void do_time_step(double time_step)
    {
        double x0 = config.get("x0", 0.0);
        double x1 = config.get("x1", 1.0);
        x.clear();
        y.clear();
        for (double t = x0; t < x1 + time_step*1e-10; t += time_step)
        {
            x.push_back(t);
            y.push_back(f(t));
        }
    }

    virtual void write_output(const char *filename)
    {
        const char *name;
        bool verbose = config.get("verbose", false);
        if (verbose)
            name = config.get("name");
        std::ofstream file(filename);
        for (unsigned i = 0; i < y.size(); ++i)
            if (verbose)
                file << name << "(" << std::setw(12) << x[i] << ") = "
                     << std::setw(12) << y[i] << "\n";
            else
                file << std::setw(12) << y[i] << "\n";
    }
private:
    Capy::Object f;
    std::vector<double> x;
    std::vector<double> y;
};

PyMODINIT_FUNC
initsamplesim()
{
    PyObject *m = Capy::init_simulation_module(
        "samplesim", "An example of a simulation wrapped with Capy");
    Capy::add_simulation_type<MySimulation>(
        m, "MySimulation", "A stupid simulation examples class");
}
