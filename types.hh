#ifndef SIMWRAP_TYPES_HH
#define SIMWRAP_TYPES_HH

#include <Python.h>

namespace SimWrap
{
    // Simple wrapper around Python mapping types
    class Mapping
    {
    public:
        explicit Mapping(PyObject *map_);
        Mapping(const Mapping &other);
        ~Mapping();
        template <typename T>
        T get(const char *key) const;
        template <typename T>
        T get(const char *key, T default_value) const;
        template <typename T>
        void set(const char *key, T value) const;
        bool has_key(const char *key) const;
        PyObject *get_python_mapping() const;
    private:
        PyObject *const map;
        Mapping &operator=(const Mapping &other);
    };

    // Simple wrapper for Python functions
    class Function
    {
    public:
        explicit Function(PyObject *pyfunc_);
        Function(const Function &other);
        ~Function();
        template <typename RT>
        RT call();
        template <typename RT, typename T1>
        RT call(T1 arg1);
        template <typename RT, typename T1, typename T2>
        RT call(T1 arg1, T2 arg2);
        template <typename RT, typename T1, typename T2, typename T3>
        RT call(T1 arg1, T2 arg2, T3 arg3);
    private:
        PyObject *const pyfunc;
        Function &operator=(const Function &other);
    };

    // Convert basic Python types to the corresponding C++ type
    template <typename T>
    T convert_from_py(PyObject *obj);
    template <>
    inline void convert_from_py(PyObject *obj)
    {}
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
    template <>
    inline Mapping convert_from_py(PyObject *obj)
    {
        return Mapping(obj);
    }
    template <>
    inline Function convert_from_py(PyObject *obj)
    {
        return Function(obj);
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

    template <typename T>
    T eval_py_expr(const char *expr)
    {
        PyObject *globals = PyEval_GetGlobals();
        PyObject *locals = PyEval_GetLocals();
        if (!locals || !globals)
            throw ExceptionInPythonAPI();
        PyObject *result = PyRun_String(expr, Py_eval_input, globals, locals);
        if (!result)
            throw ExceptionInPythonAPI();
        return convert_from_py<T>(result);
    }

    // Implementation of Mapping
    template <typename T>
    T Mapping::get(const char *key) const
    {
        PyObject *obj = PyMapping_GetItemString(map, const_cast<char *>(key));
        if (!obj)
            throw ExceptionInPythonAPI();
        return convert_from_py<T>(obj);
    }
    template <typename T>
    T Mapping::get(const char *key, T default_value) const
    {
        if (has_key(key))
            return get<T>(key);
        return default_value;
    }
    template <typename T>
    void Mapping::set(const char *key, T value) const
    {
        if (PyMapping_SetItemString(
                map, const_cast<char *>(key), convert_to_py(value)) == -1)
            throw ExceptionInPythonAPI();
    }

    // Implementation of Function
    template <typename RT>
    RT Function::call()
    {
        return convert_from_py<RT>(PyObject_CallObject(pyfunc, 0));
    }
    template <typename RT, typename T1>
    RT Function::call(T1 arg1)
    {
        return convert_from_py<RT>(PyObject_CallFunctionObjArgs(
            pyfunc, convert_to_py(arg1), 0));
    }
    template <typename RT, typename T1, typename T2>
    RT Function::call(T1 arg1, T2 arg2)
    {
        return convert_from_py<RT>(PyObject_CallFunctionObjArgs(
            pyfunc, convert_to_py(arg1), convert_to_py(arg2), 0));
    }
    template <typename RT, typename T1, typename T2, typename T3>
    RT Function::call(T1 arg1, T2 arg2, T3 arg3)
    {
        return convert_from_py<RT>(PyObject_CallFunctionObjArgs(
            pyfunc, convert_to_py(arg1), convert_to_py(arg2),
            convert_to_py(arg3), 0));
    }
}

#endif
