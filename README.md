# udaq [![Linux](https://github.com/samangh/udaq/actions/workflows/linux.yml/badge.svg)](https://github.com/samangh/udaq/actions/workflows/linux.yml) [![Windows](https://github.com/samangh/udaq/actions/workflows/windows.yml/badge.svg)](https://github.com/samangh/udaq/actions/workflows/windows.yml)

## Requirements

This library works on Linux, macOS and Windows.

The requirements are:

* an OpenGL capable graphics card if using the graphical user interface;
* a VISA implementation;
* drivers for GPIB (IEEE 488.2) if using a GPIB interface.

If using the graphical interface with a virtual machine (_e.g_
VirtualBox, Parallels Desktop or VMWare Fusion/Workstation), please
enable OpenGL pass-through for the virtual machine.

If you have LabView installed, then the National Instruments VISA will
already have been installed and so you should be ready to go.

### VISA implementation

This software will work with _any_ VISA implementation that adheres to
the Virtual Instrument Software Architecture standard.

Major implementations are from [National
Instruments](https://www.ni.com/en-gb/support/downloads/drivers/download.ni-visa.html),
[Agilent/Keysight](https://www.keysight.com/find/iosuite),
[Tektronix](https://uk.tek.com/oscilloscope/tds7054-software/tekvisa-connectivity-software-v420),
and [Rohde &
Schwarz](https://www.rohde-schwarz.com/sg/applications/r-s-visa-application-note_56280-148812.html). Here
is a compatibility table of which implementation can be used with which
operating system (note that this is defined by the manufacturers, and
not udaq).

| Operating System    | R&S | National Instruments | Keysight/Agilent | Tektronix | Kikusui |
|---------------------|:---:|:--------------------:|:----------------:|:---------:|:--------|
| Windows             | ✔   | ✔                    | ✔                | ✔         | ✔       |
| macoS               | ✔   | ✔                    | ✗                | ✗         | ✗       |
| Linux CentOS / RHEL | ✔   | ✔                    | ✔                | ✗         | ✗       |
| Linux openSuSE      | ✔   | ✔                    | ✗                | ✗         | ✗       |
| Linux Ubuntu        | ✔   | ✗                    | ✔                | ✗         | ✗       |
| Linux Raspian       | ✔   | ✗                    | ✗                | ✗         | ✗       |

This software has been tested with:

* Windows 10 (using National Instrument VISA);
* macOS (using National Instruments and R&S VISA);
* Linux CentOS 8 (using National Instruments, R&S and Keysight VISA);
* Linux openSuSE 8 (using National Instruments VISA).

## License

This software is licensed under the Lesser General Public License (LGPL)
version 2.1 ONLY, with the following exceptions:

1.  Any contributors grant Saman Ghannadzadeh the permission to deal
    with their contributed code without restriction, including without
    limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of software containing
    their contributions.
