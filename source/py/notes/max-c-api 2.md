# Max C API Notes


## Memory management

```c
char *ptr;
char **hand;
ptr = sysmem_newptr(2000);
post("I have a pointer %lx and it is %ld bytes in size",ptr, sysmem_ptrsize(ptr));
ptr = sysmem_resizeptrclear(ptr, 3000);
post("Now I have a pointer %lx and it is %ld bytes in size",ptr,
      sysmem_ptrsize(ptr));
sysmem_freeptr(ptr);
hand = sysmem_newhandle(2000);
post("I have a handle %lx and it is %ld bytes in size",hand,
      sysmem_handlesize(hand));
sysmem_resizehandle(hand, 3000);
post("Now the handle %lx is %ld bytes in size",hand, sysmem_ptrsize(hand));
sysmem_freehandle(hand);
```

## Typed vs Untyped Methods
This is from the Max API docs:

Max objects, such as the one you write, are C data structures in which methods are dynamically bound to functions. Your object's methods are called by Max, but your object can also call methods itself. When you call a method, it is
essential to know whether the method you are calling is typed or not.

Calling a typed method requires passing arguments as an array of atoms. Calling an untyped method requires that you know the exact arguments of the C function implementing the method. In both cases, you supply a symbol that names the method.

In the typed method case, Max will take the array of atoms and pass the arguments to the object according to the method's argument type specifier list. For example, if the method is declared to have an argument type specifier list of A_LONG, 0, the first atom in the array you pass will be converted to an int and passed to the function on the stack. If there are no arguments supplied, invoking a typed method that has A_LONG, 0 as an argument type specifier will fail. To make typed method calls, use `object_method_typed()` or `typedmess()`.

In the untyped method case, Max merely does a lookup of the symbol in the object, and, if a matching function is found, calls the function with the arguments you pass.
Certain methods you write for your object, such as the assist method for describing your object and the DSP method in audio objects, are declared as untyped using the A_CANT argument type specifier. This means that Max will not typecheck the arguments you pass to these methods, but, most importantly, a user cannot hook up a message box to your object and send it a message to invoke an untyped method. (Try this for yourself – send the assist message to a standard Max object.)

When you use an outlet, you're effectively making a typed method call on any objects connected to the outlet.

## Sending arbitrary messages to an object
In https://cycling74.com/forums/error-handling-with-object_method_typed, there is a need to figure the type of the method which is being called in sending.


It looks like the `t_messlist` struct in `t_object` is key!

### Forum Discussions

see: https://cycling74.com/forums/error-handling-with-object_method_typed

```In the Min-Devkit there is an example object called min.remote whose source code is @ https://github.com/Cycling74/min-devkit/blob/master/source/projects/min.remote/min.remote.cpp 
On line 35 a method is called on the "box".  What happens inside of this to get to the correct incantation of object_method() and friends is on line 101 of the file @ https://github.com/Cycling74/min-api/blob/55c65a02a7d4133ac261908f5d47e1be2b7ef1fb/include/c74_min_patcher.h#L101
```

```cpp
template<typename T1, typename T2>
        atom operator()(symbol method_name, T1 arg1, T2 arg2) {
            auto m { find_method(method_name) };

            if (m.type == max::A_GIMME) {
                atoms   as { arg1, arg2 };
                return max::object_method_typed(m.ob, method_name, as.size(), &as[0], nullptr);
            }
            else if (m.type == max::A_GIMMEBACK) {
                atoms       as { arg1, arg2 };
                max::t_atom rv {};

                max::object_method_typed(m.ob, method_name, as.size(), &as[0], &rv);
                return rv;
            }
            else {
                if (typeid(T1) != typeid(atom))
                    return m.fn(m.ob, arg1, arg2);
                else {
                    // atoms must be converted to native types and then reinterpreted as void*
                    // doubles cannot be converted -- supporting those will need to be handled separately
                    return m.fn(m.ob, atom_to_generic(arg1), atom_to_generic(arg2));
                }
            }
        }
```



see: https://cycling74.com/forums/sending-arbitrary-messages-to-other-objects-from-c

Sending to a coll:

Assuming you already have a valid pointer to a Max object the API  function you use to send a message to a receiving object is:

`object_method()` for methods where Max cannot type check the arguments (`A_CANT`)

or

`object_method_typed()` for methods that take a certain number of arguments of a certain type and therefore have a typed function signature (`A_GIMME`)

If you want to send messages to Max object instantiated in a patch, you probably will want to use `object_method_typed()`.

Be aware that in `ext_obex_util.h` there is a number of convenience functions that allow you to specifically send integer, floats, symbols and more...

For example, if you wanted to send the clear message to a [coll] object, this is the code you would write:

`object_method_typed(collObjPtr, gensym("clear"), 0, NULL, NULL);`

or shorter version: (untyped)

`object_method(collObjPtr, gensym("clear"));`

In this example since the message "clear" has no arguments the typed/untyped distinction doesn't really matter and you could use them interchangeably.

### ext_mess.h

```c

/** A list of symbols and their corresponding methods,
  complete with typechecking information. 
  @ingroup obj
*/
typedef struct messlist
{
  struct symbol *m_sym;   ///< Name of the message
  method m_fun;       ///< Method associated with the message
  char m_type[MSG_MAXARG + 1];  ///< Argument type information
} t_messlist;
C74_DEPRECATED( typedef struct messlist Messlist );

//...

/** The structure for the head of any object which wants to have inlets or outlets,
  or support attributes.
  @ingroup obj
*/
typedef struct object
{
  struct messlist *o_messlist;  ///<  list of messages and methods. The -1 entry of the message list of an object contains a pointer to its #t_class entry.
                  // (also used as freelist link)
#ifdef CAREFUL
  t_ptr_int o_magic;          ///< magic number
#endif
  struct inlet *o_inlet;      ///<  list of inlets
  struct outlet *o_outlet;    ///<  list of outlets
} t_object;
C74_DEPRECATED( typedef struct object Object );

```

Look for functions which return `t_messlist`

### ext_obex.h



Undocumented:

```c
t_messlist *class_mess(t_class *x, t_symbol *methodname);
t_messlist *object_mess(t_object *x, t_symbol *methodname); // <- promising!
t_messlist *class_typedwrapper_get(t_class *x, t_symbol *s);
t_messlist *object_typedwrapper_get(t_object *x, t_symbol *s);
```

There is a `t_method_object` as well:

```c
typedef struct _method_object
{
  t_object  ob;
  t_messlist  messlist_entry;
} t_method_object;
```

Undocumented:
```c
t_method_object *method_object_new(method m, C74_CONST char *name, ...);
t_method_object *method_object_new_messlist(t_messlist *m);
void method_object_free(t_method_object *x);
t_symbol *method_object_getname(t_method_object *x);
void method_object_setname(t_method_object *x, t_symbol *s);
method method_object_getmethod(t_method_object *x);
void method_object_setmethod(t_method_object *x, method m);
t_messlist *method_object_getmesslist(t_method_object *x);
void method_object_setmesslist(t_method_object *x, t_messlist *m);

t_method_object *class_getmethod_object(t_class *x, t_symbol *methodname);

// these methods are private -- instance methods are not actually fully implemented at this time
t_method_object *object_getmethod_object(t_object *x, t_symbol *methodname);
```


For Untyped Methods (A_CANT)

```c

/**
  Sends an untyped message to an object. 
  There are some caveats to its use, however, particularly for 64-bit architectures.
  object_method_direct() should be used in cases where floating-point or other non-integer types are being passed on the stack or in return values.

  @ingroup obj

  @param  x   The object that will receive the message 
  @param  s   The message selector
  @param  ...   Any arguments to the message

  @return     If the receiver object can respond to the message, object_method() returns the result. Otherwise, the function will return 0. 

  @remark     Example: To send the message <tt>bang</tt> to the object <tt>bang_me</tt>:
  void *bang_result;
  bang_result = object_method(bang_me, gensym("bang"));
  @endcode
*/

void *object_method(void *x, t_symbol *s, ...);

/**
  do a strongly typed direct call to a method of an object

  @ingroup obj

  
  @param  rt    The type of the return value (double, void*, void...)
  @param  sig   the actual signature of the function in brackets ! 
          something like (t_object *, double, long)   
  @param  x   The object where the method we want to call will be looked for,
          it will also always be the first argument to the function call
  @param  s   The message selector
  @param  ...   Any arguments to the call, the first one will always be the object (x)

  @return     will return anything that the called function returns, typed by (rt)
 
  @remark     Example: To call the function identified by <tt>getcolorat</tt> on the object <tt>pwindow</tt>
          which is declared like:
          t_jrgba pwindow_getcolorat(t_object *window, double x, double y)
  double x = 44.73;
  double y = 79.21;
  t_object *pwindow;
  t_jrgba result = object_method_direct(t_jrgba, (t_object *, double, double), pwindow, gensym("getcolorat"), x, y);
  @endcode
*/
    
#define object_method_direct(rt, sig, x, s, ...) ((rt (*)sig)object_method_direct_getmethod((t_object *)x, s))(object_method_direct_getobject((t_object *)x, s), __VA_ARGS__)

method object_method_direct_getmethod(t_object *x, t_symbol *sym);
void *object_method_direct_getobject(t_object *x, t_symbol *sym);
```


For Typed Methods (A_GIMME)
```c
/**
  Sends a type-checked message to an object.

  @ingroup obj

  @param  x   The object that will receive the message 
  @param  s   The message selector
  @param  ac    Count of message arguments in <tt>av</tt>
  @param  av    Array of t_atoms; the message arguments
  @param  rv    Return value of function, if available

  @return     This function returns the error code #MAX_ERR_NONE if successful, 
          or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

  @remark     If the receiver object can respond to the message, object_method_typed() returns the result in <tt>rv</tt>. Otherwise, <tt>rv</tt> will contain an #A_NOTHING atom.
*/
t_max_err object_method_typed(void *x, t_symbol *s, long ac, t_atom *av, t_atom *rv);
```

### ext_obex_util.h

```c
/**
  Convenience wrapper for object_method_typed() that uses atom_setparse() to define the arguments.

  @ingroup  obj
  @param    x     The object to which the message will be sent.
  @param    s     The name of the method to call on the object.
  @param    parsestr  A C-string to parse into an array of atoms to pass to the method.
  @param    rv      The address of an atom to hold a return value.
  @return   A Max error code.
  
  @see    object_method_typed()
  @see    atom_setparse()
*/
t_max_err object_method_parse(t_object *x, t_symbol *s, C74_CONST char *parsestr, t_atom *rv);
```

## Object Reference
It looks like `obex` is a type `hashtab` (Hash Table), which can be used for storing object references?

## Find named object

see: https://cycling74.com/forums/find-named-object-and-send-it-a-message

  I'm looking at some of the patcher scripting stuff in the api.  iterator.c is a good guide, but I do want to check if there's  a simple method similar to "getnamed" in javascript, such that I don't have to iterate through all the boxes in a patcher.
  Something along the lines of

```c
  t_object *desiredobject = jpatcher_get_namedobject("scriptedname");
```
  and you could then just pass desiredobject into the various jbox goodies.

and the answer

```c
t_max_err err;
t_object *patcher = NULL;
t_object *yourobject = NULL;

// get the patcher
err = object_obex_lookup(yourobjectpointer, gensym("#P"), &patcher);

// get the object that you're looking for.
yourobject = (t_object *)object_method(patcher, gensym("getnamedbox"), gensym("theobjectname"));
```

----

see: https://cycling74.com/forums/error-handling-with-object_method_typed

Avoiding crashes when sending:

  For messages which are internally defined as A_GIMME the correct call to use is object_method_typed(). But for other messages, say one with A_FLOAT as the argument, you will likely want to use object_method(). 


----

see: https://cycling74.com/forums/messnamed-equivalent-send-function-in-c-send-to-named-object


Question: 

  "want to send values to a named object, as there is in javascript with 'messnamed'... send values to receive objects."

Answer:

  all send's and receive's with the same name reference a single nobox object called "through", which you can simply retrieving by looking at the s_thing field of the name symbol — I mean, if you need to retrieve the through object associated to the "foo" symbol, just look for gensym("foo")->s_thing.
  Once you have the object, just send it a message with object_method().

```c
t_max_err object_send_method_typed(void *x, t_symbol *name, t_symbol *s, long ac, t_atom *av, t_atom *rv)
{
   t_object *thing = name->s_thing;

   if (!thing) {
       return MAX_ERR_INVALID_PTR;
   }
   if (NOGOOD(thing)) {
       return MAX_ERR_INVALID_PTR;
   }
   if (!object_classname_compare(thing, gensym("through"))) {
       return MAX_ERR_GENERIC;
   }
   return object_method_typed(thing, s, ac, av, rv);
}
```

## Storing Refs on a Hashtable
see: https://cycling74.com/forums/help!-crashing-when-scripting-patcher-and-storing-refs-in-a-hashtable

```
solution for future readers: I needed to use the OBJ_FLAG_REF flag to the hashtab so that it wouldn't try to free pointers to objects.
```

## Coding GIMMEBACK
see: https://cycling74.com/forums/multiple-atoms-return-by-a_gimmeback


```c
void max_jit_obex_gimmeback_dumpout(void *x, t_symbol *s, long ac, t_atom *av)
{
    t_atom rv;
    t_atom *rav = NULL;
    long rac = 0;
    object_method_typed(max_jit_obex_jitob_get(x),s,ac,av,&rv);
    if (rv.a_type == A_NOTHING) return;
    if (rv.a_type == A_OBJ) { 
        object_getvalueof(rv.a_w.w_obj, &rac, &rav);
    } else {
        rac = 1;
        rav = &rv;
    }
    if (s&&s->s_name[0]=='g'&& s->s_name[1]=='e'&& s->s_name[2]=='t')
        s = gensym(s->s_name+3);
    max_jit_obex_dumpout(x,s,rac,rav);
    if (rv.a_type==A_OBJ) { 
        if (rac&&rav)
            freebytes(rav,rac*sizeof(t_atom));    
        if(rv.a_w.w_obj)
            freeobject(rv.a_w.w_obj); 
  }
}
```

## Path operations
see: https://cycling74.com/forums/locatefolder

## Compiling on Ubuntu 20.04
see: https://stackoverflow.com/questions/27672572/embedding-python-in-c-linking-fails-with-undefined-reference-to-py-initialize

## Writing a text file
see: https://cycling74.com/forums/problem-with-sysfile_writetextfile

```c
 void buffTest_writefile(t_buffTest *x, char *filename, short path)
{
    char *buf  = "write me to a file";
    t_handle h = sysmem_newhandle(0);
    sysmem_ptrandhand(buf, h, strlen(buf));
    long err;
    t_filehandle fh;
    err = path_createsysfile(filename, path, 'TEXT', &fh);
    if (err)
        return;
    err = sysfile_writetextfile(fh, h, TEXT_LB_NATIVE);
    sysfile_close(fh);
    sysmem_freehandle(h);
}
```

## Getting Atoms from Argument
in ext_obex.h:

```c
/**
  Retrieves type from a #t_atom. 

  @ingroup atom
  @param  a   Pointer to a #t_atom whose type is of interest
  @return     This function returns the type of the specified t_atom as defined in #e_max_atomtypes
*/
long atom_gettype(const t_atom *a);

//the following are useful for setting the values _only_ if there is an arg
//rather than setting it to 0 or _sym_nothing

/**
  Retrieves the integer value of a particular t_atom from an atom list, if the atom exists.

  @ingroup atom

  @param  c   Pointer to a long variable to receive the atom's data if the function is successful.
  @param  idx   Offset into the atom list of the atom of interest, starting from 0. 
          For instance, if you want data from the 3rd atom in the atom list, <tt>idx</tt> should be set to 2.
  @param  ac    Count of av.
  @param  av    Pointer to the first t_atom of an atom list.

  @return     This function returns the error code #MAX_ERR_NONE if successful, 
          or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

  @remark     The atom_arg_getlong() function only changes the value of <tt>c</tt> if the function is successful. 
          For instance, the following code snippet illustrates a simple, but typical use:
  void myobject_mymessage(t_myobject *x, t_symbol *s, long ac, t_atom *av)
  {
    t_atom_long var = -1;

    // here, we are expecting a value of 0 or greater
    atom_arg_getlong(&var, 0, ac, av);
    if (val == -1) // i.e. unchanged
      post("it is likely that the user did not provide a valid argument");
    else {
      ...
    }
  }
  @endcode
*/
t_max_err atom_arg_getlong(t_atom_long *c, long idx, long ac, const t_atom *av);


/**
  Retrieves the floating point value of a particular t_atom from an atom list, if the atom exists.

  @ingroup atom

  @param  c   Pointer to a float variable to receive the atom's data if the function is successful. Otherwise, the value is left unchanged.
  @param  idx   Offset into the atom list of the atom of interest, starting from 0. 
          For instance, if you want data from the 3rd atom in the atom list, <tt>idx</tt> should be set to 2.
  @param  ac    Count of av.
  @param  av    Pointer to the first t_atom of an atom list.

  @return     This function returns the error code #MAX_ERR_NONE if successful, 
          or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
long atom_arg_getfloat(float *c, long idx, long ac, const t_atom *av);


/**
  Retrieves the floating point value, as a double, of a particular t_atom from an atom list, if the atom exists.

  @ingroup atom

  @param  c   Pointer to a double variable to receive the atom's data if the function is successful. Otherwise the value is left unchanged.
  @param  idx   Offset into the atom list of the atom of interest, starting from 0. 
          For instance, if you want data from the 3rd atom in the atom list, <tt>idx</tt> should be set to 2.
  @param  ac    Count of av.
  @param  av    Pointer to the first t_atom of an atom list.

  @return     This function returns the error code #MAX_ERR_NONE if successful, 
          or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
long atom_arg_getdouble(double *c, long idx, long ac, const t_atom *av);


/**
  Retrieves the t_symbol * value of a particular t_atom from an atom list, if the atom exists.

  @ingroup atom

  @param  c   Pointer to a t_symbol * variable to receive the atom's data if the function is successful. Otherwise, the value is left unchanged.
  @param  idx   Offset into the atom list of the atom of interest, starting from 0. 
          For instance, if you want data from the 3rd atom in the atom list, <tt>idx</tt> should be set to 2.
  @param  ac    Count of av.
  @param  av    Pointer to the first t_atom of an atom list.

  @return     This function returns the error code #MAX_ERR_NONE if successful, 
          or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

  @remark     The atom_arg_getsym() function only changes the value of <tt>c</tt> if the function is successful. 
          For instance, the following code snippet illustrates a simple, but typical use:
  void myobject_open(t_myobject *x, t_symbol *s, long ac, t_atom *av)
  {
    t_symbol *filename = _sym_nothing;

    // here, we are expecting a file name.
    // if we don't get it, open a dialog box 
    atom_arg_getsym(&filename, 0, ac, av);
    if (filename == _sym_nothing) { // i.e. unchanged
      // open the file dialog box,
      // get a value for filename
    }
    // do something with the filename
  }
  @endcode
*/
long atom_arg_getsym(t_symbol **c, long idx, long ac, const t_atom *av);
```

## Object Creation & Freeing

in ext_obex.h

```c
/**
  Allocates the memory for an instance of an object class and initialize its object header <em>internal to Max</em>. 
  It is used similarly to the traditional function newinstance(), but its use is required with obex-class objects. 
  The object_new_typed() function differs from object_new() by its use of an atom list for object arguments—in this way, 
  it more resembles the effect of typing something into an object box from the Max interface.

  @ingroup obj

  @param  name_space  The desired object's name space. Typically, either the 
            constant #CLASS_BOX, for obex classes which can 
            instantiate inside of a Max patcher (e.g. boxes, UI objects, 
            etc.), or the constant #CLASS_NOBOX, for classes 
            which will only be used internally. Developers can define 
            their own name spaces as well, but this functionality is 
            currently undocumented.
  @param  classname The name of the class of the object to be created
  @param  ac      Count of arguments in <tt>av</tt>
  @param  av      Array of t_atoms; arguments to the class's instance creation function.

  @return       This function returns a new instance of the object class if successful, or NULL if unsuccessful.
*/
void *object_new_typed(t_symbol *name_space, t_symbol *classname, long ac, t_atom *av);


/**
  Call the free function and release the memory for an instance of an internal object class previously instantiated using object_new(), object_new_typed() or other new-style object constructor functions (e.g. hashtab_new()). 
  It is, at the time of this writing, a wrapper for the traditional function freeobject(), but its use is suggested with obex-class objects.

  @ingroup obj
  @param  x   The pointer to the object to be freed. 
  @return     This function returns the error code #MAX_ERR_NONE if successful, 
          or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_free(void *x);
```

## Checking whether an object is an instance of a class
in ext_obex.h

```c
/**
  Determines if a particular object is an instance of a given class.

  @ingroup obj

  @param  x   The object to test
  @param  name  The name of the class to test this object against
  @return     This function returns 1 if the object is an instance of the named class. Otherwise, 0 is returned.
  @remark     For instance, to determine whether an unknown object pointer is a pointer to a print object, one would call:

  long isprint = object_classname_compare(x, gensym("print"));
  @endcode
*/
long object_classname_compare(void *x, t_symbol *name);
```

## Outlets
- outlet creation order is important in `outlet_new(x, NULL)`?

### Dynamic Outlets


see: https://cycling74.com/forums/dynamic-inlets-outlets

```c
// start the transaction with the box
t_object *b = NULL;
object_obex_lookup(x, _sym_pound_B, (t_object **)&b);
object_method(b, gensym("dynlet_begin"));

// update outlets with one or both of these calls
//outlet_delete(outlet_nth((t_object*)x, i));
//outlet_append((t_object*)x, NULL, gensym("signal"));

// end the transaction with the box
object_method(b, gensym("dynlet_end"));
```
