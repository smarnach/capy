#ifndef SIMWRAP_CONVERSIONS_HH
#define SIMWRAP_CONVERSIONS_HH

#include <Python.h>

namespace SimWrap
{
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
}

#endif
