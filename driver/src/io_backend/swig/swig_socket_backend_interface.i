%module xaiesocket
%{
#include "swig_socket_interface.h"
%}

%include "stdint.i"

%apply uint8_t { u8, uint8_t };
%apply uint64_t { u64, uint64_t };
%apply int { int, u32, s8 };

%include "swig_socket_interface.h"

