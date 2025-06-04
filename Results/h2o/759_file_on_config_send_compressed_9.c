static int on_config_send_compressed(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct st_h2o_file_configurator_t *self = (void *)cmd->configurator;

    switch (h2o_configurator_get_one_of(cmd, node, "OFF,ON,gunzip")) {
    case 0: /* off */
        self->vars->flags &= ~H2O_FILE_FLAG_SEND_COMPRESSED;
        break;
    case 1: /* on */
        self->vars->flags |= H2O_FILE_FLAG_SEND_COMPRESSED;
        break;
    case 2: /* gunzip */
        self->vars->flags |= (H2O_FILE_FLAG_SEND_COMPRESSED | H2O_FILE_FLAG_GUNZIP);
        break;
    default: /* error */
        return -1;
    }

    return 0;
}


// Source: file.c
// Lines 91-110
