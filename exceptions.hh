#ifndef SIMWRAP_EXCEPTIONS_HH
#define SIMWRAP_EXCEPTIONS_HH

#include <Python.h>
#include <exception>

// This header reproduces part of the Python exception hierarchy for
// use by C++ code.  Throwing one of these exceptions will result in
// the corresponding Python exception being raised in the Python
// interpreter.

namespace SimWrap
{
    // Exception occured in Python API function.  We don't need any
    // further information as we can rely on Python's exception
    // information.
    class ExceptionInPythonAPI : public std::exception {};

    class Exception :  public std::exception
    {
    public:
        Exception(const char *msg_, const PyObject *pyexc_ = PyExc_Exception)
            : msg(msg_), pyexc(pyexc_) {}
        const char *what() const throw()
        {
            return msg;
        }
        void raise() const throw()
        {
            PyErr_SetString(const_cast<PyObject *>(pyexc), msg);
        }
    private:
        const char *msg;
        const PyObject *pyexc;
    };

    class StandardError : public Exception
    {
    public:
        StandardError(const char *msg_, const PyObject *pyexc_ = PyExc_StandardError)
            : Exception(msg_, pyexc_) {}
    };

    class ArithmeticError : public StandardError
    {
    public:
        ArithmeticError(const char *msg_, const PyObject *pyexc_ = PyExc_ArithmeticError)
            : StandardError(msg_, pyexc_) {}
    };

    class FloatingPointError : public ArithmeticError
    {
    public:
        FloatingPointError(const char *msg_, const PyObject *pyexc_ = PyExc_FloatingPointError)
            : ArithmeticError(msg_, pyexc_) {}
    };

    class OverflowError : public ArithmeticError
    {
    public:
        OverflowError(const char *msg_, const PyObject *pyexc_ = PyExc_OverflowError)
            : ArithmeticError(msg_, pyexc_) {}
    };

    class ZeroDivisionError : public ArithmeticError
    {
    public:
        ZeroDivisionError(const char *msg_, const PyObject *pyexc_ = PyExc_ZeroDivisionError)
            : ArithmeticError(msg_, pyexc_) {}
    };

    class AssertionError : public StandardError
    {
    public:
        AssertionError(const char *msg_, const PyObject *pyexc_ = PyExc_AssertionError)
            : StandardError(msg_, pyexc_) {}
    };

    class EnvironmentError : public StandardError
    {
    public:
        EnvironmentError(const char *msg_, const PyObject *pyexc_ = PyExc_EnvironmentError)
            : StandardError(msg_, pyexc_) {}
    };

    class IOError : public EnvironmentError
    {
    public:
        IOError(const char *msg_, const PyObject *pyexc_ = PyExc_IOError)
            : EnvironmentError(msg_, pyexc_) {}
    };

    class OSError : public EnvironmentError
    {
    public:
        OSError(const char *msg_, const PyObject *pyexc_ = PyExc_OSError)
            : EnvironmentError(msg_, pyexc_) {}
    };

    class MemoryError : public StandardError
    {
    public:
        MemoryError(const char *msg_, const PyObject *pyexc_ = PyExc_MemoryError)
            : StandardError(msg_, pyexc_) {}
    };

    class RuntimeError : public StandardError
    {
    public:
        RuntimeError(const char *msg_, const PyObject *pyexc_ = PyExc_RuntimeError)
            : StandardError(msg_, pyexc_) {}
    };

    class NotImplementedError : public RuntimeError
    {
    public:
        NotImplementedError(const char *msg_, const PyObject *pyexc_ = PyExc_NotImplementedError)
            : RuntimeError(msg_, pyexc_) {}
    };

    class TypeError : public StandardError
    {
    public:
        TypeError(const char *msg_, const PyObject *pyexc_ = PyExc_TypeError)
            : StandardError(msg_, pyexc_) {}
    };

    class ValueError : public StandardError
    {
    public:
        ValueError(const char *msg_, const PyObject *pyexc_ = PyExc_ValueError)
            : StandardError(msg_, pyexc_) {}
    };
}

#endif