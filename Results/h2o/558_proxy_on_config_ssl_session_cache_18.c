static int on_config_ssl_session_cache(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct proxy_configurator_t *self = (void *)cmd->configurator;
    size_t capacity = 0;
    uint64_t duration = 0;
    h2o_cache_t *current_cache = h2o_socket_ssl_get_session_cache(self->vars->ssl_ctx);

    switch (node->type) {
    case YOML_TYPE_SCALAR:
        if (strcasecmp(node->data.scalar, "OFF") == 0) {
            if (current_cache != NULL) {
                /* set the cache NULL */
                h2o_cache_t *empty_cache = NULL;
                update_ssl_ctx(&self->vars->ssl_ctx, NULL, -1, &empty_cache);
            }
            return 0;
        } else if (strcasecmp(node->data.scalar, "ON") == 0) {
            /* use default values */
            capacity = H2O_DEFAULT_PROXY_SSL_SESSION_CACHE_CAPACITY;
            duration = H2O_DEFAULT_PROXY_SSL_SESSION_CACHE_DURATION;
        } else {
            h2o_configurator_errprintf(cmd, node, "scalar argument must be either of: `OFF`, `ON`");
            return -1;
        }
        break;
    case YOML_TYPE_MAPPING: {
        yoml_t **capacity_node, **lifetime_node;
        if (h2o_configurator_parse_mapping(cmd, node, "capacity:*,lifetime:*", NULL, &capacity_node, &lifetime_node) != 0)
            return -1;
        if (h2o_configurator_scanf(cmd, *capacity_node, "%zu", &capacity) != 0)
            return -1;
        if (capacity == 0) {
            h2o_configurator_errprintf(cmd, *capacity_node, "capacity must be greater than zero");
            return -1;
        }
        unsigned lifetime = 0;
        if (h2o_configurator_scanf(cmd, *lifetime_node, "%u", &lifetime) != 0)
            return -1;
        if (lifetime == 0) {
            h2o_configurator_errprintf(cmd, *lifetime_node, "lifetime must be greater than zero");
            return -1;
        }
        duration = (uint64_t)lifetime * 1000;
    } break;
    default:
        h2o_configurator_errprintf(cmd, node, "node must be a scalar or a mapping");
        return -1;
    }

    if (current_cache != NULL) {
        size_t current_capacity = h2o_cache_get_capacity(current_cache);
        uint64_t current_duration = h2o_cache_get_duration(current_cache);
        if (capacity == current_capacity && duration == current_duration) {
            /* parameters aren't changed, so reuse it */
            return 0;
        }
    }

    h2o_cache_t *new_cache = create_ssl_session_cache(capacity, duration);
    update_ssl_ctx(&self->vars->ssl_ctx, NULL, -1, &new_cache);
    return 0;
}


// Source: proxy.c
// Lines 244-305
