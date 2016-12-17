#ifndef CANARY_CONFIG_H_
#define CANARY_CONFIG_H_

// Very important file. After change it all files be compiled if(!) it insert first in *.c
//   or *.cc file.
// Template:
//
// some.cc
// #include "canary/config.h"  // !!""
// #include "canary/engine/someUnit.h"  // !!""
//
// // Sys libs - if used
// #include <stdio.h>  // !!<>
//
// // Third party
// #include <some_path/cuda.h>  // !!<>
//
// // Inner reuse
// #include "canary/inner_reuse/own_utils.h"  // !!""
//
// // App - applications
// #include "canary/unit.h"

static const int GkMaxCountChannels = 4;

#define G_SOME_FEATURE  // G_* - global

#endif  // CANARY_CONFIG_H_
