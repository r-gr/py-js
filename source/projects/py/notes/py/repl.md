# Is a real REPL possible?

## Is it possible to launch a python interactive loop / repl from the py external?

Ideally it would be fantastic to launch a python repl per py external instance (and of course full access to the globals namespace and the embedded cython wrapping of the max api) which would allow one to script ma via an enteractive loop (real livecoding)

- What about embedding the jupyter kernel? (tried the main python-based kernel -> caused Max to crash). Perhaps need to launch with nogil? or in a different thread or process?

- What about integrating the new jupyter c++-based kernl [xeus](https://github.com/jupyter-xeus/xeus) or its python implementation [xeus-python](https://github.com/jupyter-xeus/xeus-python)?

- What about just using `osc` via `[udpreceive]`? see [python-osc](https://github.com/attwad/python-osc)

- Iain Duncan's [Schema for Max](https://github.com/iainctduncan/scheme-for-max) has a novel repl which work nicely. Could be another way

- This forum post has a novel idea for unquoting textedit output: https://cycling74.com/forums/textedit-adds-quotes-around-text-that-contains-space-how-to-avoid-this

## Javascript terminal emulators

- core tech <https://xtermjs.org>

- gritty <https://github.com/cloudcmd/gritty> (based on xtermjs and <https://github.com/microsoft/node-pty>)

- this is probably not suitable - <http://www.erikosterberg.com/terminaljs/>

## Protocols

- websockets: <https://websockets.readthedocs.io/en/stable/intro.html>

- OSC ...

- zmq (see below)

## zmq implementations in max

from the implementer of [nt.python_for_max](https://github.com/2bbb/nt.python_for_max):

An implemetation of zmq in Max which must be studied.

- see also:
  - <https://github.com/2bbb/bbb.max.dev>
  - <https://github.com/2bbb/bbb.zmq.sub>
  - <https://github.com/2bbb/bbb.zmq.pub>
  - <https://github.com/2bbb/maxcpp>
