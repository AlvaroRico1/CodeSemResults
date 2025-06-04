static int on_config_quic_nodes(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    yoml_t **self_node, **mapping_node;

    if (h2o_configurator_parse_mapping(cmd, node, "self:s,mapping:m", NULL, &self_node, &mapping_node) != 0)
        return -1;

    /* obtain node-id of this server */
    if (h2o_configurator_scanf(cmd, *self_node, "%" SCNu64, &conf.quic.node_id) != 0)
        return -1;

    /* build list of servers */
    h2o_vector_reserve(NULL, &conf.quic.forward_nodes, (*mapping_node)->data.mapping.size);
    size_t i;
    for (i = 0; i != (*mapping_node)->data.mapping.size; ++i) {
        if (configure_quic_forward_node(cmd, conf.quic.forward_nodes.entries + i, (*mapping_node)->data.mapping.elements + i) != 0)
            return -1;
    }
    conf.quic.forward_nodes.size = (*mapping_node)->data.mapping.size;

    return 0;
}


// Source: main.c
// Lines 2312-2333
