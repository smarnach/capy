#include <Python.h>
#include <exception>
#include <new>

#include "simwrap.hh"

namespace SimWrap
{
    // Implementation of Exception methods
    Exception::Exception(const char *msg_)
        : msg(msg_) {}
    const char *Exception::what() const throw()
    {
        return msg;
    }
    void Exception::raise() const throw()
    {
        PyErr_SetString(PyExc_Exception, msg);
    }

    //Implementation of TypeError methods
    TypeError::TypeError(const char *msg_)
        : Exception(msg_) {}
    void TypeError::raise() const throw()
    {
        PyErr_SetString(PyExc_TypeError, what());
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
        catch (ExceptionInPythonAPI e) {
            return 0;
        }
        Py_RETURN_NONE;
    }

    static PyMethodDef simulation_methods[] = {
        {"do_time_step", (PyCFunction)simulation_do_time_step, METH_VARARGS,
         "Run a single time step of the simulation."},
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
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
        simulation_doc,            /* tp_doc */
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
        simulation_new<Simulation>, /* tp_new */
    };

    static PyMethodDef simwrap_methods[] = {
        {0}  /* Sentinel */
    };

    PyObject *
    init_simulation_module(const char *mod_name, const char *doc)
    {
        PyObject *m = Py_InitModule3(mod_name, simwrap_methods, doc);
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
