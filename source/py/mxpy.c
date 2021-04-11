/*
This is an ongoing attempt to translate pdpython to maxmsp
from https://github.com/garthz/pdpython

/// pdpython.c : Pd external to bridge data in and out of Python
/// Copyright (c) 2014, Garth Zeglin.  All rights reserved.  Provided under the
/// terms of the BSD 3-clause license.

# LOG

- still not working on load..
- compiling without errors
- translated some parts to max
- renamed to mxpy.c
- removed some comments

*/

#include <Python.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <libgen.h>
#endif

#include "ext.h"
#include "ext_obex.h"


typedef struct _mxpy {
    t_object x_ob;       ///< standard object header
    void* x_outlet;      ///< left outlet for msg output
    PyObject* py_object; ///< Python class object represented by this object
} t_mxpy;

static t_class* mxpy_class;

static PyObject* t_atom_to_PyObject(t_atom* atom)
{
    switch (atom->a_type) {

    case A_FLOAT:
        return PyFloat_FromDouble(atom_getfloat(atom));

    case A_SYM:
        // symbols are returned as strings
        return PyUnicode_FromString(atom_getsym(atom)->s_name);

    case A_NOTHING:
        Py_RETURN_NONE;

    default:
        post("Warning: type %d unsupported for conversion to Python.",
             atom->a_type);
        Py_RETURN_NONE;
    }
}

static PyObject* t_atom_list_to_PyObject_list(int argc, t_atom* argv)
{
    PyObject* list = PyTuple_New(argc);
    int i;
    for (i = 0; i < argc; i++) {
        PyObject* value = t_atom_to_PyObject(&argv[i]);
        PyTuple_SetItem(list, i,
                        value); // pass the value reference to the tuple
    }
    return list;
}

static void PyObject_to_atom(PyObject* value, t_atom* atom)
{
    if (value == Py_True)
        atom_setlong(atom, 1);
    else if (value == Py_False)
        atom_setlong(atom, 0);
    else if (PyFloat_Check(value))
        atom_setfloat(atom, (float)PyFloat_AsDouble(value));
    else if (PyLong_Check(value))
        atom_setlong(atom, (float)PyLong_AsLong(value));
    else if (PyUnicode_Check(value))
        atom_setsym(atom, gensym(PyUnicode_AsUTF8(value)));
    else
        atom_setsym(atom, gensym("error"));
}

static void new_list_from_sequence(PyObject* seq, int* argc, t_atom** argv)
{
    Py_ssize_t len = 0;
    Py_ssize_t i;

    if (PyList_Check(seq)) {
        len = PyList_Size(seq);
        *argv = (t_atom*)malloc(len * sizeof(t_atom));
        for (i = 0; i < len; i++) {
            PyObject* elem = PyList_GetItem(seq, i);
            PyObject_to_atom(elem, (*argv) + i);
        }
    }
    *argc = (int)len;
}

static void emit_outlet_message(PyObject* value, void* x_outlet)
{
    if (value == Py_True)
        outlet_float(x_outlet, 1.0);
    else if (value == Py_False)
        outlet_float(x_outlet, 0.0);

    else if (PyFloat_Check(value))
        outlet_float(x_outlet, (float)PyFloat_AsDouble(value));
    else if (PyLong_Check(value))
        outlet_float(x_outlet, (float)PyLong_AsLong(value));
    else if (PyUnicode_Check(value))
        outlet_anything(x_outlet, gensym(PyUnicode_AsUTF8(value)), 0, NIL);

    else if (PyList_Check(value)) {
        t_atom* argv = NULL;
        int argc = 0;
        new_list_from_sequence(value, &argc, &argv);

        if (argc > 0) {
            if (argv[0].a_type == A_SYM) {
                outlet_anything(x_outlet, atom_getsym(&argv[0]), argc - 1,
                                argv + 1);
            } else {
                // outlet_list(x_outlet, &s_list, argc, argv);
                outlet_list(x_outlet, NULL, argc, argv);
            }
        }
        if (argv)
            free(argv);
    }
}

static void mxpy_eval(t_mxpy* x, t_symbol* selector, int argcount,
                          t_atom* argvec)
{
    PyObject* func = NULL;
    PyObject* args = NULL;
    PyObject* value = NULL;

    if (x->py_object == NULL) {
        post("Warning: message sent to uninitialized python object.");
        return;
    }

    func = PyObject_GetAttrString(x->py_object, selector->s_name);
    args = t_atom_list_to_PyObject_list(argcount, argvec);

    if (!func) {
        post("Warning: no Python function found for selector %s.",
             selector->s_name);
    } else {
        if (!PyCallable_Check(func)) {
            post("Warning: Python attribute for selector %s is not callable.",
                 selector->s_name);
        } else {
            value = PyObject_CallObject(func, args);
        }
        Py_DECREF(func);
    }

    if (args)
        Py_DECREF(args);

    if (value == NULL) {
        post("Warning: Python call for selector %s failed.", selector->s_name);

    } else {
        if (PyTuple_Check(value)) {
            // A tuple generates a sequence of outlet messages, one per item.
            int i, len = (int)PyTuple_Size(value);
            for (i = 0; i < len; i++) {
                PyObject* elem = PyTuple_GetItem(value, i);
                emit_outlet_message(elem, x->x_outlet);
            }
        } else {
            emit_outlet_message(value, x->x_outlet);
        }

        Py_DECREF(value);
    }
}


static void* mxpy_new(t_symbol* selector, int argcount, t_atom* argvec)
{
    
    t_mxpy* x = (t_mxpy*)object_alloc(mxpy_class);
    x->py_object = NULL;

    if (argcount < 2) {
        post("Error: python objects require a module and function specified "
             "in the creation arguments.");

    } else {

        // t_symbol* canvas_path = canvas_getcurrentdir();
        // PyObject* modulePath = PyUnicode_FromString(canvas_path->s_name);
        // post("modulepath: %s", canvas_path->s_name);

        // PyObject* sysPath = PySys_GetObject(
        //     (char*)"path"); // borrowed reference

        // if (!PySequence_Contains(sysPath, modulePath)) {
        //     post("Appending current canvas path to Python load path: %s",
        //          canvas_path->s_name);
        //     PyList_Append(sysPath, modulePath);
        // }
        // Py_DECREF(modulePath);


        // try loading the module
        PyObject* module_name = t_atom_to_PyObject(&argvec[0]);
        PyObject* module = PyImport_Import(module_name);
        Py_DECREF(module_name);

        if (module == NULL) {
            post("Error: unable to import Python module %s.",
                 atom_getsym(argvec + 0)->s_name);

        } else {
            PyObject* func = PyObject_GetAttrString(
                module, atom_getsym(argvec + 1)->s_name);

            if (func == NULL) {
                post("Error: Python function %s not found.",
                     atom_getsym(argvec + 1)->s_name);

            } else {
                if (!PyCallable_Check(func)) {
                    post("Error: Python attribute %s is not callable.",
                         atom_getsym(argvec + 1)->s_name);

                } else {
                    PyObject* args = t_atom_list_to_PyObject_list(argcount - 2,
                                                                  argvec + 2);
                    x->py_object = PyObject_CallObject(func, args);
                    Py_DECREF(args);
                }
                Py_DECREF(func);
            }
            Py_DECREF(module);
        }
    }

    // create an outlet on which to return values
    x->x_outlet = outlet_new(&x->x_ob, NULL);
    return (void*)x;
}
/****************************************************************/
/// Release an instance of a Pd 'python' object.
static void mxpy_free(t_mxpy* x)
{
    post("python freeing object");
    if (x) {
        object_free(x->x_outlet); // ????
        //outlet_free(x->x_outlet);
        if (x->py_object)
            Py_DECREF(x->py_object);
        x->x_outlet = NULL;
        x->py_object = NULL;
    }
}

static PyObject* mxgui_post(PyObject* self __attribute__((unused)),
                            PyObject* args)
{
    char* text;
    if (!PyArg_ParseTuple(args, "s", &text)) {
        post("Warning: unprintable object posted to the console from a python "
             "object.");
        return NULL;
    } else {
        post(text);
        Py_RETURN_NONE;
    }
}

static PyMethodDef mxgui_methods[] = { { "post", mxgui_post, METH_VARARGS,
                                         "Print a string to the Pd console." },
                                       { NULL, NULL, 0, NULL } };

static struct PyModuleDef mxguimodule = {
    PyModuleDef_HEAD_INIT, "mxgui", /* name of module */
    NULL,                           /* module documentation, may be NULL */
    -1, /* size of per-interpreter state of the module,
            or -1 if the module keeps state in global variables. */
    mxgui_methods
};

PyMODINIT_FUNC PyInit_mxgui(void) { return PyModule_Create(&mxguimodule); }

void ext_main(void* moduleRef)
{
    t_class* c;

    c = class_new("python",   // t_symbol *name
                    (method)mxpy_new,     // t_newmethod newmethod
                    (method)mxpy_free,    // t_method freemethod
                    (long)sizeof(t_mxpy), // size_t size
                    0,                    // int flags
                    A_GIMME, 0);          // t_atomtype arg1, ...

    class_addmethod(c, (method)mxpy_eval, "eval", A_GIMME, 0);
//    class_addanything(mxpy_class,
//                      (method)mxpy_eval); // (t_class *c, t_method fn)

    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    mxpy_class = c;

    wchar_t* program;
    program = Py_DecodeLocale("mxpy", NULL);
    if (program == NULL) {
        exit(1);
    }

    Py_SetProgramName(program);

    if (PyImport_AppendInittab("mxgui", PyInit_mxgui) == -1) {
        post("Error: unable to create the mxgui module.");
    }

    Py_Initialize();

    static wchar_t* arg0 = NULL;
    PySys_SetArgv(0, &arg0);

    post("completed: ext_main");


// #ifdef __APPLE__
//     wchar_t* python_home;
//     // char path[150];

//     // method 1
//     CFURLRef resources_url;
//     CFURLRef resources_abs_url;
//     CFStringRef resources_str;
//     const char* resources_path;

//     // Look for a bundle using its identifier
//     resources_url = CFBundleCopyResourcesDirectoryURL(moduleRef);
//     resources_abs_url = CFURLCopyAbsoluteURL(resources_url);
//     resources_str = CFURLCopyFileSystemPath(resources_abs_url,
//                                             kCFURLPOSIXPathStyle);
//     resources_path = CFStringGetCStringPtr(resources_str,
//                                            kCFStringEncodingUTF8);
//     python_home = Py_DecodeLocale(resources_path, NULL);

//     // CFRelease(resources_url);
//     // CFRelease(resources_abs_url);
//     CFRelease(resources_str);
//     // CFRelease(resources_path);

//     post("resources_path: %s", resources_path);

//     if (python_home == NULL) {
//         error("python_home is NULL");
//         // return;
//     }
//     Py_SetPythonHome(python_home);
// #endif
}
