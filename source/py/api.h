/* Generated by Cython 0.29.19 */

#ifndef __PYX_HAVE__api
#define __PYX_HAVE__api

#include "Python.h"

#ifndef __PYX_HAVE_API__api

#ifndef __PYX_EXTERN_C
  #ifdef __cplusplus
    #define __PYX_EXTERN_C extern "C"
  #else
    #define __PYX_EXTERN_C extern
  #endif
#endif

#ifndef DL_IMPORT
  #define DL_IMPORT(_T) _T
#endif

__PYX_EXTERN_C PyObject *hello(int __pyx_skip_dispatch);

#endif /* !__PYX_HAVE_API__api */

/* WARNING: the interface of the module init function changed in CPython 3.5. */
/* It now returns a PyModuleDef instance instead of a PyModule instance. */

#if PY_MAJOR_VERSION < 3
PyMODINIT_FUNC initapi(void);
#else
PyMODINIT_FUNC PyInit_api(void);
#endif

#endif /* !__PYX_HAVE__api */
