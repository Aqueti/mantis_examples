# README.md for Mantis API examples

To compile:
    $ gcc -o HelloMantis HelloMantis.c -lMantisAPI -lpthread

If you get the error 
    error while loading shared libraries: libMantisAPI.so: cannot open shared object file: No such file or directory
when running the compiled executables, run the following command to fix it:
    $ export LD_LIBRARY_PATH='/usr/local/lib
