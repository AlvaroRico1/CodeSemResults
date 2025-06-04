h2o_logconf_t *h2o_logconf_compile(const char *fmt, int escape, char *errbuf)
{
    h2o_logconf_t *logconf = h2o_mem_alloc(sizeof(*logconf));
    const char *pt = fmt;
    size_t fmt_len = strlen(fmt);

    *logconf = (h2o_logconf_t){{NULL}, escape};

#define LAST_ELEMENT() (logconf->elements.entries + logconf->elements.size - 1)
/* suffix buffer is always guaranteed to be larger than the fmt + (sizeof('\n') - 1) (so that they would be no buffer overruns) */
#define NEW_ELEMENT(ty)                                                                                                            \
    do {                                                                                                                           \
        h2o_vector_reserve(NULL, &logconf->elements, logconf->elements.size + 1);                                                  \
        logconf->elements.size++;                                                                                                  \
        *LAST_ELEMENT() = (struct log_element_t){0};                                                                               \
        LAST_ELEMENT()->type = ty;                                                                                                 \
        LAST_ELEMENT()->suffix.base = h2o_mem_alloc(fmt_len + 1);                                                                  \
    } while (0)

    while (*pt != '\0') {
        if (memcmp(pt, "%%", 2) == 0) {
            ++pt; /* emit % */
        } else if (*pt == '%') {
            ++pt;
            /* handle < and > */
            int log_original = 0;
            for (;; ++pt) {
                if (*pt == '<') {
                    log_original = 1;
                } else if (*pt == '>') {
                    log_original = 0;
                } else {
                    break;
                }
            }
            /* handle {...}n */
            if (*pt == '{') {
                const h2o_token_t *token;
                const char *quote_end = strchr(++pt, '}');


// Source: logconf.c
// Lines 148-186
