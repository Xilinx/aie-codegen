%module xaiecontrolcode
%{
#include "swig_controlcode_interface.h"
%}

%include "stdint.i"

%apply uint8_t { u8, uint8_t };
%apply uint64_t { u64, uint64_t };
%apply uint32_t { u32, uint32_t };
%apply int { u32 };
%apply uint32_t * { u32 *, uint32_t * };

%{
    #include <stdlib.h>  /* For malloc and free */
    
    /* Custom wrapper for BlockWrite32 that accepts a Python list */
    int XAie_ControlCodeIO_swig_BlockWrite32_List(Swig_DevInst *DevInst, u32 RegAddr, PyObject *list, u32 NumWords) {
        u32 i;
        int result = -1;
        u32 *data = NULL;
        
        /* Validate input parameters */
        if (!DevInst || !list || !PyList_Check(list)) {
            return -1;
        }
        
        /* Validate list size matches NumWords */
        if (PyList_Size(list) != NumWords) {
            return -2;
        }
        
        /* Allocate memory for the array */
        data = (u32 *)malloc(NumWords * sizeof(u32));
        if (!data) {
            return -3;  /* Memory allocation failure */
        }
        
        /* Copy data from Python list to C array */
        for (i = 0; i < NumWords; i++) {
            PyObject *item = PyList_GetItem(list, i);
            if (!item) {
                free(data);
                return -4;  /* List access error */
            }
            
            /* Convert Python integer to unsigned 32-bit value */
            if (PyLong_Check(item)) {
                data[i] = (u32)PyLong_AsUnsignedLong(item);
            } else {
                free(data);
                return -5;  /* Type conversion error */
            }
        }
        
        /* Call the actual BlockWrite32 function with our properly typed array */
        result = XAie_ControlCodeIO_swig_BlockWrite32(DevInst, RegAddr, data, NumWords);
        
        /* Free the allocated memory */
        free(data);
        
        return result;
    }
    %}
    
    /* Expose the new function to Python */
    int XAie_ControlCodeIO_swig_BlockWrite32_List(Swig_DevInst *DevInst, u32 RegAddr, PyObject *list, u32 NumWords);

%include "swig_controlcode_interface.h"

