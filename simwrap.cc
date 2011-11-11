#include <Python.h>
#include <exception>
#include <new>

#include "simwrap.hh"

namespace SimWrap
{
    // Implementation of Mapping
    Mapping::Mapping(PyObject *map_)
        : map(map_)
    {
        if (!PyMapping_Check(map))
            throw TypeError("argument must be a mapping");
        Py_INCREF(map);
    }
    Mapping::Mapping(const Mapping &other)
        : map(other.map)
    {
        Py_INCREF(map);
    }
    Mapping::~Mapping()
    {
        Py_DECREF(map);
    }
    bool Mapping::has_key(const char *key) const
    {
        return PyMapping_HasKeyString(map, const_cast<char *>(key));
    }
    PyObject *Mapping::get_python_mapping() const
    {
        return map;
    }

    // Implementation of Function
    Function::Function(PyObject *pyfunc_)
        : pyfunc(pyfunc_)
    {
        if (!PyCallable_Check(pyfunc))
            throw TypeError("argument must be callable");
        Py_INCREF(pyfunc);
    }
    Function::Function(const Function &other)
        : pyfunc(other.pyfunc)
    {
        Py_INCREF(pyfunc);
    }
    Function::~Function()
    {
        Py_DECREF(pyfunc);
    }

    // Convert basic Python types to the corresponding C++ type
    template <>
    void convert_from_py(PyObject *obj)
    {}
    template <>
    bool convert_from_py(PyObject *obj)
    {
        bool res = PyObject_IsTrue(obj);
        if (res == -1)
            throw ExceptionInPythonAPI();
        return res;
    }
    template <>
    long convert_from_py(PyObject *obj)
    {
        long res = PyInt_AsLong(obj);
        if (res == -1 && PyErr_Occurred())
            throw ExceptionInPythonAPI();
        return res;
    }
    template <>
    double convert_from_py(PyObject *obj)
    {
        double res = PyFloat_AsDouble(obj);
        if (res == -1 && PyErr_Occurred())
            throw ExceptionInPythonAPI();
        return res;
    }
    template <>
    const char *convert_from_py(PyObject *obj)
    {
        const char *res = PyString_AsString(obj);
        if (!res)
            throw ExceptionInPythonAPI();
        return res;
    }
    template <>
    Mapping convert_from_py(PyObject *obj)
    {
        return Mapping(obj);
    }
    template <>
    Function convert_from_py(PyObject *obj)
    {
        return Function(obj);
    }

    // Convert basic C++ types to the corresponding Python type
    PyObject *convert_to_py(bool value)
    {
        // This can't fail because Py_True and Py_False are static
        return PyBool_FromLong(value);
    }
    PyObject *convert_to_py(long value)
    {
        PyObject *obj = PyInt_FromLong(value);
        if (!obj)
            throw ExceptionInPythonAPI();
        return obj;
    }
    PyObject *convert_to_py(double value)
    {
            PyObject *obj = PyFloat_FromDouble(value);
            if (!obj)
                throw ExceptionInPythonAPI();
        return obj;
    }
    PyObject *convert_to_py(const char *value)
    {
        PyObject *obj = PyString_FromString(value);
        if (!obj)
            throw ExceptionInPythonAPI();
        return obj;
    }

    static void
    simulation_dealloc(SimulationObject *self)
    {
        delete self->simulation;
        PyObject_GC_UnTrack(self);
        self->ob_type->tp_free((PyObject *)self);
    }

    static int
    simulation_traverse(SimulationObject *self, visitproc visit, void *arg)
    {
        Py_VISIT(self->simulation->config.get_python_mapping());
        return 0;
    }

    static PyMethodDef simulation_methods[] = {
        {"do_time_step",
         wrap_method<double, &Simulation::do_time_step>,
         METH_VARARGS,
         "Run a single time step of the simulation."},
        {"write_output",
         wrap_method<const char*, &Simulation::write_output>,
         METH_VARARGS,
         "Write output to the given file name."},
        {0}  /* Sentinel */
    };

    static PyObject *
    simulation_get_config(PyObject *self, void *)
    {
        PyObject *map = ((SimulationObject *)self)->
            simulation->config.get_python_mapping();
        Py_INCREF(map);
        return map;
    }

    static PyGetSetDef simulation_getset[] = {
        {(char *)"config", simulation_get_config, 0,
         (char *)"config attribute", 0},
        {0}  /* Sentinel */
    };

    const char *simulation_doc =
        "Base object for simulation wrappers.";

    PyTypeObject SimulationType = {
        PyObject_HEAD_INIT(0)
        0,                         /* ob_size */
        0,                         /* tp_name */
        sizeof(SimulationObject),  /* tp_basicsize */
        0,                         /* tp_itemsize */
        (destructor)simulation_dealloc, /* tp_dealloc */
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
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE |
        Py_TPFLAGS_HAVE_GC,        /* tp_flags */
        simulation_doc,            /* tp_doc */
        (traverseproc)simulation_traverse, /* tp_traverse */
        0,                         /* tp_clear */
        0,                         /* tp_richcompare */
        0,                         /* tp_weaklistoffset */
        0,                         /* tp_iter */
        0,                         /* tp_iternext */
        simulation_methods,        /* tp_methods */
        0,                         /* tp_members */
        simulation_getset,         /* tp_getset */
        0,                         /* tp_base */
        0,                         /* tp_dict */
        0,                         /* tp_descr_get */
        0,                         /* tp_descr_set */
        0,                         /* tp_dictoffset */
        0,                         /* tp_init */
        0,                         /* tp_alloc */
        simulation_new<Simulation>, /* tp_new */
    };

    PyObject *
    init_simulation_module(const char *mod_name, const char *doc,
                           PyMethodDef *methods)
    {
        PyObject *m = Py_InitModule3(mod_name, methods, doc);
        if (!m)
            return 0;
        char *qname = new char[256];
        sprintf(qname, "%s.Simulation", mod_name);
        SimulationType.tp_name = qname;
        if (PyType_Ready(&SimulationType) == -1)
            return m;
        Py_INCREF(&SimulationType);
        PyModule_AddObject(m, "Simulation", (PyObject *)&SimulationType);
        return m;
    }
}
