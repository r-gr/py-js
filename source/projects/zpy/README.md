# zpy

A max external which uses czmq to connect to a separate python process

The objective is to replicate what the `py` external does but using czmq.


## Requires

```bash
brew install czmq
```



## Status

- [ ] proof-of-concept




## TODO

- how to launch python server automatically and close it with the patch


## Alternatives

- [ ] run as subprocess and read and write from stdin and stdout via a pipe


## Research

- https://developer.apple.com/library/archive/documentation/MacOSX/Conceptual/BPSystemStartup/Chapters/CreatingLaunchdJobs.html

- https://github.com/sheredom/subprocess.h
