#include <Python.h>
#include <exception>
#include <new>

namespace SimWrap
{
    // Exception occured in Python API function.  We don't need any
    // further information as we can rely on Python's exception
    // information.
    class ExceptionInPythonAPI : public std::exception {};

    // Base class for exceptions that need to be turned into Python
    // excpetions
    class Exception :  public std::exception
    {
    public:
        Exception(const char *msg_);
        const char *what() const throw();
        void raise() const throw();
    private:
        const char *msg;
    };

    class TypeError : public Exception
    {
    public:
        TypeError(const char *msg_);
        void raise() const throw();
    };

    // Convert basic Python types to the corresponding C++ type
    template <typename T>
    T convert_from_py(PyObject *obj);
    template <>
    inline bool convert_from_py(PyObject *obj)
    {
        bool res = PyObject_IsTrue(obj);
        if (res == -1)
            throw ExceptionInPythonAPI();
        return res;
    }
    template <>
    inline long convert_from_py(PyObject *obj)
    {
        long res = PyInt_AsLong(obj);
        if (res == -1 && PyErr_Occurred())
            throw ExceptionInPythonAPI();
        return res;
    }
    template <>
    inline double convert_from_py(PyObject *obj)
    {
        double res = PyFloat_AsDouble(obj);
        if (res == -1 && PyErr_Occurred())
            throw ExceptionInPythonAPI();
        return res;
    }
    template <>
    inline const char *convert_from_py(PyObject *obj)
    {
        const char *res = PyString_AsString(obj);
        if (!res)
            throw ExceptionInPythonAPI();
        return res;
    }

    // Convert basic C++ types to the corresponding Python type
    template <typename T>
    PyObject *convert_to_py(T value);
    template <>
    inline PyObject *convert_to_py(bool value)
    {
        // This can't fail because Py_True and Py_False are static
        return PyBool_FromLong(value);
    }
    template <>
    inline PyObject *convert_to_py(long value)
    {
        PyObject *obj = PyInt_FromLong(value);
        if (!obj)
            throw ExceptionInPythonAPI();
        return obj;
    }
    template <>
    inline PyObject *convert_to_py(double value)
    {
            PyObject *obj = PyFloat_FromDouble(value);
            if (!obj)
                throw ExceptionInPythonAPI();
        return obj;
    }
    template <>
    inline PyObject *convert_to_py(const char *value)
    {
        PyObject *obj = PyString_FromString(value);
        if (!obj)
            throw ExceptionInPythonAPI();
        return obj;
    }

    class Config
    {
    public:
        Config(PyObject *map_)
            : map(map_)
        {
            Py_INCREF(map);
        }
        ~Config()
        {
            Py_DECREF(map);
        }
        template <typename T>
        void get(const char *key, T &value) const
        {
            PyObject *obj = PyMapping_GetItemString(map, const_cast<char *>(key));
            if (!obj)
                throw ExceptionInPythonAPI();
            value = convert_from_py<T>(obj);
        }
        template <typename T>
        void set(const char *key, T value) const
        {
            if (PyMapping_SetItemString(
                    map, const_cast<char *>(key), convert_to_py(value)) == -1)
                throw ExceptionInPythonAPI();
        }
    private:
        PyObject *const map;
    };

    class Simulation
    {
    public:
        Simulation(const Config& config_)
            : config(config_)
        {}
        virtual ~Simulation()
        {}
        virtual void do_time_step(double time_step)
        {}
    protected:
        Config config;
    };

    typedef struct
    {
        PyObject_HEAD
        Simulation *simulation;
    } SimulationObject;

    template <typename Sim> PyObject *
    simulation_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
    {
        PyObject *map;
        static char *kwlist[] = {(char *)"config", 0};
        if (!PyArg_ParseTupleAndKeywords(
                args, kwargs, "O:Simulation", kwlist, &map))
            return 0;
        if (!PyMapping_Check(map)) {
            PyErr_SetString(PyExc_TypeError, "argument must be a mapping");
            return 0;
        }
        Config config(map);
        SimulationObject *sim = (SimulationObject *)type->tp_alloc(type, 0);
        if (!sim)
            return 0;
        try {
            sim->simulation = new(std::nothrow) Sim(config);
        }
        catch (ExceptionInPythonAPI e) {
            return 0;
        }
        if (!sim->simulation) {
            Py_DECREF(sim);
            PyErr_SetString(PyExc_MemoryError,
                            "could not allocate Simulation object");
            return 0;
        }
        return (PyObject *)sim;
    }

    extern PyTypeObject SimulationType;

    PyObject *init_simulation_module(const char *name, const char *doc);

    template <typename Sim> void
    add_simulation_type(PyObject *module, const char *type_name, const char *doc)
    {
        if (!module)
            return;
        PyTypeObject *new_type =
            (PyTypeObject *)PyType_Type.tp_alloc(&PyType_Type, 0);
        if (!new_type)
            return;
        new_type->tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
        const char *mod_name = PyModule_GetName(module);
        if (mod_name)
        {
            char *qname = new char[256];
            sprintf(qname, "%s.%s", mod_name, type_name);
            new_type->tp_name = qname;
        }
        else {
            new_type->tp_name = type_name;
        }
        new_type->tp_doc = doc;
        new_type->tp_base = &SimulationType;
        new_type->tp_new = simulation_new<Sim>;
        if (PyType_Ready(new_type) == -1)
        {
            Py_DECREF(new_type);
            return;
        }
        PyModule_AddObject(module, type_name, (PyObject *)new_type);
    }
}
