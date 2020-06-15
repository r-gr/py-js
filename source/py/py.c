// py.c

/*--------------------------------------------------------------------------*/
// INCLUDES

/* py external api */
#include "py.h"

/* max/msp api */
#include "api.h"

/*--------------------------------------------------------------------------*/
// GLOBALS

t_class* py_class; // global pointer to object class

static int py_global_obj_count = 0; // when 0 then free interpreter

static t_hashtab* py_global_registry = NULL; // global object lookups

// static wchar_t* program;

/*--------------------------------------------------------------------------*/
// HELPERS

// WARNING: if PY_MAX_LOG_CHAR (which defines PY_MAX_ERR_CHAR) is too low
// long log or err messages will crash 

void py_log(t_py* x, char* fmt, ...)
{
    if (x->p_debug) {
        char msg[PY_MAX_LOG_CHAR];

        va_list va;
        va_start(va, fmt);
        vsprintf(msg, fmt, va);
        va_end(va);

        post("[py %s]: %s", x->p_name->s_name, msg);
    }
}

void py_error(t_py* x, char* fmt, ...)
{
    char msg[PY_MAX_ERR_CHAR];

    va_list va;
    va_start(va, fmt);
    vsprintf(msg, fmt, va);
    va_end(va);

    error("[py %s]: %s", x->p_name->s_name, msg);
}

void py_init_builtins(t_py* x)
{
    PyObject* p_name = NULL;
    PyObject* builtins = NULL;
    int err = -1;

    p_name = PyUnicode_FromString(x->p_name->s_name);
    if (p_name == NULL)
        goto error;

    builtins = PyEval_GetBuiltins();
    if (builtins == NULL)
        goto error;

    err = PyDict_SetItemString(builtins, "PY_OBJ_NAME", p_name);
    if (err == -1)
        goto error;

    err = PyDict_SetItemString(x->p_globals, "__builtins__", builtins);
    if (err == -1)
        goto error;

    Py_XDECREF(p_name);
    // Py_XDECREF(builtins);
    return;

error:
    py_handle_error(x, "could not update object namespace with object name");
    Py_XDECREF(p_name);
    // Py_XDECREF(builtins);
}

t_hashtab* get_global_registry(void) { return py_global_registry; }


void py_locate_path_from_symbol(t_py* x, t_symbol* s)
{
    t_max_err err;

    if (s == gensym("")) { // if no arg supplied ask for file
        x->p_code_filename[0] = 0;

        if (open_dialog(x->p_code_filename, &x->p_code_path,
                        &x->p_code_outtype, &x->p_code_filetype, 1))
            // non-zero: cancelled
            return;

    } else {
        // must copy symbol before calling locatefile_extended
        strncpy_zero(x->p_code_filename, s->s_name, MAX_PATH_CHARS);
        if (locatefile_extended(x->p_code_filename, &x->p_code_path, 
                                &x->p_code_outtype, &x->p_code_filetype, 1)) {
            // nozero: not found
            py_error(x, "can't find file %s", s->s_name);
            return;
        } else {
            err = path_toabsolutesystempath(x->p_code_path, 
                                            x->p_code_filename, 
                                            x->p_code_pathname);
            if (err != MAX_ERR_NONE) {
                py_error(x, "can't convert %s to absolutepath", s->s_name);
                return;
            }
        }

        // success
        // set attribute from pathname symbol
        x->p_code_filepath = gensym(x->p_code_pathname);
    }
}

/*--------------------------------------------------------------------------*/
// INIT & FREE

void ext_main(void* r)
{
    t_class* c;

    c = class_new("py", (method)py_new, (method)py_free, (long)sizeof(t_py),
                  0L, A_GIMME, 0);

    // object methods
    //------------------------------------------------------------------------
    // clang-format off
     

    // testing
    class_addmethod(c, (method)py_bang,       "bang",       0);

    // core
    class_addmethod(c, (method)py_import,     "import",     A_SYM,    0);
    class_addmethod(c, (method)py_eval,       "eval",       A_GIMME,  0);
    class_addmethod(c, (method)py_exec,       "exec",       A_GIMME,  0);
    class_addmethod(c, (method)py_execfile,   "execfile",   A_DEFSYM, 0);

    // core extra
    class_addmethod(c, (method)py_assign,     "assign",     A_GIMME,  0);
    class_addmethod(c, (method)py_call,       "call",       A_GIMME,  0);
    class_addmethod(c, (method)py_code,       "code",       A_GIMME,  0);
    
    // meta
    class_addmethod(c, (method)py_assist,     "assist",     A_CANT, 0);
    class_addmethod(c, (method)py_count,      "count",      A_NOTHING, 0);

    // interobject
    class_addmethod(c, (method)py_scan,       "scan",       A_NOTHING, 0);
    class_addmethod(c, (method)py_send,       "send",       A_GIMME,   0);

    // code editor
    class_addmethod(c, (method)py_read,       "read",       A_DEFSYM, 0);
    class_addmethod(c, (method)py_dblclick,   "dblclick",   A_CANT,   0);
    class_addmethod(c, (method)py_edclose,    "edclose",    A_CANT,   0);
    class_addmethod(c, (method)py_edsave,     "edsave",     A_CANT,   0);
    class_addmethod(c, (method)py_load,       "load",       A_DEFSYM, 0);

    // object attributes
    //------------------------------------------------------------------------

    CLASS_ATTR_LABEL(c, "name", 0,  "unique object id");
    CLASS_ATTR_SYM(c,   "name", 0,   t_py, p_name);
    CLASS_ATTR_BASIC(c, "name", 0);
    // CLASS_ATTR_INVISIBLE(c, "name", 0);

    CLASS_ATTR_LABEL(c,  "debug", 0,  "debug log to console");
    CLASS_ATTR_CHAR(c,   "debug", 0,  t_py, p_debug);
    CLASS_ATTR_STYLE(c,  "debug", 0, "onoff");
    CLASS_ATTR_BASIC(c,  "debug", 0);
    CLASS_ATTR_SAVE(c,   "debug", 0);

    CLASS_ATTR_LABEL(c,  "file", 0,  "default python script");
    CLASS_ATTR_SYM(c,    "file", 0,   t_py,  p_code_filepath);
    CLASS_ATTR_STYLE(c,  "file", 0,   "file");
    CLASS_ATTR_BASIC(c,  "file", 0);
    CLASS_ATTR_SAVE(c,   "file", 0);

    CLASS_ATTR_LABEL(c,  "autoload", 0,  "autoload default python script");
    CLASS_ATTR_CHAR(c,   "autoload", 0,  t_py, p_autoload);
    CLASS_ATTR_STYLE(c,  "autoload", 0, "onoff");
    CLASS_ATTR_BASIC(c,  "autoload", 0);
    CLASS_ATTR_SAVE(c,   "autoload", 0);


    CLASS_ATTR_LABEL(c,  "pythonpath", 0,  "per-object pythonpath");
    CLASS_ATTR_SYM(c,    "pythonpath", 0,  t_py, p_pythonpath);
    CLASS_ATTR_STYLE(c,  "pythonpath", 0,  "file");
    CLASS_ATTR_BASIC(c,  "pythonpath", 0);
    CLASS_ATTR_SAVE(c,   "pythonpath", 0);

    CLASS_ATTR_ORDER(c,  "name",        0,  "1");
    CLASS_ATTR_ORDER(c,  "file",        0,  "2");
    CLASS_ATTR_ORDER(c,  "autoload",    0,  "3");
    CLASS_ATTR_ORDER(c,  "pythonpath",  0,  "4");
    CLASS_ATTR_ORDER(c,  "debug",       0,  "5");

    // clang-format on
    //------------------------------------------------------------------------

    class_register(CLASS_BOX, c);

    /* for js registration (can't be both box and nobox) */
    // c->c_flags = CLASS_FLAG_POLYGLOT;
    // class_register(CLASS_NOBOX, c);

    py_class = c;
}

void* py_new(t_symbol* s, long argc, t_atom* argv)
{
    t_py* x = NULL;

    x = (t_py*)object_alloc(py_class);

    if (x) {

        if (py_global_obj_count == 0) {
            // first py obj is called '__main__'
            x->p_name = gensym("__main__");
        } else {
            x->p_name = symbol_unique();
        }
        // x->p_name = symbol_unique();

        // communication
        x->p_patcher = NULL;
        x->p_box = NULL;

        // python-related
        x->p_pythonpath = gensym("");
        x->p_debug = 1;

        // text editor
        x->p_code = sysmem_newhandle(0);
        x->p_code_size = 0;
        x->p_code_editor = NULL;
        x->p_code_filetype = FOUR_CHAR_CODE('TEXT');
        x->p_code_outtype = 0;
        // x->p_code_filename[MAX_PATH_CHARS];
        // x->p_code_pathname[MAX_PATH_CHARS];
        // short p_code_path;
        x->p_code_filepath = gensym("");

        // create inlet(s)
        // create outlet(s)
        x->p_outlet_right = outlet_new(x, NULL);
        x->p_outlet_middle = outlet_new(x, NULL);
        x->p_outlet_left = outlet_new(x, NULL);

        // process @arg attributes
        attr_args_process(x, argc, argv);

        object_obex_lookup(x, gensym("#P"), (t_patcher**)&x->p_patcher);
        if (x->p_patcher == NULL)
            error("patcher object not created.");

        object_obex_lookup(x, gensym("#B"), (t_box**)&x->p_box);
        if (x->p_box == NULL)
            error("patcher object not created.");

        // create scripting name
        t_max_err err = jbox_set_varname(x->p_box, x->p_name);
        if (err != MAX_ERR_NONE) {
            error("could not set scripting name");
        }

        // python init
        py_init(x);

        py_log(x, "object created");
        for (int i = 0; i < argc; i++) {
            py_log(x, "%d: %s", i, atom_getsym(argv + i)->s_name);
            post("argc: %d  argv: %s", i, atom_getsym(argv + i)->s_name);
        }
    }

    return (x);
}

void py_init(t_py* x)
{
    // wchar_t *program;

    // program = Py_DecodeLocale(argv[0], NULL);
    // program = Py_DecodeLocale("py", NULL);
    // if (program == NULL) {
    //     exit(1);
    // }

    /* Add the cythonized 'api' built-in module, before Py_Initialize */
    if (PyImport_AppendInittab("api", PyInit_api) == -1) {
        py_error(x, "could not add api to builtin modules table");
    }

    // Py_SetProgramName(program);

    Py_Initialize();

    // python init
    PyObject* main_mod = PyImport_AddModule(x->p_name->s_name); // borrowed
    x->p_globals = PyModule_GetDict(main_mod); // borrowed reference
    py_init_builtins(x); // does this have to be a separate function?

    // register the object
    object_register(CLASS_BOX, x->p_name, x);

    // increment global object counter
    py_global_obj_count++;

    if (py_global_obj_count == 1) {
        // if first py object create the py_global_registry;
        py_global_registry = (t_hashtab*)hashtab_new(0);
        hashtab_flags(py_global_registry, OBJ_FLAG_REF);
    }
}

void py_free(t_py* x)
{
    // code editor cleanup
    object_free(x->p_code_editor);
    if (x->p_code)
        sysmem_freehandle(x->p_code);

    Py_XDECREF(x->p_globals);
    // python objects cleanup
    py_log(x, "will be deleted");
    py_global_obj_count--;
    if (py_global_obj_count == 0) {
        /* WARNING: don't call x here or max will crash */
        hashtab_chuck(py_global_registry);

        post("last py obj freed -> finalizing py mem / interpreter.");
        // PyMem_RawFree(program);
        Py_FinalizeEx();
    }
}

/*--------------------------------------------------------------------------*/
// DOCUMENTATION

void py_assist(t_py* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) { // inlet
        sprintf(s, "I am inlet %ld", a);
    } else { // outlet
        sprintf(s, "I am outlet %ld", a);
    }
}

void py_count(t_py* x) { outlet_int(x->p_outlet_left, py_global_obj_count); }
/*--------------------------------------------------------------------------*/
// TESTING

void py_bang(t_py* x)
{
    // just a basic bang out the left outlet method
    outlet_bang(x->p_outlet_left);
}

/*--------------------------------------------------------------------------*/
// COMMON HANDLERS

void py_handle_error(t_py* x, char* fmt, ...)
{
    if (PyErr_Occurred()) {

        // build custom msg
        char msg[50];

        va_list va;
        va_start(va, fmt);
        vsprintf(msg, fmt, va);
        va_end(va);

        // get error info
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);

        // PyObject* ptype_pstr = PyObject_Repr(ptype);
        // const char* ptype_str = PyUnicode_AsUTF8(ptype_pstr);
        Py_XDECREF(ptype);
        // Py_XDECREF(ptype_pstr);

        PyObject* pvalue_pstr = PyObject_Repr(pvalue);
        const char* pvalue_str = PyUnicode_AsUTF8(pvalue_pstr);
        Py_XDECREF(pvalue);
        Py_XDECREF(pvalue_pstr);

        Py_XDECREF(ptraceback);

        error("[py %s] <- (%s): %s", x->p_name->s_name, msg, pvalue_str);
    }
}
void py_handle_float_output(t_py* x, PyObject* pfloat, bool free_now)
{
    if (pfloat == NULL) {
        goto error;
    }

    if (PyFloat_Check(pfloat)) {
        float float_result = (float)PyFloat_AsDouble(pfloat);
        if (float_result == -1.0) {
            if (PyErr_Occurred())
                goto error;
        }

        outlet_float(x->p_outlet_left, float_result);
        outlet_bang(x->p_outlet_right);
    }

    if (free_now) {
        Py_XDECREF(pfloat);
    }
    return;

error:
    py_handle_error(x, "python exception occurred");
    Py_XDECREF(pfloat);
    outlet_bang(x->p_outlet_middle);
}

void py_handle_long_output(t_py* x, PyObject* plong, bool free_now)
{
    if (plong == NULL) {
        goto error;
    }

    if (PyLong_Check(plong)) {
        long long_result = PyLong_AsLong(plong);
        if (long_result == -1) {
            if (PyErr_Occurred())
                goto error;
        }
        outlet_int(x->p_outlet_left, long_result);
        outlet_bang(x->p_outlet_right);
    }

    if (free_now) {
        Py_XDECREF(plong);
    }
    return;

error:
    py_handle_error(x, "python exception occurred");
    Py_XDECREF(plong);
    outlet_bang(x->p_outlet_middle);
}

void py_handle_string_output(t_py* x, PyObject* pstring, bool free_now)
{
    if (pstring == NULL) {
        goto error;
    }

    if (PyUnicode_Check(pstring)) {
        const char* unicode_result = PyUnicode_AsUTF8(pstring);
        if (unicode_result == NULL) {
            goto error;
        }
        outlet_anything(x->p_outlet_left, gensym(unicode_result), 0, NIL);
        outlet_bang(x->p_outlet_right);
    }

    if (free_now) {
        Py_XDECREF(pstring);
    }
    return;

error:
    py_handle_error(x, "python exception occurred");
    Py_XDECREF(pstring);
    outlet_bang(x->p_outlet_middle);
}

void py_handle_list_output(t_py* x, PyObject* plist, bool free_now)
{
    if (plist == NULL) {
        goto error;
    }

    if (PySequence_Check(plist) && !PyUnicode_Check(plist)
        && !PyBytes_Check(plist) && !PyByteArray_Check(plist)) {
        PyObject* iter = NULL;
        PyObject* item = NULL;
        int i = 0;

        t_atom atoms_static[PY_MAX_ATOMS];
        t_atom* atoms = NULL;
        int is_dynamic = 0;

        Py_ssize_t seq_size = PySequence_Length(plist);

        if (seq_size == 0) {
            py_error(x, "cannot convert py list of length 0 to atoms");
            goto error;
        }

        if (seq_size > PY_MAX_ATOMS) {
            py_log(x, "dynamically increasing size of atom array");
            atoms = atom_dynamic_start(atoms_static, PY_MAX_ATOMS,
                                       seq_size + 1);
            is_dynamic = 1;

        } else {
            atoms = atoms_static;
        }

        if ((iter = PyObject_GetIter(plist)) == NULL) {
            goto error;
        }

        while ((item = PyIter_Next(iter)) != NULL) {
            if (PyLong_Check(item)) {
                long long_item = PyLong_AsLong(item);
                if (long_item == -1) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setlong(atoms + i, long_item);
                py_log(x, "%d long: %ld\n", i, long_item);
                i++;
            }

            if PyFloat_Check (item) {
                float float_item = PyFloat_AsDouble(item);
                if (float_item == -1.0) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setfloat(atoms + i, float_item);
                py_log(x, "%d float: %f\n", i, float_item);
                i++;
            }

            if PyUnicode_Check (item) {
                const char* unicode_item = PyUnicode_AsUTF8(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                atom_setsym(atoms + i, gensym(unicode_item));
                py_log(x, "%d unicode: %s\n", i, unicode_item);
                i++;
            }
            Py_DECREF(item);
        }

        outlet_list(x->p_outlet_left, NULL, i, atoms);
        outlet_bang(x->p_outlet_right);
        py_log(x, "end iter op: %d", i);

        if (is_dynamic) {
            py_log(x, "restoring to static atom array");
            atom_dynamic_end(atoms_static, atoms);
        }
    }

    if (free_now) {
        Py_XDECREF(plist);
    }
    return;

error:
    py_handle_error(x, "python exception occurred");
    Py_XDECREF(plist);
    outlet_bang(x->p_outlet_middle);
}

void py_handle_output(t_py* x, PyObject* pval)
{
    // <python type> -> left outlet output handlers
    // error handling is already provided in each handler
    py_handle_float_output(x, pval, 0);
    py_handle_long_output(x, pval, 0);
    py_handle_string_output(x, pval, 0);
    py_handle_list_output(x, pval, 0);

    // final cleanup
    Py_XDECREF(pval);
    return;
}

/*--------------------------------------------------------------------------*/
// TRANSLATORS
PyObject* py_atom_to_list(t_py* x, long argc, t_atom* argv, int start_from)
{

    PyObject* plist = NULL; // python list

    if ((plist = PyList_New(0)) == NULL) {
        py_error(x, "could not create an empty python list");
        goto error;
    }

    for (int i = start_from; i < argc; i++) {
        switch ((argv + i)->a_type) {
        case A_FLOAT: {
            double c_float = atom_getfloat(argv + i);
            PyObject* p_float = PyFloat_FromDouble(c_float);
            if (p_float == NULL) {
                goto error;
            }
            PyList_Append(plist, p_float);
            Py_DECREF(p_float);
            break;
        }
        case A_LONG: {
            PyObject* p_long = PyLong_FromLong(atom_getlong(argv + i));
            if (p_long == NULL) {
                goto error;
            }
            PyList_Append(plist, p_long);
            Py_DECREF(p_long);
            break;
        }
        case A_SYM: {
            PyObject* p_str = PyUnicode_FromString(
                atom_getsym(argv + i)->s_name);
            if (p_str == NULL) {
                goto error;
            }
            PyList_Append(plist, p_str);
            Py_DECREF(p_str);
            break;
        }
        default:
            py_log(x, "cannot process unknown type");
            break;
        }
    }
    return plist;

error:
    py_error(x, "atom to list conversion failed");
    return NULL;
}

/*--------------------------------------------------------------------------*/
// CORE

void py_import(t_py* x, t_symbol* s)
{
    PyObject* x_module = NULL;

    if (s != gensym("")) {
        x_module = PyImport_ImportModule(s->s_name);
        // x_module borrrowed ref
        if (x_module == NULL) {
            goto error;
        }
        PyDict_SetItemString(x->p_globals, s->s_name, x_module);
        outlet_bang(x->p_outlet_right);
        py_log(x, "imported: %s", s->s_name);
    }
    return;

error:
    py_handle_error(x, "import %s", s->s_name);
    outlet_bang(x->p_outlet_middle);
}

void py_eval(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    char* py_argv = atom_getsym(argv)->s_name;
    py_log(x, "%s %s", s->s_name, py_argv);

    PyObject* pval = PyRun_String(py_argv, Py_eval_input, x->p_globals,
                                  x->p_globals);

    if (pval != NULL) {
        py_handle_output(x, pval);
        return;
    } else {
        py_handle_error(x, "eval %s", py_argv);
    }
}

void py_exec(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    char* py_argv = NULL;
    PyObject* pval = NULL;

    py_argv = atom_getsym(argv)->s_name;
    if (py_argv == NULL) {
        goto error;
    }

    pval = PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals);
    if (pval == NULL) {
        goto error;
    }
    outlet_bang(x->p_outlet_right);

    // success cleanup
    Py_DECREF(pval);
    py_log(x, "exec %s", py_argv);
    return;

error:
    py_handle_error(x, "exec %s", py_argv);
    Py_XDECREF(pval);
    outlet_bang(x->p_outlet_middle);
}

void py_execfile(t_py* x, t_symbol* s)
{

    PyObject* pval = NULL;
    FILE* fhandle = NULL;

    if (s != gensym("")) {
        // set x->p_code_filepath
        py_locate_path_from_symbol(x, s);
    }

    if (s == gensym("") || x->p_code_filepath == gensym("")) {
        py_error(x, "could not set filepath");
        goto error;
    } 

    // assume x->p_code_filepath has be been set without errors

    py_log(x, "pathname: %s",  x->p_code_filepath->s_name);
    fhandle = fopen(x->p_code_filepath->s_name, "r+");

    if (fhandle == NULL) {
        py_error(x, "could not open file");
        goto error;
    }

    pval = PyRun_File(fhandle,  x->p_code_filepath->s_name, Py_file_input, 
                      x->p_globals, x->p_globals);
    if (pval == NULL) {
        fclose(fhandle);
        goto error;
    }

    // success cleanup
    fclose(fhandle);
    Py_DECREF(pval);
    outlet_bang(x->p_outlet_right);
    return;

error:
    py_handle_error(x, "execfile");
    Py_XDECREF(pval);
    outlet_bang(x->p_outlet_middle);
}

/*--------------------------------------------------------------------------*/
// EXTRA

void py_call(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    char* callable_name = NULL;
    PyObject* py_argslist = NULL;
    PyObject* pval = NULL;
    PyObject* py_callable = NULL;
    // python list
    PyObject* py_args = NULL; // python tuple

    // first atom in argv must be a symbol
    if (argv->a_type != A_SYM) {
        py_error(x, "first atom must be a symbol!");
        goto error;

    } else {
        callable_name = atom_getsym(argv)->s_name;
        py_log(x, "callable_name: %s", callable_name);
    }

    py_callable = PyRun_String(callable_name, Py_eval_input, x->p_globals,
                               x->p_globals);
    if (py_callable == NULL) {
        py_error(x, "could not evaluate %s", callable_name);
        goto error;
    }

    py_argslist = py_atom_to_list(x, argc, argv, 1);
    if (py_argslist == NULL) {
        py_error(x, "atom to py list conversion failed");
        goto error;
    }

    py_log(x, "length of argc:%ld list: %d", argc, PyList_Size(py_argslist));

    // convert py_args to tuple
    py_args = PyList_AsTuple(py_argslist);
    if (py_args == NULL) {
        py_error(x, "unable to convert args list to tuple");
        goto error;
    }

    // pval = PyObject_Call(py_callable, py_args, NULL);
    pval = PyObject_CallObject(py_callable, py_args);
    if (!PyErr_ExceptionMatches(PyExc_TypeError)) {
        if (pval == NULL) {
            py_error(x, "unable to apply callable(*args)");
            goto error;
        }
        goto handle_output;
    }
    PyErr_Clear();

    pval = PyObject_CallFunctionObjArgs(py_callable, py_argslist, NULL);
    if (pval == NULL) {
        py_error(x, "could not retrieve result of callable(list)");
        goto error;
    }
    goto handle_output; // this is redundant but safe in case code is added

handle_output:

    py_handle_output(x, pval);
    // success cleanup
    Py_XDECREF(py_callable);
    Py_XDECREF(py_argslist);
    py_log(x, "END %s", s->s_name);
    outlet_bang(x->p_outlet_right);
    return;

error:

    py_handle_error(x, "anything %s", s->s_name);
    // cleanup
    Py_XDECREF(py_callable);
    Py_XDECREF(py_argslist);
    Py_XDECREF(pval);
    outlet_bang(x->p_outlet_middle);
}

void py_assign(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    char* varname = NULL;
    PyObject* list = NULL;

    if (s != gensym(""))
        py_log(x, "s: %s", s->s_name);

    // first atom in argv must be a symbol
    if (argv->a_type != A_SYM) {
        py_error(x, "first atom must be a symbol!");
        goto error;

    } else {
        varname = atom_getsym(argv)->s_name;
        py_log(x, "varname: %s", varname);
    }

    list = py_atom_to_list(x, argc, argv, 1);
    if (list == NULL) {
        py_error(x, "atom to py list conversion failed");
        goto error;
    }

    if (PyList_Size(list) != argc - 1) {
        py_error(x, "PyList_Size(list) != argc - 1");
        goto error;
    } else {
        py_log(x, "length of list: %d", PyList_Size(list));
    }

    // finally, assign list to varname in object namespace
    py_log(x, "setting %s to list in namespace", varname);
    int res = PyDict_SetItemString(x->p_globals, varname, list);
    if (res != 0) {
        py_error(x, "assign varname to list failed");
        goto error;
    }
    // Py_XDECREF(list); // causes a crash
    outlet_bang(x->p_outlet_right);
    return;

error:
    py_handle_error(x, "assign %s", s->s_name);
    Py_XDECREF(list);
    outlet_bang(x->p_outlet_middle);
}

void py_code(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    long textsize = 0;
    char* text = NULL;
    PyObject* co = NULL;
    PyObject* pval = NULL;
    t_max_err err;
    int is_eval = 1;

    err = atom_gettext(argc, argv, &textsize, &text,
                       OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    if (err == MAX_ERR_NONE && textsize && text) {
        py_log(x, "call %s", text);
    } else {
        goto error;
    }

    co = Py_CompileString(text, x->p_name->s_name, Py_eval_input);

    if (PyErr_ExceptionMatches(PyExc_SyntaxError)) {
        PyErr_Clear();
        co = Py_CompileString(text, x->p_name->s_name, Py_single_input);
        is_eval = 0;
    }

    if (co == NULL) { // can be eval-co or exec-co or NULL here
        goto error;
    }
    sysmem_freeptr(text);

    pval = PyEval_EvalCode(co, x->p_globals, x->p_globals);
    if (pval == NULL) {
        goto error;
    }
    Py_DECREF(co);

    if (!is_eval) {
        // bang for exec-type op
        outlet_bang(x->p_outlet_right);
    } else {
        py_handle_output(x, pval);
    }
    return;

error:
    py_handle_error(x, "call failed");
    // fail bang
    outlet_bang(x->p_outlet_middle);
}

void py_scan(t_py* x)
{
    long result = 0;

    hashtab_clear(py_global_registry);

    if (x->p_patcher == NULL) {
        post("p_patcher == NULL");
    } else {
        post("p_patcher != NULL");
    }

    if (x->p_patcher) {
        object_method(x->p_patcher, gensym("iterate"),
                      (method)py_scan_callback, x, PI_DEEP | PI_WANTBOX,
                      &result);
    } else {
        py_error(x, "scan failed");
    }
}

long py_scan_callback(t_py* x, t_object* box)
{
    t_rect jr;
    t_object* p;
    t_symbol* s;
    t_symbol* varname;
    t_object* obj;
    t_symbol* obj_id;

    jbox_get_patching_rect(box, &jr);
    p = jbox_get_patcher(box);
    varname = jbox_get_varname(box);
    obj = jbox_get_object(box);

    // STRANGE BUG: single quotes in py_log cause a crash but not with post!!
    // perhaps because post is a macro for object_post?
    if (varname && varname != gensym("")) {
        // post("XXXX -> '%s'", varname->s_name);
        py_log(x, "storing object %s in the global registry", varname->s_name);
        hashtab_store(py_global_registry, varname, obj);
    }

    obj_id = jbox_get_id(box);
    s = jpatcher_get_name(p);
    object_post(
        (t_object*)x,
        "in patcher:%s, varname:%s id:%s box @ x %ld y %ld, w %ld, h %ld",
        s->s_name, varname->s_name, obj_id->s_name, (long)jr.x, (long)jr.y,
        (long)jr.width, (long)jr.height);
    return 0;
}

void py_send(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    // see:
    // https://cycling74.com/forums/error-handling-with-object_method_typed
    t_object* obj = NULL;
    char* obj_name = NULL;
    t_symbol* msg_sym = NULL;
    t_max_err err = NULL;

    if (argc < 2) {
        py_error(x, "need at least 2 args to send msg");
        goto error;
    }

    if ((argv + 0)->a_type != A_SYM) {
        py_error(
            x, "1st arg of send needs to be a symbol name of receiver object");
        goto error;
    }

    // argv+0 is the object name to send to
    obj_name = atom_getsym(argv)->s_name;
    if (obj_name == NULL) {
        goto error;
    }

    // if registry is empty, scan it
    if (hashtab_getsize(py_global_registry) == 0) {
        py_scan(x);
    }

    // // lookup name in registry
    err = hashtab_lookup(py_global_registry, gensym(obj_name), &obj);
    if (err != MAX_ERR_NONE || obj == NULL) {
        py_error(x, "no object found in the registry");
        goto error;
    }

    // atom after the name of the receiver
    switch ((argv + 1)->a_type) {
    case A_SYM: {
        msg_sym = atom_getsym(argv + 1);
        if (msg_sym == NULL) { // should check type here
            goto error;
        }
        // address the minimum case: e.g a bang
        if (argc - 2 == 0) { //
            argc = 0;
            argv = NULL;
        } else {
            argc = argc - 2;
            argv = argv + 2;
        }
        break;
    }
    case A_FLOAT: {
        msg_sym = gensym("float");
        if (msg_sym == NULL) { // should check type here
            goto error;
        }

        argc = argc - 1;
        argv = argv + 1;

        break;
    }
    case A_LONG: {
        msg_sym = gensym("int");
        if (msg_sym == NULL) { // should check type here
            goto error;
        }

        argc = argc - 1;
        argv = argv + 1;

        break;
    }
    default:
        py_log(x, "cannot process unknown type");
        break;
    }

    // methods to get method type
    t_messlist* messlist = object_mess((t_object*)obj, msg_sym);
    if (messlist) {
        post("messlist->m_sym  (name of msg): %s", messlist->m_sym->s_name);
        post("messlist->m_type (type of msg): %d", messlist->m_type[0]);
    }

    err = object_method_typed(obj, msg_sym, argc, argv, NULL);
    if (err) {
        py_error(x, "failed to send a message to object %s", obj_name);
        goto error;
    }

    // success
    return;

error:
    py_error(x, "send failed");
    return;
}

/*--------------------------------------------------------------------------*/
// EDITOR

void py_dblclick(t_py* x)
{
    if (x->p_code_editor)
        object_attr_setchar(x->p_code_editor, gensym("visible"), 1);
    else {
        x->p_code_editor = object_new(CLASS_NOBOX, gensym("jed"), x, 0);
        object_method(x->p_code_editor, gensym("settext"), *x->p_code,
                      gensym("utf-8"));
        object_attr_setchar(x->p_code_editor, gensym("scratch"), 1);
        object_attr_setsym(x->p_code_editor, gensym("title"),
                           gensym("py-editor"));
    }
}

void py_read(t_py* x, t_symbol* s)
{
    defer((t_object*)x, (method)py_doread, s, 0, NULL);
}

void py_doread(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    t_max_err err;
    t_filehandle fh;

    py_locate_path_from_symbol(x, s);
    err = path_opensysfile(x->p_code_filename, x->p_code_path, &fh, READ_PERM);
    if (!err) {
        sysfile_readtextfile(fh, x->p_code, 0,
                             TEXT_LB_UNIX | TEXT_NULL_TERMINATE);
        sysfile_close(fh);
        x->p_code_size = sysmem_handlesize(x->p_code);
    }
}

// void py_doread(t_py* x, t_symbol* s, long argc, t_atom* argv)
// {
//     t_fourcc filetype = FOUR_CHAR_CODE('TEXT'), outtype;
//     char filename[MAX_PATH_CHARS];
//     char pathname[MAX_PATH_CHARS];
//     short path;
//     t_max_err err;
//     t_filehandle fh;

//     if (s == gensym("")) { // if no arg supplied ask for file
//         filename[0] = 0;

//         if (open_dialog(filename, &path, &outtype, &filetype, 1))
//             // non-zero: cancelled
//             return;

//     } else {
//         // must copy symbol before calling locatefile_extended
//         strcpy(filename, s->s_name);
//         if (locatefile_extended(filename, &path, &outtype, &filetype, 1)) {
//             // nozero: not found
//             py_error(x, "can't find file %s", s->s_name);
//             return;
//         } else {
//             err = path_toabsolutesystempath(path, filename, pathname);
//         }

//         // success
//         // set attribute from pathname symbol
//         x->p_code_filepath = gensym(pathname);
//         err = path_opensysfile(filename, path, &fh, READ_PERM);
//         if (!err) {
//             sysfile_readtextfile(fh, x->p_code, 0,
//                                  TEXT_LB_UNIX | TEXT_NULL_TERMINATE);
//             sysfile_close(fh);
//             x->p_code_size = sysmem_handlesize(x->p_code);
//         }
//     }
// }

void py_edclose(t_py* x, char** text, long size)
{
    if (x->p_code)
        sysmem_freehandle(x->p_code);

    x->p_code = sysmem_newhandleclear(size + 1);
    sysmem_copyptr((char*)*text, *x->p_code, size);
    x->p_code_size = size + 1;
    x->p_code_editor = NULL;
}

void py_edsave(t_py* x, char** text, long size)
{
    PyObject* pval = NULL;

    if (text == NULL) {
        goto error;
    }

    pval = PyRun_String(*text, Py_file_input, x->p_globals, x->p_globals);
    if (pval == NULL) {
        goto error;
    }

    // success cleanup
    Py_DECREF(pval);
    return;

error:
    py_handle_error(x, "edclose-exec %s", x->p_code_filepath->s_name);
    Py_XDECREF(pval);
}

void py_load(t_py* x, t_symbol* s)
{
    if (s == gensym("")) {
        if (x->p_code_filepath != gensym("")) {
            py_read(x, x->p_code_filepath);
            py_execfile(x, x->p_code_filepath);
            return;
        }
    } else {
        py_read(x, s);
        py_execfile(x, s);
    }
}
