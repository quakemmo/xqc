#include "tr_local.h"

void tr_XQ_SetShaderFlags(qhandle_t qh) {

    tr.shaders[qh]->stages[0]->stateBits =  GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;

}

