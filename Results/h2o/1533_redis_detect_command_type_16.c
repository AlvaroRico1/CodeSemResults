static h2o_redis_command_type_t detect_command_type(const char *formatted)
{
#define CHECK(c)                                                                                                                   \
    if (c == NULL)                                                                                                                 \
    return H2O_REDIS_COMMAND_TYPE_ERROR

    char *p = (char *)formatted;
    CHECK(p);

    assert(p[0] == '*');

    p = strchr(p, '$');
    CHECK(p);
    p = strchr(p, '\n');
    CHECK(p);
    ++p;
    CHECK(p);

#define MATCH(c, target) strncasecmp(c, target, sizeof(target) - 1) == 0
    if (MATCH(p, "subscribe\r\n"))
        return H2O_REDIS_COMMAND_TYPE_SUBSCRIBE;
    if (MATCH(p, "unsubscribe\r\n"))
        return H2O_REDIS_COMMAND_TYPE_UNSUBSCRIBE;
    if (MATCH(p, "psubscribe\r\n"))
        return H2O_REDIS_COMMAND_TYPE_PSUBSCRIBE;
    if (MATCH(p, "punsubscribe\r\n"))
        return H2O_REDIS_COMMAND_TYPE_PUNSUBSCRIBE;
    if (MATCH(p, "monitor\r\n"))
        return H2O_REDIS_COMMAND_TYPE_MONITOR;
#undef MATCH
    return H2O_REDIS_COMMAND_TYPE_NORMAL;
#undef CHECK
}


// Source: redis.c
// Lines 270-302
