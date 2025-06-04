static h2o_socketpool_target_t *parse_backend(h2o_configurator_command_t *cmd, yoml_t *backend)
{
    yoml_t **url_node;
    h2o_socketpool_target_conf_t lb_per_target_conf = {0}; /* default weight of each target */

    switch (backend->type) {
    case YOML_TYPE_SCALAR:
        url_node = &backend;
        break;
    case YOML_TYPE_MAPPING: {
        yoml_t **weight_node;
        if (h2o_configurator_parse_mapping(cmd, backend, "url:s", "weight:*", &url_node, &weight_node) != 0)
            return NULL;
        if (weight_node != NULL) {
            unsigned weight;
            if (h2o_configurator_scanf(cmd, *weight_node, "%u", &weight) != 0)
                return NULL;
            if (!(1 <= weight && weight <= H2O_SOCKETPOOL_TARGET_MAX_WEIGHT)) {
                h2o_configurator_errprintf(cmd, *weight_node, "weight must be an integer in range 1 - 256");
                return NULL;
            }
            lb_per_target_conf.weight_m1 = weight - 1;
        }
    } break;
    default:
        h2o_configurator_errprintf(cmd, backend,
                                   "items of arguments passed to proxy.reverse.url must be either a scalar or a mapping");
        return NULL;
    }

    h2o_url_t url;
    if (h2o_url_parse((*url_node)->data.scalar, SIZE_MAX, &url) != 0) {
        h2o_configurator_errprintf(cmd, *url_node, "failed to parse URL: %s\n", (*url_node)->data.scalar);
        return NULL;
    }
    return h2o_socketpool_create_target(&url, &lb_per_target_conf);
}


// Source: proxy.c
// Lines 307-343
