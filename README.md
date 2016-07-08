# petrovich-c

## Introduction

petrovich-c is a C implementation of [Petrovich](https://github.com/petrovich), a library to inflect Russian
anthroponyms such as first names, last names, and middle names.

## Requirements

This library requires:

 * A C99-compatible compiler (tested with GCC 4.9 and GCC 5)
 * [CMake](https://cmake.org) 3.0 or later
 * [libyaml](http://pyyaml.org/wiki/LibYAML) (tested with v. 0.1.5)

In order to run the testsuite:

 * [Python 3](https://python.org)

## Third-party components

The library uses a [rules file](rules.yml), taken from petrovich
[github repo](https://github.com/petrovich/petrovich-rules).

## Examples

See [test.c](test/test.c) for API usage example.

## License

This library is distributed under permissive [MIT License](LICENSE.md).
