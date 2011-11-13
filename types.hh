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
            check_error(obj);
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
            check_error(obj);
        }
        Object(double value)
            : obj(PyFloat_FromDouble(value))
        {
            check_error(obj);
        }
        Object(const char *value)
            : obj(PyString_FromString(value))
        {
            check_error(obj);
        }
        operator bool() const
        {
            return check_error(PyObject_IsTrue(obj));
        }
        operator long() const
        {
            return check_error(PyInt_AsLong(obj));
        }
        operator double() const
        {
            return check_error(PyFloat_AsDouble(obj));
        }
        operator const char *() const
        {
            return check_error(PyString_AsString(obj));
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
        bool callable() const
        {
            return PyCallable_Check(obj);
        }
        Object operator()() const
        {
            return Object(PyObject_CallObject(obj, 0));
        }
        Object operator()(Object arg1) const
        {
            return Object(PyObject_CallFunctionObjArgs(obj,
                                                       (PyObject *)arg1, 0));
        }
        Object operator()(Object arg1, Object arg2) const
        {
            return Object(PyObject_CallFunctionObjArgs(obj,
                                                       (PyObject *)arg1,
                                                       (PyObject *)arg2, 0));
        }
        Object operator()(Object arg1, Object arg2, Object arg3) const
        {
            return Object(PyObject_CallFunctionObjArgs(obj,
                                                       (PyObject *)arg1,
                                                       (PyObject *)arg2,
                                                       (PyObject *)arg3, 0));
        }
        Object operator()(Object arg1, Object arg2, Object arg3, Object arg4) const
        {
            return Object(PyObject_CallFunctionObjArgs(obj,
                                                       (PyObject *)arg1,
                                                       (PyObject *)arg2,
                                                       (PyObject *)arg3,
                                                       (PyObject *)arg4, 0));
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
        void set(const char *key, Object value) const
        {
            check_error(PyMapping_SetItemString(
                            obj, const_cast<char *>(key), value));
        }
        void del(const char *key) const
        {
            check_error(PyMapping_DelItemString(obj, const_cast<char *>(key)));
        }
        bool has_key(const char *key) const
        {
            return PyMapping_HasKeyString(obj, const_cast<char *>(key));
        }
    };

    // Simple wrapper around Python sequence types
    class Sequence : public Object
    {
    public:
        explicit Sequence(PyObject *obj_)
            : Object(obj_)
        {
            if (!PySequence_Check(obj))
                throw TypeError("argument must be a sequence");
        }
        Sequence(const Object &other)
            : Object(other)
        {
            if (!PySequence_Check(obj))
                throw TypeError("argument must be a sequence");
        }
        Object get(ssize_t item) const
        {
            return Object(PySequence_GetItem(obj, item));
        }
        void set(ssize_t item, Object value) const
        {
            check_error(PySequence_SetItem(obj, item, value));
        }
        void del(ssize_t item) const
        {
            check_error(PySequence_DelItem(obj, item));
        }
    };
}

#endif
