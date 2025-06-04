sentinelRedisInstance *getSentinelRedisInstanceByAddrAndRunID(dict *instances, char *addr, int port, char *runid) {
    dictIterator *di;
    dictEntry *de;
    sentinelRedisInstance *instance = NULL;
    sentinelAddr *ri_addr = NULL;

    serverAssert(addr || runid);   /* User must pass at least one search param. */
    if (addr != NULL) {
        /* Resolve addr, we use the IP as a key even if a hostname is used */
        ri_addr = createSentinelAddr(addr, port);
        if (!ri_addr) return NULL;
    }
    di = dictGetIterator(instances);
    while((de = dictNext(di)) != NULL) {
        sentinelRedisInstance *ri = dictGetVal(de);

        if (runid && !ri->runid) continue;
        if ((runid == NULL || strcmp(ri->runid, runid) == 0) &&
            (addr == NULL || (strcmp(ri->addr->ip, ri_addr->ip) == 0 &&
                            ri->addr->port == port)))
        {
            instance = ri;
            break;
        }
    }


// Source: sentinel.c
// Lines 1482-1506
