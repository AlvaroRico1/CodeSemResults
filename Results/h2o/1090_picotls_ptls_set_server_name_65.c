int ptls_set_server_name(ptls_t *tls, const char *server_name, size_t server_name_len)
{
    char *duped = NULL;

    if (server_name != NULL) {
        if (server_name_len == 0)
            server_name_len = strlen(server_name);
        if ((duped = malloc(server_name_len + 1)) == NULL)
            return PTLS_ERROR_NO_MEMORY;
        memcpy(duped, server_name, server_name_len);
        duped[server_name_len] = '\0';
    }

    free(tls->server_name);
    tls->server_name = duped;

    return 0;
}


// Source: picotls.c
// Lines 4433-4450
