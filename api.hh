#ifndef CAPY_BUILTINS_HH
#define CAPY_BUILTINS_HH

namespace Capy
{
    inline Object eval(const char *expr)
    {
        PyObject *globals = check_error(PyEval_GetGlobals());
        PyObject *locals = check_error(PyEval_GetLocals());
        return Object(PyRun_String(expr, Py_eval_input, globals, locals));
    }
    inline Object eval(const char *expr, Dict globals)
    {
        return Object(PyRun_String(expr, Py_eval_input, globals, globals));
    }
    inline Object eval(const char *expr, Dict globals, Mapping locals)
    {
        return Object(PyRun_String(expr, Py_eval_input, globals, locals));
    }
    inline void exec(const char *expr)
    {
        PyObject *globals = check_error(PyEval_GetGlobals());
        PyObject *locals = check_error(PyEval_GetLocals());
        check_error(PyRun_String(expr, Py_file_input, globals, locals));
    }
    inline void exec(const char *expr, Dict globals)
    {
        check_error(PyRun_String(expr, Py_file_input, globals, globals));
    }
    inline void exec(const char *expr, Dict globals, Mapping locals)
    {
        check_error(PyRun_String(expr, Py_file_input, globals, locals));
    }
    inline bool hasattr(Object obj, const char *attr)
    {
        return PyObject_HasAttrString(obj, attr);
    }
    inline Object getattr(Object obj, const char *attr)
    {
        return Object(PyObject_GetAttrString(obj, attr));
    }
    inline void setattr(Object obj, const char *attr, Object value)
    {
        check_error(PyObject_SetAttrString(obj, attr, value));
    }
    inline void delattr(Object obj, const char *attr)
    {
        check_error(PyObject_DelAttrString(obj, attr));
    }
    inline const char *repr(Object obj)
    {
        return Object(PyObject_Repr(obj));
    }
    inline const char *str(Object obj)
    {
        return Object(PyObject_Str(obj));
    }
    inline void print(Object obj)
    {
        check_error(PyObject_Print(obj, stdout, 0));
    }
    inline bool isinstance(Object obj, Object cls)
    {
        return check_error(PyObject_IsInstance(obj, cls));
    }
    inline bool issubclass(Object derived, Object cls)
    {
        return check_error(PyObject_IsSubclass(derived, cls));
    }
    inline long hash(Object obj)
    {
        return check_error(PyObject_Hash(obj));
    }
    inline Object type(Object obj)
    {
        return Object(PyObject_Type(obj));
    }
    inline ssize_t len(Object obj)
    {
        return check_error(PyObject_Size(obj));
    }
    inline ssize_t len(Mapping obj)
    {
        return check_error(PyMapping_Size(obj));
    }
    inline ssize_t len(Sequence obj)
    {
        return check_error(PySequence_Size(obj));
    }
    inline ssize_t len(List obj)
    {
        return check_error(PyList_Size(obj));
    }
    inline ssize_t len(Dict obj)
    {
        return check_error(PyDict_Size(obj));
    }
    inline bool callable(Object obj)
    {
        return PyCallable_Check(obj);
    }
}

#endif
