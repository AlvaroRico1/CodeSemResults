void afterErrorReply(client *c, const char *s, size_t len) {
    /* Increment the global error counter */
    server.stat_total_error_replies++;
    /* Increment the error stats
     * If the string already starts with "-..." then the error prefix
     * is provided by the caller ( we limit the search to 32 chars). Otherwise we use "-ERR". */
    if (s[0] != '-') {
        incrementErrorCount("ERR", 3);
    } else {
        char *spaceloc = memchr(s, ' ', len < 32 ? len : 32);
        if (spaceloc) {
            const size_t errEndPos = (size_t)(spaceloc - s);
            incrementErrorCount(s+1, errEndPos-1);
        } else {
            /* Fallback to ERR if we can't retrieve the error prefix */
            incrementErrorCount("ERR", 3);
        }
    }


// Source: networking.c
// Lines 419-436
