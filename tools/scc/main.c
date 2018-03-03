#include "scc.h"

int main(int argc, const char** argv)
{
        errcode result = EC_ERROR;
        scc_env env;
        scc_env_init(&env);
        if (EC_SUCCEEDED(scc_env_setup(&env, argc, argv)))
                result = cc_run(&env.cc);
        scc_env_dispose(&env);
        return result;
}
