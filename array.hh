#ifndef CAPY_NUMPY_HH
#define CAPY_NUMPY_HH

#include "capy.hh"
#include <numpy/arrayobject.h>

namespace Capy
{
    template <typename T>
    struct NumpyTypeCode
    {};
    template <>
    struct NumpyTypeCode<bool>
    {
        static const int value = NPY_BOOL;
    };
    template <>
    struct NumpyTypeCode<npy_byte>
    {
        static const int value = NPY_BYTE;
    };
    template <>
    struct NumpyTypeCode<npy_ubyte>
    {
        static const int value = NPY_UBYTE;
    };
    template <>
    struct NumpyTypeCode<npy_short>
    {
        static const int value = NPY_SHORT;
    };
    template <>
    struct NumpyTypeCode<npy_ushort>
    {
        static const int value = NPY_USHORT;
    };
    template <>
    struct NumpyTypeCode<npy_int>
    {
        static const int value = NPY_INT;
    };
    template <>
    struct NumpyTypeCode<npy_uint>
    {
        static const int value = NPY_UINT;
    };
    template <>
    struct NumpyTypeCode<npy_long>
    {
        static const int value = NPY_LONG;
    };
    template <>
    struct NumpyTypeCode<npy_ulong>
    {
        static const int value = NPY_ULONG;
    };
    template <>
    struct NumpyTypeCode<npy_longlong>
    {
        static const int value = NPY_LONGLONG;
    };
    template <>
    struct NumpyTypeCode<npy_ulonglong>
    {
        static const int value = NPY_ULONGLONG;
    };
    template <>
    struct NumpyTypeCode<npy_float>
    {
        static const int value = NPY_FLOAT;
    };
    template <>
    struct NumpyTypeCode<npy_double>
    {
        static const int value = NPY_DOUBLE;
    };
    template <>
    struct NumpyTypeCode<npy_longdouble>
    {
        static const int value = NPY_LONGDOUBLE;
    };

    class Array : public Object
    {
    public:
        explicit Array(PyObject *self_)
            : Object(self_)
        {
            if (!PyArray_Check(self))
                throw TypeError("argument must be a numpy.ndarray instance");
        }
        Array(const Object &other)
            : Object(other)
        {
            if (!PyArray_Check(self))
                throw TypeError("argument must be a numpy.ndarray instance");
        }
        template <typename T>
        Array(T *data, int nd, npy_intp *dims)
            : Object(PyArray_SimpleNewFromData(
                         nd, dims, NumpyTypeCode<T>::value, data))
        {}
        template <typename T>
        Array(T *data, npy_intp size)
            : Object(PyArray_SimpleNewFromData(
                         1, &size, NumpyTypeCode<T>::value, data))
        {}
        template <typename T>
        T *data()
        {
            return static_cast<T *>(PyArray_DATA(self));
        }
        int ndim()
        {
            return PyArray_NDIM(self);
        }
        npy_intp *dims()
        {
            return PyArray_DIMS(self);
        }
        npy_intp *strides()
        {
            return PyArray_STRIDES(self);
        }
    };
}

#endif
