#include "scc/cc/cc.h"

int main(int argc, const char** argv)
{
        serrcode result = S_ERROR;
        jmp_buf on_fatal_error;
        scc_cc cc;

        if (setjmp(on_fatal_error))
                goto cleanup;

        scc_cc_init(&cc, stderr, on_fatal_error);
        if (S_FAILED(scc_cc_parse_opts(&cc, argc, argv)))
                goto cleanup;

        result = scc_cc_run(&cc);

cleanup:
        scc_cc_dispose(&cc);
        return result;

}
