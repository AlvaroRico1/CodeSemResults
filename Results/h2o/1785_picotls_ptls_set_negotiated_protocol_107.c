int ptls_set_negotiated_protocol(ptls_t *tls, const char *protocol, size_t protocol_len)
{
    char *duped = NULL;

    if (protocol != NULL) {
        if (protocol_len == 0)
            protocol_len = strlen(protocol);
        if ((duped = malloc(protocol_len + 1)) == NULL)
            return PTLS_ERROR_NO_MEMORY;
        memcpy(duped, protocol, protocol_len);
        duped[protocol_len] = '\0';
    }

    free(tls->negotiated_protocol);
    tls->negotiated_protocol = duped;

    return 0;
}


// Source: picotls.c
// Lines 4457-4474
