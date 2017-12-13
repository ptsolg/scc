#include "scc/cc/cc.h"

int main(int argc, const char** argv)
{
        serrcode result = S_ERROR;
        cc_instance cc;
        cc_init(&cc, stdout);

        char cd[S_MAX_PATH_LEN + 1];
        if (S_FAILED(path_get_cd(cd)))
                goto cleanup;
        if (S_FAILED(cc_add_source_dir(&cc, cd)))
                goto cleanup;

        if (S_SUCCEEDED(cc_parse_opts(&cc, argc, argv)))
                result = cc_run(&cc);

cleanup:
        cc_dispose(&cc);
        return result;
}
