#ifndef __CUSTOM_DEF_H__
#define __CUSTOM_DEF_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#define PERFORMANCE_RUN 1

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#if defined(__clang__)
#define COMPILER_NAME "Clang " __VERSION__
#elif defined(__GNUC__)
#define COMPILER_NAME "GCC " __VERSION__
#else
#define COMPILER_NAME "Unknown Compiler"
#endif

#endif
