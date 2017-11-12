#ifndef SSA_PRINTER_H
#define SSA_PRINTER_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ssa_printer
{
        int todo;
} ssa_printer;

extern void ssa_init_printer(ssa_printer* self);

#ifdef __cplusplus
}
#endif

#endif