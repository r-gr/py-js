# py-js: python3 objects for max

Simple (and extensible) [python3](https://www.python.org) externals for [MaxMSP](https://cycling74.com)

repo - <https://github.com/shakfu/py-js>

![py-js test](./media/screenshot.png)

## Quickstart

If you are interested to try this out, please note that it currently only works on MacOS and that while many aspects of the core externals are quite functional and relatively stable, please consider this as pre-alpha level code. There is still quite a bit to before a release can be made in good concience.

With such caveats aside installation is pretty straighforward:

1. For c compilation, make sure you have either Xcode or the command line tools installed via `xcode-select --install` in the terminal.

2. You should have also have Homebrew python3 installed on your system (see below for specific instructions how to do this) via:

    Note: that the default build script automatically reads your existing homebrew installed python version (currently 3.9.2 at the time of this writing.)

3. Git clone the `py-js` source and run the following in the cloned repo to get the required submodules:

```bash
git submodule init
git submodule update 
```

Then run the following in the root directory of the `py-js` source (other installation options are detailed below) and make sure you understand that it will automatically create a `py` package in your `$HOME/Max 8/Packages` directory:

```bash
./build.sh
```

Open up any of the patch files in the `patcher` directory in in the generated max package, and also look at the `.maxhelp` patcher to understand how the `py` and the `pyjs` objects work. If you want to test both externals at the same time, open the `py_test_standalone.maxpat` file.

Have fun!

## Summary

This is a project which provides two max externals:

### `py` external

```text
globals
    obj_count                    : number of active py objects
    registry                     : global registry to lookup object names

patchers
    subpatchers
        py_repl                  : a basic single line repl for py
        py_repl_plus             : embeds a py object in a py_repl

py max external
    attributes
        name                     : unique object name
        file                     : file to load into editor
        autoload                 : load file at start
        pythonpath               : add path to python sys.path
        debug                    : switch debug logging on/off

    methods (messages) 
        core
            import <module>      : python import to object namespace
            eval <expression>    : python 'eval' semantics
            exec <statement>     : python 'exec' semantics
            execfile <path>      : python 'execfile' semantics
        
        extra
            assign <var> [arg]   : max msg assignments to py object namespace
            call <pyfunc> [arg]  : max friendly python function calling
            pipe <arg> [pyfunc]  : process a py/max value via a pipe of py funcs
            code <expr|stmt>     : alternative way to eval or exec py code
            anything <expr|stmt> : anything version of the code method 
            
        code editor
            read <path>          : read text file into editor
            load <path>          : combo of read <path> -> execfile <path>
            run                  : run the current code in the editor
     
        interobject
            scan                 : scan patcher and store names of child objects
            send <msg>           : send an arbitrary message to a named object

        meta
            count                : give a int count of current live py objects

    inlets
        single inlet             : primary input (anything)

    outlets
        left outlet              : primary output (anything)
        middle outlet            : bang on failure
        right outlet             : bang on success 
```

![py-js test_py](./media/test_py.png)

### `pyjs` external (experimental)

```text
pyjs max external (jsextension)
    attributes
        name                     : unique object name
        file                     : file to load in object namespace
        pythonpath               : add path to python sys.path
        debug                    : switch debug logging on/off
    
    methods 
        core (messages)
            import <module>      : python import to object namespace
            eval <expression>    : python 'eval' semantics
            exec <stmnt>         : python 'exec' semantics
            execfile <path>      : python 'execfile' semantics
        
        extra
            code <expr|stmt>     : eval/exec/import python code (see above)
            

        in-code (non-message)
            eval_to_json <expr>  : python 'eval' returns json


```

## Overview

`py/js` started out as an attempt (during a covid-19 lockdown) to develop a basic python3 external for maxmsp. It then evolved into a more ambitious framework for using python3 in max.

There are two implementation variations:

1. A `py` external which provides a more featureful two-way interface between max and python in a way that feels natural to both languages.

2. A `pyjs` max external/jsextension providing a `PyJS` class and a minimal subset of `py's` features which work well with the max `js` object and javascript code (like returning json directly from evaluations of python expressions).

Both externals have access to builtin python modules and the whole universe of 3rd party modules, and further have the option of importing a builtin `api` module which uses [cython](https://cython.org) to wrap selective portions of the max c-api. This allows regular python code to directly access the max-c-api and script Max objects.

The objective is to have 3 deployment variations:

1. Linking the externals to your system python (homebrew, built from source, etc.) This has the benefit of re-using your existing python modules and is the default option.

2. Embedding the python interpreter in a Max package: in this variation, a dedicated python distribution (zipped or otherwise) is placed in the `support` folder of the `py/js` package (or any other package) and is linked to the `py` external or `pyjs` extension (or both). This makes it usable in standalones.

3. The external itself as a container for the python interpreter: a custom python distribution (zipped or otherwise) is stored inside the external/jsextension object, which can make it portable and usable in standalones.

(Note that only the two first methods work reliably right now. With the latter requiring some slight manual post-build tweaks to get the standalone working with python. Embedding the python interpreter in the external itself is still in progress and not implemented.

Deployment Scenario  | `py` | `pyjs`
:------------------- | :--: | :--------:
Link to sys python   | 1    | 1
Embed in package     | 1    | 1
Embed in external    | 0    | 0

### Key Features

The more mature `py` external has the following c-level methods:

category | method   | param(s)      | in/out | can change ns
:------- | :--------| :------------ | :----: | :------------:
core     | import   | module        | in     | yes
core     | eval     | expression    | out    | no
core     | exec     | statement     | in     | yes
core     | execfile | file          | in     | yes
extra    | assign   | var, data     | in     | yes
extra    | call     | var(s), data  | out    | no
extra    | code     | expr or stmt  | out?   | yes
extra    | anything | expr or stmt  | out?   | yes
extra    | pipe     | var, funcs    | out    | no
editor   | read     | file          | n/a    | no
editor   | load     | file          | n/a    | no
interobj | scan     |               | n/a    | no
interobj | send     | name, msg, .. | n/a    | no
meta     | count    |               | n/a    | no

The more recently developed `pyjs` external implements the following c-level methods:

category | method       | param(s)      | in/out | can change ns
:------- | :----------- | :------------ | :----: | :------------:
core     | import       | module        | in     | yes
core     | eval         | expression    | out    | no
core     | exec         | statement     | in     | yes
core     | execfile     | file          | in     | yes
extra    | code         | expr or stmt  | out?   | yes
in-code  | eval_to_json | expression    | out    | no

In both cases, the `code` method allows for import/exec/eval of python code, which can be said to make those 'fit-for-purpose' methods redundant. However, I have retained them since they are stricter in what they allow and further provide a helpful prefix in messages which indicates message intent.

#### Core

py/js's *core* features have a one-to-one correspondance to python's very high layer as specified [here](https://docs.python.org/3/c-api/veryhigh.html). In the following, when we refer to 'object', we refer to instances of either the `py` or `pyjs` externals. A note of differences between the variations will be provided when appropriate.

- **Per-object namespaces**. Each object has a unique name (which is provided automatically or can be set by the user), and responds to an `import <module>` message which loads the specified python module in its namespace (essentially a `globals` dictionary). Notably, namespaces can be different for each instance.

- **Eval Messages**. Responds to an `eval <expression>` message in the left inlet which is evaluated in the context of the namespace. `py` objects output results to the left outlet, send a bang from the right outlet upon success or a bang from the middle outlet upon failure. `pyjs` objects just return an `atomarray` of the results.

- **Exec Messages**. Responds to an `exec <statement>` message and an `execfile <filepath>` message which executes the statement or the file's code in the object's namespace. For `py` objects, this produces no output from the left outlet, sends a bang from the right outlet upon success or a bang from the middle outlet upon failure. For `pyjs` objects no output is given.

#### Extra

The *extra* category of methods  makes the `py` or `pyjs` object play nice with the max/msp ecosystem:

Implemented for `py` objects at present:

- **Assign Messages**. Responds to an `assign <varname> [x1, x2, ..., xN]` which is equivalent to `<varname> = [x1, x2, ..., xN]` in the python namespace. This is a way of creating variables in the object's python namespace using max message syntax. This produces no output from the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

- **Call Messages**. Responds to a `call <func> arg1 arg2 ... argN` kind of message where `func` is a python callable in the py object's namespace. This corresponds to the python `callable(*args)` syntax. This makes it easier to call python functions in a max-friendly way. If the callable does not have variable arguments, it will alternatively try to apply the arguments as a list i.e. `call func(args)`. Future work will try make `call` correspond to a python generic function call: `<callable> [arg1 arg2 ... arg_n] [key1=val1 key2=val2 ... keyN=valN]`. This outputs results to the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

- **Pipe message**. Like a `call` in reverse, responds to a `pipe <arg> <f1> <f2> ... <fN>` message. In this sense, a value is *piped* through a chain of python functions in the objects namespace and returns the output to the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

Implemented for both `py` and `pyjs` objects:

- **Code or Anything Messages**. Responds to a `code <expression || statement>` or (anything) `<expression || statement>` message. Arbitrary python code (expression or statement) can be used here, because the whole message body is converted to a string, the complexity of the code is only limited by Max's parsing and excaping rules. (EXPERIMENTAL and evolving).

Implemented for `pyjs` objects only:

- **Evaluate to JSON**. Can be used in javascript code only to automatically serialize the results of a python expression as a json string as follows: `evaluate_to_json <expression> -> JSON`.

#### Interobject Communication

Implemented for `py` objects only:

- **Scan Message**. Responds to a `scan` message with arguments. This scans the parent patcher of the object and stores scripting names in the global registry.

- **Send Message**. Responds to a `send <object-name> <msg> <msg-body>` message. Used to send *typed* messages to any named object. Evokes a `scan` for the patcher's objects if a `registry` of names is empty.

#### Editing Support

Implemented for `py` objects only.

- **Line REPL**. The `py`has two bpatcher line `repls`, one of which embeds a `py` object and another which has an outlet to connect to one. The repls include a convenient menu with all of the `py` object's methods and also feature coll-based history via arrow-up/arrow-down recall of entries in a session. Of course, a coll can made to save all commands if required.

- **Experimental Remote Console**. A new method (due to Ian Duncan) of sending code to the `py` node via `udp` has been implemented and allows for an send-from-editor and send-from-interactive-console capabilities. The clients are still in their infancy, but this method looks promising since you get syntax highlighting, syntax checking, and other features for free. It assumes you want to treat your `py` nodes as remotely accessible `server/interpreters-in-max`.

- **Code Editor**. Double-clicking the `py` object opens a code-editor. This is populated by a `read` message which reads a file into the editor and saves the filepath to an attribute. A `load` message also `reads` the file followed by `execfile`. Saving the text in the editor uses the attribute filepath and execs the saved text to the object's namespace.

For `pyjs` objects, code editing is already built into the `js` objects.

#### Scripting

Implemented for both `py` and `pyjs` objects:

- **Exposing Max API to Python** A portion of the max api in `c74support/max-includes` has been converted to a cython `.pxd` file called `api_max.pxd`. This makes it available for a cython implementation file, `api.pyx` which is converted to c-code during builds and embedded in the external. This code enables a custom python builtin module called `api` which can be imported by python scripts in `py` objects or via `import` messages to the object. This allows the subset of the max-api which has been wrapped in cython code to be called directly by python scripts or via messages in a patcher.

## Caveats

- As mentioned earlier, the package and standalone deployment variations are still not yet working.

- The `py` and `pyjs` objects are currently marked as experimental pre-release pre-alpha and still need further unit/functional/integration testing and field testing of course!

- As of this writing, the `api` module, does not (like apparently all 3rd party python c-extensions) unload properly between patches and requires a restart of Max to work after you close the first patch which uses it. Unfortunately, this is a known [bug](https://bugs.python.org/issue34309)in python which is being worked on and hopefully may be [fixed](https://groups.google.com/forum/?utm_medium=email&utm_source=footer#!msg/cython-users/SnVpCE7Sq8M/hdT8S2iFBgAJ) in future versions.

- As an example of the above, `Numpy`, the popular python numerical analysis package, falls in the above category. Indeed, it actually **crashes** Max if imported in a new patch after first use in a prior patch. To address this special case, the module is provided as an object in the `api` module (and this prevents a crash if used again). As above, just restart Max and use it in one patch normally. After closing the first patch, restart Max to use it again in a new patch. (New patch is taken to mean new document.)

- `core` features are supposed to be the most stable, and *should* not crash under most circumstances, `extra` features are less stable since they are more experimental, etc..

- The `api` module is the most experimental and evolving part of this project, and is completely optional. If you don't want to use it, don't import it.

## Building

Only tested on OS X at present. Should be relatively straightforward to port to windows (a pure python build script is being developed to make this easier).

The following is required:

### Xcode

Full xcode is not required, the freely available command line tools are sufficient

```bash
xcode-select --install
```

otherwise download xcode from the app store.

### py-js externals source and maxsdk

The py external is developed as a max package with a `source` folder which contains the max-sdk as a subfolder and which is conveniently available as a git submodule.

First git clone the `py-js` repo:

```bash
git clone https://github.com/shakfu/py-js.git
```

Then cd into the newly cloned source directory and run the following to get the max-sdk

```bash
git submodule init
git submodule update 

```

### Homebrew Python3

Homebrew Python3 is required. If it is not already install see [Homebrew](https://brew.sh) for the install oneliner (provided here as well for reference).

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Once Homebrew is installed, the latest version of python3 can be easily installed as follows:

```bash
brew install python
```

see: <https://installpython3.com/mac> for further info if you are interested.

### cython (optional)

[Cython](https://cython.org) is only used for development and for wrapping the max api. It is advised to install it in case you want to customize the wrapping of the max api.

Install cython as follows:

```bash
pip install cython
```

### Build it

In the root of the package:

```bash
./build.sh
```

or

```bash
make -C source/py build
```

or in the `py-js/sources/py` directory

```bash
make build
```

This builds the default 'linked-to-system|homebrew python' version of `py`. Read further for alternative ways to build and install `py`.

You can run alternative builds using make or the python `builder` from `py-js/sources/py`.

### Build Variations

One of the objectives of this project is to cater to a number of build variations. As of this writing, the homebrew based variations (except for one strange case detailed below) work the most reliably. Externals built on custom minimized static build of python from src also work well but need to be further documented.

There is generally tradeoff of size vs. portability:

name             | uses      | format     | size     | portable  | standalone
:--------------- | :-------: | :--------: | :------: | :-------: | :---------
bin-homebrew-sys | homebrew  | externals  | 300K     | no        | no  [1]
bin-homebrew-pkg | homebrew  | package    | 13.5MB   | yes       | yes
bin-homebrew-ext | homebrew  | externals  | 27.1MB   | yes       | yes [2]
static-ext       | static-py | externals  | 17.6MB   | yes       | yes

[1] additional benefit is you can use all your system python packages

[2] not 100% working yet.

#### Embed Python in the Package (Now working with Standalones)

In the root of the py-js directory:

```bash
make -C source/py bin-homebrew-pkg
```

or in `py-js/source/py`

```bash
make bin-homebrew-pkg
```

This will create a `py` package in $HOME/Documents/Max 8/packages/py

Once this is done you can run some of the patchers to in the package test the py and pyjs objects.

*NOTE*: Recent changes in Max have allowed for this to work in standalones. Just create your standalone application from a patcher which which includes the `py` and `pyjs` objects. Once it is built into a `<STANDALONE>` then copy the whole aforementioned `py` package to `<STANDALONE>/Contents/Resources/C74/packages` and delete the redundant `py.mxo` in `<STANDALONE>/Contents/Resources/C74/externals` since it already exists in the just-copied package.

#### Embedding Python in the External itself

**WARNING**: this currently 'partially' works. Strangely, it works for one exernal and not the other! Not sure why...

This places a minimized python distribution in the external `py.mxo` itself.

From the root of `py-js`, do this:

```bash
cd source/py
make homebrew-ext
```

Another implementation variation builds both externals using a minimal static python build. This has provden reproducibly successful (see `py-js/source/py/targets/static-ext` after building a static-python build. A more robust implementation will be be documented eventually.

### Sidenote about building on a Mac

If you are developing the package in `$HOME/Documents/Max 8/Packages/py` and you have your icloud drive on for Documents, you will find that `make` or `xcodebuild` will reliably fail with 1 error during development, a codesigning error that is due to icloud sync creating detritus in the dev folder. This can mostly ignored (unless your only focus is codesigning the external).

The solution is to move the external project folder to a non iCloud drive folder (such as $HOME/Downloads for example) and then run "xattr -cr ." in the the project directory to remove the detritus (ironically which Apple's system is itself creating) and then it should succeed (provided you have your Info.plist and bundle id correctly specified).

I've tried this several times and  and it works (for "sign to run locally" case and for the "Development" case).

### Style it

The coding style for this project can applied automatically during the build process with `clang-format`. On OS X, you can easily install using brew:

```bash
brew install clang-format
```

The style used in this project is specified in the `.clang-format` file.

## Prior Art and Thanks

I was motivated to start this project because I yearned to to use some python libraries or functions in Max.

Looking around for for a python max external I found the following:

- Thomas Grill's [py/pyext – Python scripting objects for Pure Data and Max](https://grrrr.org/research/software/py/) which seemed very promising but then I read that the 'available Max port is not actively maintained.' I also noted that it's written in C++ and needs an additional [c++ flext](http://grrrr.org/ext/flext) layer to compile. I was further dissuaded from diving in as it supported, at the time, only python 2 which seemed difficult to swallow considering it was no longer supported. Ironically, this project has become more active recently, so the above may no longer apply.

- [max-py](https://github.com/njazz/max-py) -- Embedding Python 2 / 3 in MaxMSP with pybind11. This looks like a reasonable effort, but only 9 commits and no further commits for 2 years as of this writing.

- [nt.python_for_max](https://github.com/2bbb/nt.python_for_max) -- Basic implementation of python in max using a fork of Graham Wakefield's old c++ interface. Hasn't really been touched in 3 years.

Around the time of the beginning of my first covid-19 lockdown, I stumbled upon Iain Duncan's [Scheme for Max](https://github.com/iainctduncan/scheme-for-max) project, and I was quite inspired by his efforts and approaches to embed a scheme implementation into a Max external.

So it was decided, during a period with less distractions than usual, to try to make a minimal python3 external, learn the max sdk, the python c-api, and how to write more than a few lines of c that didn't crash.

It's been an education and I have come to understand precisely a quote I remember somewhere about the c language: that it's "like a scalpel". I painfully now understand this to mean that in skilled hands it can do wonders, otherwise you almost always end up killing the patient.

Thanks to Luigi Castelli for his help on Max/Msp questions, to Stefan Behnel for his help with Cython questions, and to Iain Duncan for providing the initial inspiration and for saving me time with some great implementation ideas.
