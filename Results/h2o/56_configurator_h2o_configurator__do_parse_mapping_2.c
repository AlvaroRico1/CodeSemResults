int h2o_configurator__do_parse_mapping(h2o_configurator_command_t *cmd, yoml_t *node, const char *keys_required,
                                       const char *keys_optional, yoml_t ****values, size_t num_values)
{
    struct {
        h2o_iovec_t key;
        int is_required;
        unsigned type_mask;
    } *keys = alloca(sizeof(keys[0]) * num_values);
    size_t i, j;

    assert(node->type == YOML_TYPE_MAPPING);

    /* parse keys */
    i = 0;
    if (keys_required != NULL) {
        const char *p = keys_required;
        for (; p != NULL; ++i) {
            assert(i < num_values);
            p = get_next_key(p, &keys[i].key, &keys[i].type_mask);
            keys[i].is_required = 1;
        }
    }


// Source: configurator.c
// Lines 1330-1351
