#ifndef SIMWRAP_TYPES_HH
#define SIMWRAP_TYPES_HH

#include <Python.h>

namespace SimWrap
{
    // Simple wrapper around Python objects, implicitly convertible from
    // and to basic C++ types
    class Object
    {
    public:
        explicit Object(PyObject *obj_)
            : obj(obj_)
        {
            if (!obj)
                throw ExceptionInPythonAPI();
        }
        Object(const Object &other)
            : obj(other.obj)
        {
            Py_INCREF(obj);
        }
        ~Object()
        {
            Py_DECREF(obj);
        }
        Object(bool value)
            : obj(PyBool_FromLong(value))
        {}
        Object(long value)
            : obj(PyInt_FromLong(value))
        {
            if (!obj)
                throw ExceptionInPythonAPI();
        }
        Object(double value)
            : obj(PyFloat_FromDouble(value))
        {
            if (!obj)
                throw ExceptionInPythonAPI();
        }
        Object(const char *value)
            : obj(PyString_FromString(value))
        {
            if (!obj)
                throw ExceptionInPythonAPI();
        }
        operator bool() const
        {
            bool res = PyObject_IsTrue(obj);
            if (res == -1)
                throw ExceptionInPythonAPI();
            return res;
        }
        operator long() const
        {
            long res = PyInt_AsLong(obj);
            if (res == -1 && PyErr_Occurred())
                throw ExceptionInPythonAPI();
            return res;
        }
        operator double() const
        {
            double res = PyFloat_AsDouble(obj);
            if (res == -1 && PyErr_Occurred())
                throw ExceptionInPythonAPI();
            return res;
        }
        operator const char *() const
        {
            const char *res = PyString_AsString(obj);
            if (!res)
                throw ExceptionInPythonAPI();
            return res;
        }
        operator PyObject *() const
        {
            return obj;
        }
        PyObject *new_reference() const
        {
            Py_INCREF(obj);
            return obj;
        }
    protected:
        PyObject *const obj;
    };

    // Simple wrapper around Python mapping types
    class Mapping : public Object
    {
    public:
        explicit Mapping(PyObject *obj_)
            : Object(obj_)
        {
            if (!PyMapping_Check(obj))
                throw TypeError("argument must be a mapping");
        }
        Mapping(const Object &other)
            : Object(other)
        {
            if (!PyMapping_Check(obj))
                throw TypeError("argument must be a mapping");
        }
        Object get(const char *key) const
        {
            return Object(PyMapping_GetItemString(obj, const_cast<char *>(key)));
        }
        template <typename T>
        T get(const char *key, T default_value) const
        {
            if (has_key(key))
                return get(key);
            return default_value;
        }
        void set(const char *key, const Object &value) const
        {
            if (PyMapping_SetItemString(
                    obj, const_cast<char *>(key), value) == -1)
                throw ExceptionInPythonAPI();
        }
        bool has_key(const char *key) const
        {
            return PyMapping_HasKeyString(obj, const_cast<char *>(key));
        }
    };

    // Simple wrapper for Python functions
    class Function : public Object
    {
    public:
        explicit Function(PyObject *obj_)
            : Object(obj_)
        {
            if (!PyCallable_Check(obj))
                throw TypeError("argument must be callable");
        }
        Function(const Object &other)
            : Object(other)
        {
            if (!PyCallable_Check(obj))
                throw TypeError("argument must be callable");
        }
        Object call()
        {
            return Object(PyObject_CallObject(obj, 0));
        }
        Object call(Object arg1)
        {
            return Object(PyObject_CallFunctionObjArgs(obj,
                                                       (PyObject *)arg1, 0));
        }
        Object call(Object arg1, Object arg2)
        {
            return Object(PyObject_CallFunctionObjArgs(obj,
                                                       (PyObject *)arg1,
                                                       (PyObject *)arg2, 0));
        }
        Object call(Object arg1, Object arg2, Object arg3)
        {
            return Object(PyObject_CallFunctionObjArgs(obj,
                                                       (PyObject *)arg1,
                                                       (PyObject *)arg2,
                                                       (PyObject *)arg3, 0));
        }
        Object call(Object arg1, Object arg2, Object arg3, Object arg4)
        {
            return Object(PyObject_CallFunctionObjArgs(obj,
                                                       (PyObject *)arg1,
                                                       (PyObject *)arg2,
                                                       (PyObject *)arg3,
                                                       (PyObject *)arg4, 0));
        }
    };

    inline Object eval_py_expr(const char *expr)
    {
        PyObject *globals = PyEval_GetGlobals();
        PyObject *locals = PyEval_GetLocals();
        if (!locals || !globals)
            throw ExceptionInPythonAPI();
        return Object(PyRun_String(expr, Py_eval_input, globals, locals));
    }
}

#endif
