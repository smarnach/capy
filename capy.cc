#include <Python.h>
#include <exception>
#include <new>

#include "capy.hh"

namespace Capy
{
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
        Py_VISIT(self->simulation->config);
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
        return ((SimulationObject *)self)->simulation->config.new_reference();
    }

    static PyGetSetDef simulation_getset[] = {
        {(char *)"config", simulation_get_config, 0,
         (char *)"config attribute", 0},
        {0}  /* Sentinel */
    };

    static const char *simulation_doc =
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