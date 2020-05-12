/**
    py.c - basic experiment in minimal max object for calling python code

    repo - github.com/shakfu/py

    This object has 1 inlet and 2 outlets

    Basic Features

    1.  Per-Object Namespace. It responds to an 'import <module>' message in 
        the left inlet which loads a python module in its namespace.

    2.  Eval Messages. It responds to an 'eval <expression>' message in the left inlet
        which is evaluated in the namespace and outputs results to the left outlet
        and outputs a bang from the right outlet to signal end of evaluation.

    py interpreter object
        @import <module>
        @eval <code>

        (phase 1)
        @run <script>

        (phase 2)
        @load <script>
        @code <stored code>

    TODO

        - [ ] add right inlet bang after eval op ends
        - [ ] add @run <script>
        - [ ] add text edit object


*/

#include "ext.h"            // you must include this - it contains the external object's link to available Max functions
#include "ext_obex.h"       // this is required for all objects using the newer style for writing objects.

/* python */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#define NOT_STRING(x) (!PyBytes_Check(x) && !PyByteArray_Check(x) && !PyUnicode_Check(x))

typedef struct _py {
    t_object p_ob;          // object header - ALL objects MUST begin with this...
    t_symbol *p_module;     // python import: additional imports
    t_symbol *p_code;       // python code to evaluate to default outlet
    long p_value0;          // int value - received from the left inlet and stored internally for each object instance
    long p_value1;          // int value - received from the right inlet and stored internally for each object instance
    void *p_outlet;         // outlet creation - inlets are automatic, but objects must "own" their own outlets
} t_py;


// these are prototypes for the methods that are defined below
void py_bang(t_py *x);
void py_import(t_py *x, t_symbol *s);
void py_eval(t_py *x, t_symbol *s, long argc, t_atom *argv);
void py_run(t_py *x, t_symbol *s, long argc, t_atom *argv);
void py_int(t_py *x, long n);
void py_in1(t_py *x, long n);
void py_dblclick(t_py *x);
void py_assist(t_py *x, void *b, long m, long a, char *s);

void *py_new(t_symbol *s, long argc, t_atom *argv);
void py_free(t_py *x);

t_class *py_class;      // global pointer to the object class - so max can reference the object


/*--------------------------------------------------------------------------*/

void ext_main(void *r)
{
    t_class *c;

    c = class_new("py", (method)py_new, (method)py_free, (long)sizeof(t_py),
                  0L /* leave NULL!! */, A_GIMME, 0);

    // methods
    class_addmethod(c, (method)py_bang,    "bang",      0);
    class_addmethod(c, (method)py_import,  "import",    A_DEFSYM, 0);
    class_addmethod(c, (method)py_eval,    "anything",  A_GIMME, 0);
    class_addmethod(c, (method)py_eval,    "run",       A_GIMME, 0);
    class_addmethod(c, (method)py_int,     "int",       A_LONG, 0);
    class_addmethod(c, (method)py_in1,     "in1",       A_LONG, 0);

    /* you CAN'T call this from the patcher */
    class_addmethod(c, (method)py_dblclick, "dblclick", A_CANT, 0);
    class_addmethod(c, (method)py_assist,   "assist",   A_CANT, 0);

    // attributes
    CLASS_ATTR_SYM(c, "module", 0, t_py, p_module);
    CLASS_ATTR_BASIC(c, "module", 0);

    CLASS_ATTR_SYM(c, "code",    0, t_py, p_code);
    CLASS_ATTR_BASIC(c, "code", 0);


    class_register(CLASS_BOX, c);
    py_class = c;

    post("py object loaded...",0);  // post any important info to the max window when our class is loaded
}


/*--------------------------------------------------------------------------*/

void *py_new(t_symbol *s, long argc, t_atom *argv)
{
    t_py *x;

    x = (t_py *)object_alloc(py_class);

    if (x) {
        x->p_module = gensym("");
        x->p_code = gensym("");
        x->p_value0 = 0;
        x->p_value1 = 0;

        // create inlet(s)
        intin(x,1);      // create a second int inlet (leftmost inlet is automatic - all objects have one inlet by default)
        // create outlet
        x->p_outlet = outlet_new(x, NULL);
        
        // process @arg attributes
        attr_args_process(x, argc, argv);
    }    
    //post("new py object instance added to patch...", 0);
    return(x);
}


void py_free(t_py *x)
{
    ;
}


//--------------------------------------------------------------------------

void py_assist(t_py *x, void *b, long m, long a, char *s) // 4 final arguments are always the same for the assistance method
{
    if (m == ASSIST_OUTLET)
        sprintf(s,"Sum of Left and Right Inlets");
    else {
        switch (a) {
        case 0:
            sprintf(s,"Inlet %ld: Left Operand (Causes Output)", a);
            break;
        case 1:
            sprintf(s,"Inlet %ld: Right Operand (Added to Left)", a);
            break;
        }
    }
}


void py_dblclick(t_py *x)
{
    object_post((t_object *)x, "I got a double-click");
}


void py_import(t_py *x, t_symbol *s) {
    x->p_module = s;
    post("import: %s", x->p_module->s_name);
}

void py_run(t_py *x, t_symbol *s, long argc, t_atom *argv) {
    ;
}

void py_eval(t_py *x, t_symbol *s, long argc, t_atom *argv) {
    PyObject *locals, *globals;
    PyObject *pval;

    if (gensym(s->s_name) == gensym("eval")) {
        char *code_input = atom_getsym(argv)->s_name; 
        post("eval: %s", code_input);

        // python init and setup
        Py_Initialize();
        locals = PyDict_New();
        //globals = PyDict_New();
        //PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
        PyObject* main_module = PyImport_AddModule("__main__");
        PyObject* globals = PyModule_GetDict(main_module);

        if (x->p_module != gensym("")) {
            post("eval-import: %s", x->p_module->s_name);

            PyObject* x_module = PyImport_ImportModule(x->p_module->s_name);
            PyDict_SetItemString(globals, x->p_module->s_name, x_module);
            post("eval-imported: %s", x->p_module->s_name);
        }
        
        pval = PyRun_String(code_input, Py_eval_input, globals, locals);

        if (pval) {

            if (PyLong_Check(pval)) {
                long int_result = PyLong_AsLong(pval);
                outlet_int(x->p_outlet, int_result);
            } 

            else if (PyFloat_Check(pval)) {
                float float_result = (float) PyFloat_AsDouble(pval);
                outlet_float(x->p_outlet, float_result);
            }

            else if (PyUnicode_Check(pval)) {
                const char *unicode_result = PyUnicode_AsUTF8(pval);
                outlet_anything(x->p_outlet, gensym(unicode_result), 0, NIL);
            }

            else if (PyList_Check(pval)) {
                PyObject *iter;
                PyObject *item;
                int i = 0;

                t_atom atoms[128];

                if ((iter = PyObject_GetIter(pval)) == NULL) {
                    error("((iter = PyObject_GetIter(pval)) == NULL)");
                    return;
                }

                while ((item = PyIter_Next(iter)) != NULL) {
                    if (PyLong_Check(item)) {
                        long long_item = PyLong_AsLong(item);
                        atom_setlong(atoms+i, long_item);
                        post("%d long: %ld\n", i, long_item);
                        i++;
                    }
                    
                    if PyFloat_Check(item) {
                        float float_item = PyFloat_AsDouble(item);
                        atom_setfloat(atoms+i, float_item);
                        post("%d float: %f\n", i, float_item);
                        i++;
                    }

                    if PyUnicode_Check(item) {
                        const char *unicode_item = PyUnicode_AsUTF8(item);
                        post("%d unicode: %s\n", i, unicode_item);
                        atom_setsym(atoms+i, gensym(unicode_item));
                        i++;
                    }

                    Py_DECREF(item);
                }
                post("end pylist op: %d", i);
                outlet_anything(x->p_outlet, gensym("res"), i, atoms);
            }

            else {
                t_atom atoms[3];
                atom_setsym(atoms, gensym("could"));
                atom_setsym(atoms + 1, gensym("not"));
                atom_setsym(atoms + 2, gensym("evaluate"));
                outlet_anything(x->p_outlet, gensym("failure"), 3, atoms);
            }

            Py_DECREF(pval);

        } else {
            if (PyErr_Occurred()) {
                PyErr_Print();
                error("error ocurred: %s", code_input);
            }
        }
        
        // --------------
        // new refs
        Py_DECREF(locals);
        Py_DECREF(globals);
        // borrowed refs
        // Py_XDECREF(main_module);
        // Py_XDECREF(x_module);
        // --------------
        Py_FinalizeEx();

    }
    return;  
}


void py_bang(t_py *x)
{
    long sum;
    PyObject *locals, *globals;
    PyObject *pval, *xval, *yval;

    // python init and setup
    Py_Initialize();
    locals = PyDict_New();
    globals = PyDict_New();
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());

    xval = PyLong_FromLong(x->p_value0);
    yval = PyLong_FromLong(x->p_value1);

    PyDict_SetItemString(globals, "x", xval);
    PyDict_SetItemString(globals, "y", yval);

    pval = PyRun_String("x**y", Py_eval_input, globals, locals);

    if PyLong_Check(pval) {
        sum = PyLong_AsLong(pval);
    }

    outlet_int(x->p_outlet, sum);

    Py_DECREF(pval);
    Py_DECREF(xval);
    Py_DECREF(yval);
    Py_FinalizeEx();
}

void py_int(t_py *x, long n)
{
    x->p_value0 = n;
    py_bang(x);
}


void py_in1(t_py *x, long n)
{
    x->p_value1 = n;
}

