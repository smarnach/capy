#ifndef CAPY_HH
#define CAPY_HH

#include <Python.h>
#include <structmember.h>
#include <unistd.h>
#include <exception>
#include <new>

#include "exceptions.hh"
#include "types.hh"
#include "builtins.hh"

namespace Capy
{
    template <typename Cls>
    class Class
    {
    public:
        struct ClsObject
        {
            PyObject_HEAD
            Cls *instance;
        };

        Class(const char *type_name, const char *doc = 0)
        {
            memset(&type, 0, sizeof(type));
            Py_INCREF(&type);
            type.tp_name = type_name;
            type.tp_basicsize = sizeof(ClsObject);
            type.tp_dealloc = (destructor)dealloc;
            type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE |
                Py_TPFLAGS_HAVE_GC;
            type.tp_doc = doc;
            type.tp_traverse = (traverseproc)traverse;
            type.tp_base = 0; // XXX
            type.tp_new = new_;
            PyMethodDef meth = {0};
            methods.push_back(meth);
            PyGetSetDef gs = {0};
            getset.push_back(gs);
        }

        ~Class()
        {
            // XXX
            fprintf(stderr, "Warning: Class instance should be 'static'.\n");
        }

        template <typename RT, RT (Cls::*method)()>
        void add_method(const char *name, const char *doc = 0)
        {
            add_method_def(name, check_call<Class::call_method<RT, method> >, doc);
        }
        template <void (Cls::*method)()>
        void add_method(const char *name, const char *doc = 0)
        {
            add_method_def(name, check_call<Class::call_method<method> >, doc);
        }
        template <typename RT, typename T, RT (Cls::*method)(T)>
        void add_method(const char *name, const char *doc = 0)
        {
            add_method_def(name, check_call<Class::call_method<RT, T, method> >, doc);
        }
        template <typename T, void (Cls::*method)(T)>
        void add_method(const char *name, const char *doc = 0)
        {
            add_method_def(name, check_call<Class::call_method<T, method> >, doc);
        }
        template <typename RT, typename T1, typename T2, RT (Cls::*method)(T1, T2)>
        void add_method(const char *name, const char *doc = 0)
        {
            add_method_def(name, check_call<Class::call_method<RT, T1, T2, method> >, doc);
        }
        template <typename T1, typename T2, void (Cls::*method)(T1, T2)>
        void add_method(const char *name, const char *doc = 0)
        {
            add_method_def(name, check_call<Class::call_method<T1, T2, method> >, doc);
        }

        template <typename T>
        void add_member(const char *name, T Cls::*memb, const char *doc = 0)
        {
            members.push_back((int Cls::*)memb);
            PyGetSetDef gs =
                {const_cast<char *>(name),
                 (getter)(PyObject *(*)(ClsObject *, T Cls::**))get_member<T>,
                 0, const_cast<char *>(doc), &members.back()};
            getset.insert(getset.end() - 1, gs);
        }

        int add_to(PyObject *module)
        {
            if (!module)
                return -1;
            const char *type_name = type.tp_name;
            const char *mod_name = PyModule_GetName(module);
            if (mod_name)
            {
                sprintf(qname, "%s.%s", mod_name, type_name);
                type.tp_name = qname;
            }
            else
                PyErr_Clear();
            type.tp_methods = &methods[0];
            type.tp_getset = &getset[0];
            if (PyType_Ready(&type) == -1)
                return -1;
            return PyModule_AddObject(module, type_name, (PyObject *)(&type));
        }

    private:
        void add_method_def(const char *name, PyCFunction meth,
                            const char *doc)
        {
            PyMethodDef def = {name, meth, METH_VARARGS, doc};
            methods.insert(methods.end() - 1, def);
        }

        template <PyObject *(*f)(ClsObject *, PyObject *)>
        static PyObject *
        check_call(PyObject *self, PyObject *args)
        {
            try {
                return f((ClsObject *)self, args);
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

        template <typename RT, RT (Cls::*method)()>
        static PyObject *
        call_method(ClsObject *self, PyObject *args)
        {
            if (!PyArg_ParseTuple(args, ""))
                return 0;
            return Object((self->instance->*method)()).new_reference();
        }
        template <void (Cls::*method)()>
        static PyObject *
        call_method(ClsObject *self, PyObject *args)
        {
            if (!PyArg_ParseTuple(args, ""))
                return 0;
            (self->instance->*method)();
            Py_RETURN_NONE;
        }
        template <typename RT, typename T, RT (Cls::*method)(T)>
        static PyObject *
        call_method(ClsObject *self, PyObject *args)
        {
            PyObject *py_arg1;
            if (!PyArg_ParseTuple(args, "O", &py_arg1))
                return 0;
            return Object((self->instance->*method)
                          (Object(py_arg1).new_reference())).new_reference();
        }
        template <typename T, void (Cls::*method)(T)>
        static PyObject *
        call_method(ClsObject *self, PyObject *args)
        {
            PyObject *py_arg1;
            if (!PyArg_ParseTuple(args, "O", &py_arg1))
                return 0;
            (self->instance->*method)(Object(py_arg1).new_reference());
            Py_RETURN_NONE;
        }
        template <typename RT, typename T1, typename T2, RT (Cls::*method)(T1, T2)>
        static PyObject *
        call_method(ClsObject *self, PyObject *args)
        {
            PyObject *py_arg1;
            PyObject *py_arg2;
            if (!PyArg_ParseTuple(args, "OO", &py_arg1, &py_arg2))
                return 0;
            return Object((self->instance->*method)
                          (Object(py_arg1).new_reference(),
                           Object(py_arg2).new_reference())).new_reference();
        }
        template <typename T1, typename T2, void (Cls::*method)(T1, T2)>
        static PyObject *
        call_method(ClsObject *self, PyObject *args)
        {
            PyObject *py_arg1;
            PyObject *py_arg2;
            if (!PyArg_ParseTuple(args, "OO", &py_arg1, &py_arg2))
                return 0;
            (self->instance->*method)(Object(py_arg1).new_reference(),
                                      Object(py_arg2).new_reference());
            Py_RETURN_NONE;
        }

        static PyObject *
        new_helper(ClsObject *self, PyObject *map)
        {
            Mapping config(map);
            try {
                self->instance = new Cls(config);
            }
            catch (...) {
                Py_DECREF(self);
                throw;
            }
            return (PyObject *)self;
        }
        static PyObject *
        new_(PyTypeObject *type, PyObject *args, PyObject *kwargs)
        {
            PyObject *map = 0;
            if (!PyArg_ParseTuple(args, "|O", &map))
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
            PyObject *instance = type->tp_alloc(type, 0);
            if (!instance) {
                Py_DECREF(map);
                return 0;
            }
            return check_call<Class::new_helper>(instance, map);
        }

        template <typename T>
        static PyObject *get_member(ClsObject *self, T Cls::**memb)
        {
            try {
                return Object(self->instance->**memb).new_reference();
            }
            catch (ExceptionInPythonAPI&) {
                return 0;
            }
        }

        static void
        dealloc(ClsObject *self)
        {
            delete self->instance;
            PyObject_GC_UnTrack(self);
            self->ob_type->tp_free((PyObject *)self);
        }

        static int
        traverse(ClsObject *self, visitproc visit, void *arg)
        {
            // XXX
            return 0;
        }

        PyTypeObject type;
        char qname[256];
        std::vector<PyMethodDef> methods;
        std::vector<int Cls::*> members;
        std::vector<Object Cls::*> traverse_list;
        std::vector<PyGetSetDef> getset;
    };
}

#endif
