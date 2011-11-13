#ifndef CAPY_HH
#define CAPY_HH

#include <Python.h>
#include <exception>
#include <new>

#include "exceptions.hh"
#include "types.hh"
#include "builtins.hh"

namespace Capy
{
    class Simulation
    {
    public:
        Simulation(const Mapping& config_)
            : config(config_)
        {}
        virtual ~Simulation()
        {}
        virtual void do_time_step(double time_step)
        {}
        virtual void write_output(const char *filename)
        {}
        Mapping config;
    };

    typedef struct
    {
        PyObject_HEAD
        Simulation *simulation;
    } SimulationObject;

    template <PyObject *(*f)(SimulationObject *, PyObject *)>
    PyObject *
    check_call(SimulationObject *self, PyObject *args)
    {
        try {
            return f(self, args);
        }
        catch (ExceptionInPythonAPI &e) {}
        catch (Exception &e) {
            e.raise();
        }
        catch (std::bad_alloc &e) {
            MemoryError("Failed memory allocation in C++ code").raise();
        }
        catch (std::exception &e) {
            RuntimeError(e.what()).raise();
        }
        catch (...) {
            RuntimeError("Unknown C++ exception occurred").raise();
        }
        return 0;
    }

    template <typename RT, RT (Simulation::*method)()>
    PyObject *
    call_method(SimulationObject *self, PyObject *args)
    {
        if (!PyArg_ParseTuple(args, ""))
            return 0;
        return Object((self->simulation->*method)());
    }

    template <void (Simulation::*method)()>
    PyObject *
    call_method(SimulationObject *self, PyObject *args)
    {
        if (!PyArg_ParseTuple(args, ""))
            return 0;
        (self->simulation->*method)();
        Py_RETURN_NONE;
    }

    template <typename RT, typename T, RT (Simulation::*method)(T)>
    PyObject *
    call_method(SimulationObject *self, PyObject *args)
    {
        PyObject *py_arg1;
        if (!PyArg_ParseTuple(args, "O", &py_arg1))
            return 0;
        return Object((self->simulation->*method)
                      (Object(py_arg1).new_reference()));
    }

    template <typename T, void (Simulation::*method)(T)>
    PyObject *
    call_method(SimulationObject *self, PyObject *args)
    {
        PyObject *py_arg1;
        if (!PyArg_ParseTuple(args, "O", &py_arg1))
            return 0;
        (self->simulation->*method)(Object(py_arg1).new_reference());
        Py_RETURN_NONE;
    }

    template <typename RT, RT (Simulation::*method)()>
    PyObject *
    wrap_method(PyObject *self, PyObject *args)
    {
        return check_call<call_method<RT, method> >(
            (SimulationObject *)self, args);
    }

    template <void (Simulation::*method)()>
    PyObject *
    wrap_method(PyObject *self, PyObject *args)
    {
        return check_call<call_method<method> >(
            (SimulationObject *)self, args);
    }

    template <typename RT, typename T, RT (Simulation::*method)(T)>
    PyObject *
    wrap_method(PyObject *self, PyObject *args)
    {
        return check_call<call_method<RT, T, method> >(
            (SimulationObject *)self, args);
    }

    template <typename T, void (Simulation::*method)(T)>
    PyObject *
    wrap_method(PyObject *self, PyObject *args)
    {
        return check_call<call_method<T, method> >(
            (SimulationObject *)self, args);
    }

    template <typename Sim>
    PyObject *
    simulation_new_helper(SimulationObject *self, PyObject *map)
    {
        Mapping config(map);
        try {
            self->simulation = new Sim(config);
        }
        catch (...) {
            Py_DECREF(self);
            throw;
        }
        return (PyObject *)self;
    }

    template <typename Sim>
    PyObject *
    simulation_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
    {
        PyObject *map = 0;
        if (!PyArg_ParseTuple(args, "|O:Simulation", &map))
            return 0;
        if (!map && !kwargs) {
            map = PyDict_New();
            if (!map)
                return 0;
        }
        else {
            if (map && kwargs)
                if (!PyObject_CallMethod(map, (char *)"update",
                                         (char *)"O", kwargs))
                    return 0;
            if (!map)
                map = kwargs;
            Py_INCREF(map);
        }
        SimulationObject *sim = (SimulationObject *)type->tp_alloc(type, 0);
        if (!sim) {
            Py_DECREF(map);
            return 0;
        }
        return check_call<simulation_new_helper<Sim> >(sim, map);
    }

    extern PyTypeObject SimulationType;

    PyObject *init_simulation_module(const char *name, const char *doc,
                                     PyMethodDef *methods = 0);

    template <typename Sim> void
    add_simulation_type(PyObject *module, const char *type_name, const char *doc,
        PyMethodDef *methods = 0)
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
        new_type->tp_methods = methods;
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

#endif
