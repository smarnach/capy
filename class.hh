#ifndef CAPY_CLASS_HH
#define CAPY_CLASS_HH

namespace Capy
{
    template <typename Cls>
    class Class
    {
    public:
        const char *const type_name;

        struct ClsObject
        {
            PyObject_HEAD
            Cls *instance;
        };

        Class(const char *type_name_, Extension &ext, const char *doc = 0)
            : type_name(type_name_),
              extension(ext),
              type(new PyTypeObject),
              methods(new std::vector<PyMethodDef>),
              members(new std::vector<int Cls::*>),
              py_members(new std::vector<Object Cls::*>),
              getset(new std::vector<PyGetSetDef>)
        {
            memset(type, 0, sizeof(*type));
            Py_INCREF(type);
            char *qname =
                new char[strlen(extension.mod_name) + strlen(type_name) + 2];
            sprintf(qname, "%s.%s", extension.mod_name, type_name);
            type->tp_name = qname;
            type->tp_basicsize = sizeof(ClsObject);
            type->tp_dealloc = (destructor)dealloc;
            type->tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE |
                Py_TPFLAGS_HAVE_GC;
            type->tp_doc = doc;
            type->tp_traverse = (traverseproc)traverse;
            type->tp_base = 0; // XXX
            type->tp_new = new_;
            PyMethodDef meth = {0};
            methods->push_back(meth);
            PyGetSetDef gs = {0};
            getset->push_back(gs);
        }

        ~Class()
        {
            if (PyErr_Occurred())
                return;
            type->tp_methods = &methods->front();
            type->tp_getset = &getset->front();
            if (PyType_Ready(type) == -1)
                return;
            PyObject *py_members_cobj = PyCObject_FromVoidPtr(py_members, 0);
            if (!py_members_cobj)
                return;
            if (PyDict_SetItemString(type->tp_dict, "_capy_py_members",
                                     py_members_cobj) == -1) {
                Py_DECREF(py_members_cobj);
                return;
            }
            Py_DECREF(py_members_cobj);
            extension.add_object(type_name, Object((PyObject *)type));
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
            members->push_back((int Cls::*)memb);
            PyGetSetDef gs =
                {const_cast<char *>(name),
                 (getter)(PyObject *(*)(ClsObject *, T Cls::**))get_member<T>,
                 0, const_cast<char *>(doc), &members->back()};
            getset->insert(getset->end() - 1, gs);
        }
        template <typename T>
        void add_py_member(const char *name, T Cls::*memb,
                           const char *doc = 0, bool visible = true)
        {
            py_members->push_back((Object Cls::*)memb);
            if (!visible)
                return;
            PyGetSetDef gs =
                {const_cast<char *>(name),
                 (getter)(PyObject *(*)(ClsObject *, T Cls::**))get_member<T>,
                 0, const_cast<char *>(doc), &py_members->back()};
            getset->insert(getset->end() - 1, gs);
        }

    private:
        void add_method_def(const char *name, PyCFunction meth,
                            const char *doc)
        {
            PyMethodDef def = {name, meth, METH_VARARGS, doc};
            methods->insert(methods->end() - 1, def);
        }

        template <typename RT, RT (Cls::*method)()>
        static PyObject *
        call_method(PyObject *self_obj, PyObject *args)
        {
            ClsObject *self = (ClsObject *)self_obj;
            if (!PyArg_ParseTuple(args, ""))
                return 0;
            return Object((self->instance->*method)()).new_reference();
        }
        template <void (Cls::*method)()>
        static PyObject *
        call_method(PyObject *self_obj, PyObject *args)
        {
            ClsObject *self = (ClsObject *)self_obj;
            if (!PyArg_ParseTuple(args, ""))
                return 0;
            (self->instance->*method)();
            Py_RETURN_NONE;
        }
        template <typename RT, typename T, RT (Cls::*method)(T)>
        static PyObject *
        call_method(PyObject *self_obj, PyObject *args)
        {
            ClsObject *self = (ClsObject *)self_obj;
            PyObject *py_arg1;
            if (!PyArg_ParseTuple(args, "O", &py_arg1))
                return 0;
            return Object((self->instance->*method)
                          (Object(py_arg1).new_reference())).new_reference();
        }
        template <typename T, void (Cls::*method)(T)>
        static PyObject *
        call_method(PyObject *self_obj, PyObject *args)
        {
            ClsObject *self = (ClsObject *)self_obj;
            PyObject *py_arg1;
            if (!PyArg_ParseTuple(args, "O", &py_arg1))
                return 0;
            (self->instance->*method)(Object(py_arg1).new_reference());
            Py_RETURN_NONE;
        }
        template <typename RT, typename T1, typename T2, RT (Cls::*method)(T1, T2)>
        static PyObject *
        call_method(PyObject *self_obj, PyObject *args)
        {
            ClsObject *self = (ClsObject *)self_obj;
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
        call_method(PyObject *self_obj, PyObject *args)
        {
            ClsObject *self = (ClsObject *)self_obj;
            PyObject *py_arg1;
            PyObject *py_arg2;
            if (!PyArg_ParseTuple(args, "OO", &py_arg1, &py_arg2))
                return 0;
            (self->instance->*method)(Object(py_arg1).new_reference(),
                                      Object(py_arg2).new_reference());
            Py_RETURN_NONE;
        }

        static PyObject *
        new_helper(PyObject *self, PyObject *map)
        {
            Mapping config(map);
            try {
                ((ClsObject *)self)->instance = new Cls(config);
            }
            catch (...) {
                Py_DECREF(self);
                throw;
            }
            return self;
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
            PyObject *py_members_cobj = PyObject_GetAttrString(
                (PyObject *)Py_TYPE(self), "_capy_py_members");
            if (!py_members_cobj) {
                PyErr_Clear();
                return 0;
            }
            typedef std::vector<Object Cls::*> PyMembers;
            PyMembers py_members =
                *(PyMembers *)PyCObject_AsVoidPtr(py_members_cobj);
            for (unsigned i = 0; i < py_members.size(); ++i) {
                PyObject *ob = self->instance->*py_members[i];
                Py_VISIT(ob);
            }
            return 0;
        }

        Extension &extension;
        PyTypeObject *type;
        std::vector<PyMethodDef> *methods;
        std::vector<int Cls::*> *members;
        std::vector<Object Cls::*> *py_members;
        std::vector<PyGetSetDef> *getset;
    };
}

#endif
