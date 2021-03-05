#include "scc.h"

int main(int argc, const char** argv)
{
        errcode result = EC_ERROR;
        scc_env env;
        scc_init(&env);

        if (EC_SUCCEEDED(scc_setup(&env, argc, argv)))
                result = cc_run(&env.cc);
        
        scc_dispose(&env);
        return result;
}
