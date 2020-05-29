cimport api_max as mx # api is a cython keyword!
cimport api_py as px

import numpy
import numpy as np



txt = "Hey MAX!"

greeting = 'Hello World'

# name = lambda: getattr(globals(), 'PY_NAME')


cpdef public str hello():
    return greeting


def random(int n):
    return np.random.rand(n)


def echo(*args, **kwargs):
    return args


def total(*args, **kwargs):
    return sum(args)


def post(str s):
     mx.post(s.encode('utf-8'))


def error(str s):
     mx.error(s.encode('utf-8'))



cdef class PyExternal:
    cdef px.t_py *obj

    def __cinit__(self, bytes name):
        self.obj = <px.t_py *>mx.object_findregistered(
            mx.CLASS_BOX, mx.gensym(name))

    cpdef bang(self):
        px.py_bang(self.obj)


def test(key='PY_NAME'):
    if key in globals():
        s = globals()[key]
        ext = PyExternal(bytes(s))
        ext.bang()
    else:
        return 'nope'





