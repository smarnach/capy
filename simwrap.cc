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
        Exception(const char *msg_)
            : msg(msg_) {}
        const char *what() const throw()
        {
            return msg;
        }
        void raise() const throw()
        {
            PyErr_SetString(PyExc_Exception, msg);
        }
    private:
        const char *msg;
    };

    class TypeError : public Exception
    {
    public:
        TypeError(const char *msg_)
            : Exception(msg_) {}
        void raise() const throw()
        {
            PyErr_SetString(PyExc_TypeError, what());
        }
    };

    // Conversions between basic Python and C++ types
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

    // Simple wrapper for Python mapping objects
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
        {
            config.get("b", b);
            config.get("i", i);
            config.get("d", d);
            config.get("s", s);
        }

        void do_time_step(double time_step)
        {
            config.set("b", true);
            config.set("i", (long)42);
            config.set("d", 4.2);
            config.set("s", "Huhu!");
        }

    private:
        Config config;
        bool b;
        long i;
        double d;
        const char *s;
    };
}

extern "C"
{
    typedef struct
    {
        PyObject_HEAD
        SimWrap::Simulation *simulation;
    } SimulationObject;

    static PyObject *
    simulation_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
    {
        PyObject *map;
        static char *kwlist[] = {(char *)"config", NULL};
        if (!PyArg_ParseTupleAndKeywords(
                args, kwargs, "O:Simulation", kwlist, &map))
            return 0;
        if (!PyMapping_Check(map)) {
            PyErr_SetString(PyExc_TypeError, "argument must be a mapping");
            return 0;
        }
        SimWrap::Config config(map);
        SimulationObject *sim = (SimulationObject *)type->tp_alloc(type, 0);
        if (!sim)
            return 0;
        try {
            sim->simulation = new(std::nothrow) SimWrap::Simulation(config);
        }
        catch (SimWrap::ExceptionInPythonAPI e) {
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

    static void
    simulation_dealloc(SimulationObject *self)
    {
        delete self->simulation;
        self->ob_type->tp_free((PyObject *)self);
    }

    static PyObject *
    simulation_do_time_step(SimulationObject *self, PyObject *args)
    {
        double time_step;
        if (!PyArg_ParseTuple(args, "d:do_time_step", &time_step))
            return 0;
        try {
            self->simulation->do_time_step(time_step);
        }
        catch (SimWrap::ExceptionInPythonAPI e) {
            return 0;
        }
        Py_RETURN_NONE;
    }

    static PyMethodDef simulation_methods[] = {
        {"do_time_step", (PyCFunction)simulation_do_time_step, METH_VARARGS,
         "Run a single time step of the simulation."},
        {NULL}  /* Sentinel */
    };

    static PyTypeObject SimulationType = {
        PyObject_HEAD_INIT(NULL)
        0,                         /* ob_size */
        "simwrap.Simulation",      /* tp_name */
        sizeof(SimulationObject),  /* tp_basicsize */
        0,                         /* tp_itemsize */
        0,                         /* tp_dealloc */
        0,                         /* tp_print */
        0,                         /* tp_getattr */
        0,                         /* tp_setattr */
        0,                         /* tp_compare */
        0,                         /* tp_repr */
        0,                         /* tp_as_number */
        0,                         /* tp_as_sequence */
        0,                         /* tp_as_mapping */
        0,                         /* tp_hash  */
        0,                         /* tp_call */
        0,                         /* tp_str */
        0,                         /* tp_getattro */
        0,                         /* tp_setattro */
        0,                         /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,        /* tp_flags */
        "Simulation type",         /* tp_doc */
        0,                         /* tp_traverse */
        0,                         /* tp_clear */
        0,                         /* tp_richcompare */
        0,                         /* tp_weaklistoffset */
        0,                         /* tp_iter */
        0,                         /* tp_iternext */
        simulation_methods,        /* tp_methods */
        0,                         /* tp_members */
        0,                         /* tp_getset */
        0,                         /* tp_base */
        0,                         /* tp_dict */
        0,                         /* tp_descr_get */
        0,                         /* tp_descr_set */
        0,                         /* tp_dictoffset */
        0,                         /* tp_init */
        0,                         /* tp_alloc */
        simulation_new,            /* tp_new */
    };

    static PyMethodDef simwrap_methods[] = {
        {NULL}  /* Sentinel */
    };

    #ifndef PyMODINIT_FUNC      /* declarations for DLL import/export */
    #define PyMODINIT_FUNC void
    #endif
    PyMODINIT_FUNC
    initsimwrap(void)
    {
        PyObject *m;

        if (PyType_Ready(&SimulationType) < 0)
            return;
        m = Py_InitModule3("simwrap", simwrap_methods,
                           "High-level wrapping of C++ simulations in Python.");
        if (!m)
            return;
        Py_INCREF(&SimulationType);
        PyModule_AddObject(m, "Simulation", (PyObject *)&SimulationType);
    }
}
