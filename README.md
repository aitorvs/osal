# OSAL
This project is a fork of the OS Abstraction Layer originally devloped by the Flight Software Systems Branch at NASA Goddard Space Flight Center (https://github.com/nasa/osal)

# Basics
This project offers a reacher API and support for specific hardware used at the European Space Agency.

Dependencies:

* The GNU assembler linker and binary utils, binutils v2.10
* The GNU compiler, GCC v4.4.1.2-9
* The GNU C cross-compiler, sparc-rtems-gcc GCC v3.3.2
* The RTEMS real-time operating system v4.6.5
* The GNU make utility, v3.81
* The Qmake utility, v2.01a (Qt 4.3.2)
* The shared libraries for terminal handling (ncurses)
  - libncurses5, v5.6
  - libncursesw5, v5.6
  - ncurses-base, v5.6
  - ncurses-bin, v5.6


The library is documented using `doxygen`. Just generate the documentation and
check the API.

## Installation

Configure and install the library with the following commands

```
$ make menuconfig
$ make all install
```

The default installation folder is `/osal` which will contain a `/osal/include` and `/osal/libosal` folder for the header files and the static library.

The default installation folder can however be changed using defiing the following environment variable.

```
$ export INSTALL_ROOT=<your-install-folder>
```

Developed By
---

Aitor Viana Sanchez - aitor.viana.sanchez@gmail.com>

<a href="https://twitter.com/aitorvs">
  <img alt="Follow me on Twitter"
       src="https://raw.github.com/ManuelPeinado/NumericPageIndicator/master/art/twitter.png" />
</a>
<a href="https://plus.google.com/+AitorViana">
  <img alt="Follow me on Twitter"
       src="https://raw.github.com/ManuelPeinado/NumericPageIndicator/master/art/google-plus.png" />
</a>
<a href="https://www.linkedin.com/in/aitorvs">
  <img alt="Follow me on Twitter"
       src="https://raw.github.com/ManuelPeinado/NumericPageIndicator/master/art/linkedin.png" />
       

