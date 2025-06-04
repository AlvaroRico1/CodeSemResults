static struct addrinfo *resolve_address(h2o_configurator_command_t *cmd, yoml_t *node, int socktype, int protocol,
                                        const char *hostname, const char *servname)
{
    struct addrinfo hints, *res;
    int error;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = socktype;
    hints.ai_protocol = protocol;
    hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV | AI_PASSIVE;

    if ((error = getaddrinfo(hostname, servname, &hints, &res)) != 0) {
        h2o_configurator_errprintf(cmd, node, "failed to resolve address: %s", gai_strerror(error));
        return NULL;
    } else if (res == NULL) {
        h2o_configurator_errprintf(cmd, node, "failed to resolve address: getaddrinfo returned an empty list");
        return NULL;
    }

    return res;
}


// Source: main.c
// Lines 1739-1759
