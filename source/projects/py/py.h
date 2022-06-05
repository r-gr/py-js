/* py.h */

#ifndef PY_H
#define PY_H


/** \file py.h
    \brief Documentation of py external header interface.
    
    This is the main header file for the `py` external. Note that the external
    structure is not directly exposed at the header level.
*/


/*--------------------------------------------------------------------------*/
/* Includes */

/* max api */
#include "ext.h"
#include "ext_obex.h"

/* python */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

/* conditional includes */
#if defined(__APPLE__) && (defined(PY_STATIC_EXT) || defined(PY_SHARED_PKG))
#include <CoreFoundation/CoreFoundation.h>
#include <libgen.h>
#endif

/*--------------------------------------------------------------------------*/
/* Constants */

#define PY_MAX_ATOMS 128
#define PY_MAX_LOG_CHAR 500 // high number during development
#define PY_MAX_ERR_CHAR PY_MAX_LOG_CHAR

/*--------------------------------------------------------------------------*/
/* Macros */

#define _STR(x) #x
#define STR(x) _STR(x)
#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)
#define _PY_VER CONCAT(PY_MAJOR_VERSION, CONCAT(., PY_MINOR_VERSION))
#define PY_VER STR(_PY_VER)
// PY_VERSION is already defined as Major.Minor.Patch by patchlevel.h

/*--------------------------------------------------------------------------*/
/* Globals */

t_class* py_class;                    // global pointer to object class
static int py_global_obj_count;       // when 0 then free interpreter
static t_hashtab* py_global_registry; // global object lookups

/*--------------------------------------------------------------------------*/
/* Datastructures */

typedef struct t_py t_py;

/*--------------------------------------------------------------------------*/
/* Object creation and destruction Methods */
void* py_new(t_symbol* s, long argc, t_atom* argv);
void py_free(t_py* x);
void py_init(t_py* x);

/*--------------------------------------------------------------------------*/
/* Helpers */

void py_log(t_py* x, char* fmt, ...);
void py_error(t_py* x, char* fmt, ...);
void py_init_builtins(t_py* x);
void py_init_osx_set_home_static_ext(void);
void py_init_osx_set_home_shared_pkg(void);
void py_init_osx_set_home_framework_ext(void);
t_hashtab* get_global_registry(void);
t_max_err py_locate_path_from_symbol(t_py* x, t_symbol* s);
t_max_err py_eval_text(t_py* x, long argc, t_atom* argv, int offset);

/*--------------------------------------------------------------------------*/
/* Side-effect helpers */

void py_bang(t_py* x);
void py_bang_success(t_py* x);
void py_bang_failure(t_py* x);
void* get_outlet(t_py* x);

/*--------------------------------------------------------------------------*/
/* Common handlers */

void py_handle_error(t_py* x, char* fmt, ...);
t_max_err py_handle_float_output(t_py* x, PyObject* pval);
t_max_err py_handle_long_output(t_py* x, PyObject* pval);
t_max_err py_handle_string_output(t_py* x, PyObject* pval);
t_max_err py_handle_list_output(t_py* x, PyObject* pval);
t_max_err py_handle_dict_output(t_py* x, PyObject* pval);
t_max_err py_handle_output(t_py* x, PyObject* pval);

/*--------------------------------------------------------------------------*/
/* Core Python Methods */

t_max_err py_import(t_py* x, t_symbol* s);
t_max_err py_eval(t_py* x, t_symbol* s, long argc, t_atom* argv);
t_max_err py_exec(t_py* x, t_symbol* s, long argc, t_atom* argv);
t_max_err py_execfile(t_py* x, t_symbol* s);

/*--------------------------------------------------------------------------*/
/* Extra Python Methods */

t_max_err py_assign(t_py* x, t_symbol* s, long argc, t_atom* argv);
t_max_err py_call(t_py* x, t_symbol* s, long argc, t_atom* argv);
t_max_err py_code(t_py* x, t_symbol* s, long argc, t_atom* argv);
t_max_err py_pipe(t_py* x, t_symbol* s, long argc, t_atom* argv);
t_max_err py_anything(t_py* x, t_symbol* s, long argc, t_atom* argv);

/*--------------------------------------------------------------------------*/
/* Informations Methods */

void py_count(t_py* x);
void py_assist(t_py* x, void* b, long m, long a, char* s);
void py_appendtodict(t_py* x, t_dictionary* dict);

/*--------------------------------------------------------------------------*/
/* Time-based Methods */

t_max_err py_task(t_py* x);
t_max_err py_sched(t_py* x, t_symbol* s, long argc, t_atom* argv);

/*--------------------------------------------------------------------------*/
/* Interobject Methods */

t_max_err py_send(t_py* x, t_symbol* s, long argc, t_atom* argv);
void py_scan(t_py* x);
long py_scan_callback(t_py* x, t_object* obj);

/*--------------------------------------------------------------------------*/
/* Code editor Methods */

void py_read(t_py* x, t_symbol* s);
void py_load(t_py* x, t_symbol* s); // read(f) -> execfile(f)
void py_doread(t_py* x, t_symbol* s, long argc, t_atom* argv);
void py_dblclick(t_py* x);
void py_run(t_py* x);
void py_edclose(t_py* x, char** text, long size);
t_max_err py_edsave(t_py* x, char** text, long size);
// void py_okclose(t_py* x, char *s, short *result);

/*--------------------------------------------------------------------------*/
/* max datastructure support methods */

// table
bool py_table_exists(t_py* x, char* table_name);
t_max_err py_list_to_table(t_py* x, char* table_name, PyObject* plist);
PyObject* py_table_to_list(t_py* x, char* table_name);



#endif // PY_H
