#ifndef SCC_CC_RUN_H
#define SCC_CC_RUN_H

#include "cc.h"

extern serrcode scc_cc_lex(scc_cc* self);
extern serrcode scc_cc_parse(scc_cc* self);
extern serrcode scc_cc_compile(scc_cc* self);
extern serrcode scc_cc_link(scc_cc* self);

#endif