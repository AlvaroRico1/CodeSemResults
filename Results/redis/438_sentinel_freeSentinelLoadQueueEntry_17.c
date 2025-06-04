void freeSentinelLoadQueueEntry(void *item) {
    struct sentinelLoadQueueEntry *entry = item;
    sdsfreesplitres(entry->argv,entry->argc);
    sdsfree(entry->line);
    zfree(entry);
}


// Source: sentinel.c
// Lines 1803-1808
