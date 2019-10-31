# GEGELATI
_Generic Evolvable Graphs for Efficient Learning of Artificial Tangled Intelligence_

GEGELATI _\[dʒedʒelati\]_ is a fresh open-source reinforcement learning framework for training artificial intelligence based on [Tangled Program Graphs (TPGs)](http://stephenkelly.ca/research_files/Kelly-Stephen-PhD-CSCI-June-2018.pdf). The purpose of this framework, developed as a C++ shared library, is to make it as easy and as fast as possible and to train an agent on a new learning environment. The C++ library is developed to be portable, fully documented, and thoroughly unit tested to ensure its maintainability. GEGELATI is developed at the Institute of Electronics and Telecommunications-Rennes (IETR).

## Overview
* [Continuous Integration](#-continuous-integration)
* [Build and Install the Library](#-build-and-install-the-library)
* [How to Use the GEGELATI Library](#-how-to-use-the-gegelati-library)
* [License](#-license)

## Continuous Integration
_(coming soon...)_

## Build and Install the Library
### Binaries
_(coming soon...)_
Because being too lazy to build the library yourself is OK too.

### Build tools
The build process of GEGELATI relies on [cmake](https://cmake.org) to configure a project for a wide variety of development environments and operating systems. Install [cmake](https://cmake.org/download/) on your system before building the library.

The GEGELATI code is annotated with the [doxygen](http://www.doxygen.nl/) syntax. Because having a proper code documentation is fundamental, its generation is a mandatory part of the build process. Install [doxygen](http://www.doxygen.nl/download.html) on your system before building the library.

### Hammer time!
To build the shared library (and its code documentation), you can execute the following commands:

```shell
git clone https://github.com/gegelati/gegelati.git
cd gegelati/bin
cmake ..
cmake --build . --target INSTALL
```

On Windows, the shared library will be installed in the `gegelati/bin/gegelatilib-<version>` folder.

## How to Use the GEGELATI Library
### Learn by Examples
Since sometimes, a simple example is worth 10.000 pages of documentations, examples of applications built with the GEGELATI library are available in the [gegelati-apps repository](https://github.com/gegelati/gegelati-apps).

### Utilization Guide
_(10.000 pages of documentation (or less) coming soon...)_


## License
This project is distributed under the CeCILL-C license (see [LICENSE file](LICENSE)).