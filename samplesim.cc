#include "capy.hh"
#include "array.hh"

#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>

class MySimulation
{
public:
    MySimulation(const Capy::Mapping& config_)
        : config(config_),
          f(config.get("f", Capy::eval("lambda x: x * x")))
    {}

    void do_time_step(double time_step)
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
        config.set("x", Capy::Array(&x[0], x.size()));
        config.set("y", Capy::Array(&y[0], y.size()));
    }

    void write_output(const char *filename)
    {
        const char *name;
        bool verbose = config.get("verbose", false);
        if (verbose)
            name = config["name"];
        std::ofstream file(filename);
        for (unsigned i = 0; i < y.size(); ++i)
            if (verbose)
                file << name << "(" << std::setw(12) << x[i] << ") = "
                     << std::setw(12) << y[i] << "\n";
            else
                file << std::setw(12) << y[i] << "\n";
    }

private:
    Capy::Mapping config;
    Capy::Object f;
    std::vector<double> x;
    std::vector<double> y;
};

PyMODINIT_FUNC
initsamplesim()
{
    PyObject *module = Py_InitModule3(
        "samplesim", 0, "An example of a simulation wrapped with Capy");
    static Capy::Class<MySimulation> mysim(
        "MySimulation", "A stupid simulation examples class");
    mysim.add_method<double, &MySimulation::do_time_step>(
        "do_time_step", "Run a single time step of the simulation.");
    mysim.add_method<const char *, &MySimulation::write_output>(
        "write_output", "Write output to the given file name.");
    mysim.add_to(module);
    import_array();
}
