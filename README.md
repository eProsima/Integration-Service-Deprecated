# eProsima Integration Service
![http://www.eprosima.com](https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcSd0PDlVz1U_7MgdTe0FRIWD0Jc9_YH-gGi0ZpLkr-qgCI6ZEoJZ5GBqQ)

*eProsima Integration Service* is a library based on *Fast RTPS* for creating parameterized 
communication bridges between different systems, services and protocols. 
Also it is able to perform some transformations over the messages such as customized routing, 
mapping between input and output attributes or data modification.

Some of the possibilities offered by *Integration Service* are:

-   Connects two different domains.
-   Mapping between different data types.
-   User-defined operations over the circulating messages.
-   Communication with others environments, as *ROS2*.

<p align="center"> <img src="docs/IS-main.png" alt="Default behaviour"/> </p>

<hr></hr>

#### Table Of Contents

[Installation](#installation)

[Documentation](#documentation)

[Getting Help](#getting-help)
<hr></hr>

#### Installation

Before compiling *eProsima Integration Service* you need to have installed *Fast RTPS* as described 
in its [documentation](http://eprosima-fast-rtps.readthedocs.io/en/latest/binaries.html). 
For cloning this project execute:

```bash
    $ git clone --recursive https://github.com/eProsima/integration-service
```

IMPORTANT: *eProsima Integration Service* is using new features that aren't released 
on the master branch of FastRTPS yet. 
To compile it you must swith to the *Develop* branch.

Now, for compiling, if you are on Linux execute:

```bash
    $ mkdir build && cd build
    $ cmake ..
    $ make
```

If you are on Windows choose your version of Visual Studio:

```bash
    > mkdir build && cd build
    > cmake ..  -G "Visual Studio 14 2015 Win64"
    > cmake --build .
```

If you want to compile *eProsima Integration Service* without an installed version of *Fast RTPS* you can add 
an additional parameter that downloads it as a third party library.

If you are on Linux execute:

```bash
    $ mkdir build && cd build
    $ cmake -DTHIRDPARTY=ON ..
    $ make
```

If you are on Windows choose your version of Visual Studio:

```bash
    > mkdir build && cd build
    > cmake ..  -G "Visual Studio 14 2015 Win64" -DTHIRDPARTY=ON
    > cmake --build .
```

<hr></hr>

## Documentation

You can access the documentation online, which is hosted on [Read the Docs](http://eprosima-fast-rtps.readthedocs.io).

<hr></hr>

## Getting Help

If you need support you can reach us by mail at `support@eProsima.com` or by phone at `+34 91 804 34 48`.
