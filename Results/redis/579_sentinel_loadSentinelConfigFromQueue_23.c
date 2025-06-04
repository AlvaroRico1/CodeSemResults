void loadSentinelConfigFromQueue(void) {
    const char *err = NULL;
    listIter li;
    listNode *ln;
    int linenum = 0;
    sds line = NULL;

    /* if there is no sentinel_config entry, we can return immediately */
    if (server.sentinel_config == NULL) return;

    /* loading from pre monitor config queue first to avoid dependency issues */
    listRewind(server.sentinel_config->pre_monitor_cfg,&li);
    while((ln = listNext(&li))) {
        struct sentinelLoadQueueEntry *entry = ln->value;
        err = sentinelHandleConfiguration(entry->argv,entry->argc);
        if (err) {
            linenum = entry->linenum;
            line = entry->line;
            goto loaderr;
        }
    }

    /* loading from monitor config queue */
    listRewind(server.sentinel_config->monitor_cfg,&li);
    while((ln = listNext(&li))) {
        struct sentinelLoadQueueEntry *entry = ln->value;
        err = sentinelHandleConfiguration(entry->argv,entry->argc);
        if (err) {
            linenum = entry->linenum;
            line = entry->line;
            goto loaderr;
        }
    }


// Source: sentinel.c
// Lines 1842-1874
