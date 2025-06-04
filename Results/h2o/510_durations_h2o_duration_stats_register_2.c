void h2o_duration_stats_register(h2o_globalconf_t *conf)
{
    int i, k;
    h2o_logger_t *logger;
    h2o_hostconf_t *hconf;

    durations_logger = logger = h2o_mem_alloc(sizeof(*logger));
    memset(logger, 0, sizeof(*logger));
    logger->_config_slot = conf->_num_config_slots++;
    logger->log_access = stat_access;
    logger->on_context_init = on_context_init;
    logger->on_context_dispose = on_context_dispose;

    for (k = 0; conf->hosts[k]; k++) {
        hconf = conf->hosts[k];
        for (i = 0; i < hconf->paths.size; i++) {
            int j;
            for (j = 0; j < hconf->paths.entries[i]->handlers.size; j++) {
                h2o_pathconf_t *pathconf = hconf->paths.entries[i];
                h2o_vector_reserve(NULL, &pathconf->_loggers, pathconf->_loggers.size + 1);
                pathconf->_loggers.entries[pathconf->_loggers.size++] = (void *)logger;
            }
        }
    }
}


// Source: durations.c
// Lines 205-229
