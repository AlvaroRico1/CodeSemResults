#define INITIAL_BUF_SIZE 256

    char *errbuf = h2o_mem_alloc_pool(&req->pool, char, INITIAL_BUF_SIZE);
    int errlen;
    va_list args;

    va_start(args, fmt);
    errlen = vsnprintf(errbuf, INITIAL_BUF_SIZE, fmt, args);
    va_end(args);

    if (errlen >= INITIAL_BUF_SIZE) {
        errbuf = h2o_mem_alloc_pool(&req->pool, char, errlen + 1);
        va_start(args, fmt);
        errlen = vsnprintf(errbuf, errlen + 1, fmt, args);
        va_end(args);
    }
    h2o_iovec_t msg = h2o_iovec_init(errbuf, errlen);

#undef INITIAL_BUF_SIZE

    /* build prefix */
    char *pbuf = h2o_mem_alloc_pool(&req->pool, char, sizeof("[] in request::") + 32 + strlen(module)), *p = pbuf;
    p += sprintf(p, "[%s] in request:", module);
    if (req->path.len < 32) {
        memcpy(p, req->path.base, req->path.len);
        p += req->path.len;
    } else {
        memcpy(p, req->path.base, 29);
        p += 29;
        memcpy(p, "...", 3);
        p += 3;
    }
    *p++ = ':';
    h2o_iovec_t prefix = h2o_iovec_init(pbuf, p - pbuf);

    /* run error callback (save and emit the log if needed) */
    req->error_log_delegate.cb(req->error_log_delegate.data, prefix, msg);
}


// Source: request.c
// Lines 724-761
