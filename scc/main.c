#include "scc-env.h"

int main(int argc, const char** argv)
{
        scc_env scc;
        if (S_FAILED(scc_init(&scc, stderr, argc, argv)))
                return 0;

        int res = scc_run(&scc);
        scc_dispose(&scc);
        return res;
}
