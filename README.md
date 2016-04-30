# esexpr.hh
## Version

0.0.0

## Introduction

Esexpr is a toy subset grammar for Emacs Lisp like S-expression written with [PEGTL](https://github.com/ColinH/PEGTL).

This grammar supports parsing:

* proper lists
* vectors
* strings
* characters
* floating point numbers
* integers (decimal, hexadecimal, binary)
* symbols
* quotes
* circular-objects
* comments

and does *not* support:

* dotted-pairs (improper lists)
* octal integers and radix-n integers
* infinities and NaNs
* backquotes

NB: Strings and characters parser may be overly permissive for escape sequences.

## Requirements

* C++ 11
* [PEGTL](https://github.com/ColinH/PEGTL) 1.3.1
* cmake

## How to use

```
#include <esexpr.hh>
```

## Compile and Run Examples

```
% mkdir -p build
% cd build
% cmake ..
% make
% cd exapmles
% ./example -f example.el
```

## Run Tests

```
% mkdir -p build
% cd build
% cmake ..
% env CTEST_OUTPUT_ON_FAILURE=1 make all test
```
