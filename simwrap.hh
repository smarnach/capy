#ifndef SIMWRAP_HH
#define SIMWRAP_HH

#include <Python.h>
#include <exception>
#include <new>

#include "exceptions.hh"
#include "conversions.hh"

namespace SimWrap
{
    // Simple wrapper around Python mapping types
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

    template <PyCFunction f>
    PyObject *
    check_call(PyObject *self, PyObject *args)
    {
        try {
            return f(self, args);
        }
        catch (ExceptionInPythonAPI e) {
            return 0;
        }
        catch (Exception e) {
            e.raise();
            return 0;
        }
        catch (std::exception e) {
            RuntimeError(e.what()).raise();
            return 0;
        }
        catch (...) {
            RuntimeError("Unknown C++ exception occurred").raise();
            return 0;
        }
    }

    template <void (Simulation::*method)()>
    PyObject *
    call_method(PyObject *self, PyObject *args)
    {
        if (!PyArg_ParseTuple(args, ""))
            return 0;
        (((SimulationObject *)self)->simulation->*method)();
        Py_RETURN_NONE;
    }

    template <typename T, void (Simulation::*method)(T)>
    PyObject *
    call_method(PyObject *self, PyObject *args)
    {
        PyObject *py_arg1;
        if (!PyArg_ParseTuple(args, "O", &py_arg1))
            return 0;
        T arg1 = convert_from_py<T>(py_arg1);
        (((SimulationObject *)self)->simulation->*method)(arg1);
        Py_RETURN_NONE;
    }

    template <void (Simulation::*method)()>
    PyObject *
    wrap_method(PyObject *self, PyObject *args)
    {
        return check_call<call_method<method> >(self, args);
    }

    template <typename T, void (Simulation::*method)(T)>
    PyObject *
    wrap_method(PyObject *self, PyObject *args)
    {
        return check_call<call_method<T, method> >(self, args);
    }

    template <typename Sim>
    PyObject *
    simulation_new_helper(PyObject *self, PyObject *map)
    {
        Config config(map);
        ((SimulationObject *)self)->simulation = new(std::nothrow) Sim(config);
        if (!((SimulationObject *)self)->simulation) {
            Py_DECREF(self);
            PyErr_SetString(PyExc_MemoryError,
                            "could not allocate Simulation object");
            return 0;
        }
        return self;
    }

    template <typename Sim>
    PyObject *
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
        PyObject *sim = type->tp_alloc(type, 0);
        if (!sim)
            return 0;
        return check_call<simulation_new_helper<Sim> >(sim, map);
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

#endif
