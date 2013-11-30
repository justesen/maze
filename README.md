maze - a maze generator and solver
==================================


1) Installation
---------------
Note that you need to have a development version of the Cairo graphics library
installed. It's usually found in your distributions repository or can be
downloaded from `http://cairographics.org/`

Compile:

`$ make`

Edit the Makefile to match your local setup. It's per default installed to
/usr/local. Depending on setup you may have to be root:

`$ make install`

Remove binary and object files:

`$ make clean`


2) Running
----------
Generating a 59x59 maze and its solution is just

`$ maze`

And the resultinging images, maze.png and solve.png, are saved to the
current directory. For further information and options see the man page or
invoke maze with usage information

`$ maze --help`


3) Bugs
-------
Please forward any bugs you find to me at `mathias.justesen@gmail.com` or make
a pull request.


4) License and copyright information
------------------------
This software is licensed under the MIT License, see LICENSE for more
information.

