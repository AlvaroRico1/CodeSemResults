void h2o_configurator_define_command(h2o_configurator_t *configurator, const char *name, int flags, h2o_configurator_command_cb cb)
{
    h2o_configurator_command_t *cmd;

    h2o_vector_reserve(NULL, &configurator->commands, configurator->commands.size + 1);
    cmd = configurator->commands.entries + configurator->commands.size++;
    cmd->configurator = configurator;
    cmd->flags = flags;
    cmd->name = name;
    cmd->cb = cb;
}


// Source: configurator.c
// Lines 1181-1191
