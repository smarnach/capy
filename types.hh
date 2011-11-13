#ifndef CAPY_TYPES_HH
#define CAPY_TYPES_HH

#include <vector>

// This header contains wrappers around Python objects

namespace Capy
{
    // Wrapper around Python objects, implicitly convertible from and
    // to basic C++ types
    class Object
    {
    public:
        explicit Object(PyObject *self_)
            : self(self_)
        {
            check_error(self);
        }
        Object(const Object &other)
            : self(other.self)
        {
            Py_INCREF(self);
        }
        ~Object()
        {
            Py_DECREF(self);
        }
        Object(bool value)
            : self(PyBool_FromLong(value))
        {}
        Object(long value)
            : self(PyInt_FromLong(value))
        {
            check_error(self);
        }
        Object(double value)
            : self(PyFloat_FromDouble(value))
        {
            check_error(self);
        }
        Object(const char *value)
            : self(PyString_FromString(value))
        {
            check_error(self);
        }
        operator bool() const
        {
            return check_error(PyObject_IsTrue(self));
        }
        operator long() const
        {
            return check_error(PyInt_AsLong(self));
        }
        operator double() const
        {
            return check_error(PyFloat_AsDouble(self));
        }
        operator const char *() const
        {
            return check_error(PyString_AsString(self));
        }
        operator PyObject *() const
        {
            return self;
        }
        PyObject *new_reference() const
        {
            Py_INCREF(self);
            return self;
        }
        bool callable() const
        {
            return PyCallable_Check(self);
        }
        Object operator()() const
        {
            return Object(PyObject_CallObject(self, 0));
        }
        Object operator()(Object arg1) const
        {
            return Object(PyObject_CallFunctionObjArgs(self,
                                                       (PyObject *)arg1, 0));
        }
        Object operator()(Object arg1, Object arg2) const
        {
            return Object(PyObject_CallFunctionObjArgs(self,
                                                       (PyObject *)arg1,
                                                       (PyObject *)arg2, 0));
        }
        Object operator()(Object arg1, Object arg2, Object arg3) const
        {
            return Object(PyObject_CallFunctionObjArgs(self,
                                                       (PyObject *)arg1,
                                                       (PyObject *)arg2,
                                                       (PyObject *)arg3, 0));
        }
        Object operator()(Object arg1, Object arg2, Object arg3, Object arg4) const
        {
            return Object(PyObject_CallFunctionObjArgs(self,
                                                       (PyObject *)arg1,
                                                       (PyObject *)arg2,
                                                       (PyObject *)arg3,
                                                       (PyObject *)arg4, 0));
        }
    protected:
        PyObject *const self;
    };

    class Mapping : public Object
    {
    public:
        explicit Mapping(PyObject *self_)
            : Object(self_)
        {
            if (!PyMapping_Check(self))
                throw TypeError("argument must be a mapping");
        }
        Mapping(const Object &other)
            : Object(other)
        {
            if (!PyMapping_Check(self))
                throw TypeError("argument must be a mapping");
        }
        Object get(const char *key) const
        {
            return Object(PyMapping_GetItemString(self, const_cast<char *>(key)));
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
                            self, const_cast<char *>(key), value));
        }
        void del(const char *key) const
        {
            check_error(PyMapping_DelItemString(self, const_cast<char *>(key)));
        }
        bool has_key(const char *key) const
        {
            return PyMapping_HasKeyString(self, const_cast<char *>(key));
        }
    };

    class Dict : public Mapping
    {
    public:
        explicit Dict(PyObject *self_)
            : Mapping(self_)
        {
            if (!PyDict_Check(self))
                throw TypeError("argument must be a dictionary");
        }
        Dict(const Object &other)
            : Mapping(other)
        {
            if (!PyDict_Check(self))
                throw TypeError("argument must be a dictionary");
        }
        Dict()
            : Mapping(PyDict_New())
        {}
        void clear()
        {
            PyDict_Clear(self);
        }
    };

    class Sequence : public Object
    {
    public:
        explicit Sequence(PyObject *self_)
            : Object(self_)
        {
            if (!PySequence_Check(self))
                throw TypeError("argument must be a sequence");
        }
        Sequence(const Object &other)
            : Object(other)
        {
            if (!PySequence_Check(self))
                throw TypeError("argument must be a sequence");
        }
        Object get(ssize_t item) const
        {
            return Object(PySequence_GetItem(self, item));
        }
        void set(ssize_t item, Object value) const
        {
            check_error(PySequence_SetItem(self, item, value));
        }
        void del(ssize_t item) const
        {
            check_error(PySequence_DelItem(self, item));
        }
    };

    class List : public Sequence
    {
    public:
        explicit List(PyObject *self_)
            : Sequence(self_)
        {
            if (!PyList_Check(self))
                throw TypeError("argument must be a list");
        }
        List(const Object &other)
            : Sequence(other)
        {
            if (!PyList_Check(self))
                throw TypeError("argument must be a list");
        }
        List()
            : Sequence(PyList_New(0))
        {}
        template <typename T>
        List(const std::vector<T> &v)
            : Sequence(PyList_New(v.size()))
        {
            for (size_t i = 0; i < v.size(); ++i)
                PyList_SET_ITEM(self, i, Object(v[i]).new_reference());
        }
        template <typename T>
        void as_std_vector(std::vector<T> &v)
        {
            v.clear();
            v.resize(Py_SIZE(self));
            for (size_t i = 0; i < v.size(); ++i)
            {
                Object item(PyList_GET_ITEM(self, i));
                item.new_reference();
                v[i] = item;
            }
        }
        void insert(ssize_t index, Object value)
        {
            check_error(PyList_Insert(self, index, value));
        }
        void append(Object value)
        {
            check_error(PyList_Append(self, value));
        }
        void sort()
        {
            check_error(PyList_Sort(self));
        }
        void reverse()
        {
            check_error(PyList_Reverse(self));
        }
    };
}

#endif
