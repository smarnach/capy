#ifndef CAPY_EXTENSION_HH
#define CAPY_EXTENSION_HH

namespace Capy
{
    class Extension
    {
    public:
        const char *const mod_name;

        Extension(const char *mod_name_, const char *mod_doc_ = 0)
            : mod_name(mod_name_),
              mod_doc(mod_doc_),
              functions(new std::vector<PyMethodDef>)
        {
            PyMethodDef func = {0};
            functions->push_back(func);
        }

        ~Extension()
        {
            if (PyErr_Occurred())
                return;
            PyObject *module =
                Py_InitModule3(mod_name, &functions->front(), mod_doc);
            if (!module)
                return;
            for (unsigned i = 0; i < objects.size(); ++i)
                if (PyModule_AddObject(module, obj_names[i],
                                       objects[i].new_reference()) == -1)
                    return;
        }

        template <typename RT, RT (*func)()>
        void add_function(const char *name, const char *doc = 0)
        {
            add_function_def(name, check_call<call_function<RT, func> >, doc);
        }
        template <void (*func)()>
        void add_function(const char *name, const char *doc = 0)
        {
            add_function_def(name, check_call<call_function<func> >, doc);
        }
        template <typename RT, typename T, RT (*func)(T)>
        void add_function(const char *name, const char *doc = 0)
        {
            add_function_def(name, check_call<call_function<RT, T, func> >, doc);
        }
        template <typename T, void (*func)(T)>
        void add_function(const char *name, const char *doc = 0)
        {
            add_function_def(name, check_call<call_function<T, func> >, doc);
        }
        template <typename RT, typename T1, typename T2, RT (*func)(T1, T2)>
        void add_function(const char *name, const char *doc = 0)
        {
            add_function_def(name, check_call<call_function<RT, T1, T2, func> >, doc);
        }
        template <typename T1, typename T2, void (*func)(T1, T2)>
        void add_function(const char *name, const char *doc = 0)
        {
            add_function_def(name, check_call<call_function<T1, T2, func> >, doc);
        }

        void add_object(const char *name, Object obj)
        {
            obj_names.push_back(name);
            objects.push_back(obj);
        }

    private:
        void add_function_def(const char *name, PyCFunction func,
                              const char *doc)
        {
            PyMethodDef def = {name, func, METH_VARARGS, doc};
            functions->insert(functions->end() - 1, def);
        }

        template <typename RT, RT (*func)()>
        static PyObject *
        call_function(PyObject *self, PyObject *args)
        {
            if (!PyArg_ParseTuple(args, ""))
                return 0;
            return Object(func()).new_reference();
        }
        template <void (*func)()>
        static PyObject *
        call_function(PyObject *self, PyObject *args)
        {
            if (!PyArg_ParseTuple(args, ""))
                return 0;
            func();
            Py_RETURN_NONE;
        }
        template <typename RT, typename T, RT (*func)(T)>
        static PyObject *
        call_function(PyObject *self, PyObject *args)
        {
            PyObject *py_arg1;
            if (!PyArg_ParseTuple(args, "O", &py_arg1))
                return 0;
            return Object(func(Object(py_arg1).new_reference())).new_reference();
        }
        template <typename T, void (*func)(T)>
        static PyObject *
        call_function(PyObject *self, PyObject *args)
        {
            PyObject *py_arg1;
            if (!PyArg_ParseTuple(args, "O", &py_arg1))
                return 0;
            func(Object(py_arg1).new_reference());
            Py_RETURN_NONE;
        }
        template <typename RT, typename T1, typename T2, RT (*func)(T1, T2)>
        static PyObject *
        call_function(PyObject *self, PyObject *args)
        {
            PyObject *py_arg1;
            PyObject *py_arg2;
            if (!PyArg_ParseTuple(args, "OO", &py_arg1, &py_arg2))
                return 0;
            return Object(func(Object(py_arg1).new_reference(),
                               Object(py_arg2).new_reference())).new_reference();
        }
        template <typename T1, typename T2, void (*func)(T1, T2)>
        static PyObject *
        call_function(PyObject *self, PyObject *args)
        {
            PyObject *py_arg1;
            PyObject *py_arg2;
            if (!PyArg_ParseTuple(args, "OO", &py_arg1, &py_arg2))
                return 0;
            func(Object(py_arg1).new_reference(), Object(py_arg2).new_reference());
            Py_RETURN_NONE;
        }

        const char *mod_doc;
        std::vector<PyMethodDef> *functions;
        std::vector<const char *> obj_names;
        std::vector<Object> objects;
    };
}

#endif
