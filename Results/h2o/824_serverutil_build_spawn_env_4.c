static char **build_spawn_env(void)
{
    extern char **environ;
    size_t num;

    /* calculate number of envvars, as well as looking for H2O_ROOT= */
    for (num = 0; environ[num] != NULL; ++num)
        if (strncmp(environ[num], "H2O_ROOT=", sizeof("H2O_ROOT=") - 1) == 0)
            return NULL;

    /* not found */
    char **newenv = h2o_mem_alloc(sizeof(*newenv) * (num + 2) + sizeof("H2O_ROOT=" H2O_TO_STR(H2O_ROOT)));
    memcpy(newenv, environ, sizeof(*newenv) * num);
    newenv[num] = (char *)(newenv + num + 2);
    newenv[num + 1] = NULL;
    strcpy(newenv[num], "H2O_ROOT=" H2O_TO_STR(H2O_ROOT));

    return newenv;
}


// Source: serverutil.c
// Lines 122-140
