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
                if (quote_end == NULL) {
                    sprintf(errbuf, "failed to compile log format: unterminated header name starting at: \"%16s\"", pt);
                    goto Error;
                }
                const char modifier = quote_end[1];
                switch (modifier) {
                case 'i':
                case 'o': {
                    h2o_iovec_t name = strdup_lowercased(pt, quote_end - pt);
                    token = h2o_lookup_token(name.base, name.len);
                    if (token != NULL) {
                        free(name.base);
                        if (modifier == 'o' && token == H2O_TOKEN_SET_COOKIE) {
                            NEW_ELEMENT(ELEMENT_TYPE_OUT_HEADER_TOKEN_CONCATENATED);
                            LAST_ELEMENT()->data.header_token = token;
                        } else {
                            NEW_ELEMENT(modifier == 'i' ? ELEMENT_TYPE_IN_HEADER_TOKEN : ELEMENT_TYPE_OUT_HEADER_TOKEN);
                            LAST_ELEMENT()->data.header_token = token;
                        }
                    } else {
                        NEW_ELEMENT(modifier == 'i' ? ELEMENT_TYPE_IN_HEADER_STRING : ELEMENT_TYPE_OUT_HEADER_STRING);
                        LAST_ELEMENT()->data.name = name;
                    }
                    LAST_ELEMENT()->original_response = log_original;
                } break;
                case 'p':
                    if (h2o_memis(pt, quote_end - pt, H2O_STRLIT("local"))) {
                        NEW_ELEMENT(ELEMENT_TYPE_LOCAL_PORT);
                    } else if (h2o_memis(pt, quote_end - pt, H2O_STRLIT("remote"))) {
                        NEW_ELEMENT(ELEMENT_TYPE_REMOTE_PORT);
                    } else {
                        sprintf(errbuf, "failed to compile log format: unknown specifier for %%{...}p");
                        goto Error;
                    }
                    break;
                case 'e': {
                    h2o_iovec_t name = h2o_strdup(NULL, pt, quote_end - pt);
                    NEW_ELEMENT(ELEMENT_TYPE_ENV_VAR);
                    LAST_ELEMENT()->data.name = name;
                } break;
                case 't':
                    if (h2o_memis(pt, quote_end - pt, H2O_STRLIT("sec"))) {
                        NEW_ELEMENT(ELEMENT_TYPE_TIMESTAMP_SEC_SINCE_EPOCH);
                    } else if (h2o_memis(pt, quote_end - pt, H2O_STRLIT("msec"))) {
                        NEW_ELEMENT(ELEMENT_TYPE_TIMESTAMP_MSEC_SINCE_EPOCH);
                    } else if (h2o_memis(pt, quote_end - pt, H2O_STRLIT("usec"))) {
                        NEW_ELEMENT(ELEMENT_TYPE_TIMESTAMP_USEC_SINCE_EPOCH);
                    } else if (h2o_memis(pt, quote_end - pt, H2O_STRLIT("msec_frac"))) {
                        NEW_ELEMENT(ELEMENT_TYPE_TIMESTAMP_MSEC_FRAC);
                    } else if (h2o_memis(pt, quote_end - pt, H2O_STRLIT("usec_frac"))) {
                        NEW_ELEMENT(ELEMENT_TYPE_TIMESTAMP_USEC_FRAC);
                    } else {
                        h2o_iovec_t name = h2o_strdup(NULL, pt, quote_end - pt);
                        NEW_ELEMENT(ELEMENT_TYPE_TIMESTAMP_STRFTIME);
                        LAST_ELEMENT()->data.name = name;
                    }
                    break;
                case 'x':
#define MAP_EXT_TO_TYPE(name, id)                                                                                                  \
    if (h2o_lcstris(pt, quote_end - pt, H2O_STRLIT(name))) {                                                                       \
        NEW_ELEMENT(id);                                                                                                           \
        goto MAP_EXT_Found;                                                                                                        \
    }
#define MAP_EXT_TO_PROTO(name, cb)                                                                                                 \
    if (h2o_lcstris(pt, quote_end - pt, H2O_STRLIT(name))) {                                                                       \
        h2o_conn_callbacks_t dummy_;                                                                                               \
        NEW_ELEMENT(ELEMENT_TYPE_PROTOCOL_SPECIFIC);                                                                               \
        LAST_ELEMENT()->data.protocol_specific_callback_index = &dummy_.log_.cb - dummy_.log_.callbacks;                           \
        goto MAP_EXT_Found;                                                                                                        \
    }
                    MAP_EXT_TO_TYPE("connection-id", ELEMENT_TYPE_CONNECTION_ID);
                    MAP_EXT_TO_TYPE("connect-time", ELEMENT_TYPE_CONNECT_TIME);
                    MAP_EXT_TO_TYPE("request-total-time", ELEMENT_TYPE_REQUEST_TOTAL_TIME);
                    MAP_EXT_TO_TYPE("request-header-time", ELEMENT_TYPE_REQUEST_HEADER_TIME);
                    MAP_EXT_TO_TYPE("request-body-time", ELEMENT_TYPE_REQUEST_BODY_TIME);
                    MAP_EXT_TO_TYPE("process-time", ELEMENT_TYPE_PROCESS_TIME);
                    MAP_EXT_TO_TYPE("response-time", ELEMENT_TYPE_RESPONSE_TIME);
                    MAP_EXT_TO_TYPE("duration", ELEMENT_TYPE_TOTAL_TIME);
                    MAP_EXT_TO_TYPE("total-time", ELEMENT_TYPE_TOTAL_TIME);
                    MAP_EXT_TO_TYPE("error", ELEMENT_TYPE_ERROR);
                    MAP_EXT_TO_TYPE("proxy.idle-time", ELEMENT_TYPE_PROXY_IDLE_TIME);
                    MAP_EXT_TO_TYPE("proxy.connect-time", ELEMENT_TYPE_PROXY_CONNECT_TIME);
                    MAP_EXT_TO_TYPE("proxy.request-time", ELEMENT_TYPE_PROXY_REQUEST_TIME);
                    MAP_EXT_TO_TYPE("proxy.process-time", ELEMENT_TYPE_PROXY_PROCESS_TIME);
                    MAP_EXT_TO_TYPE("proxy.response-time", ELEMENT_TYPE_PROXY_RESPONSE_TIME);
                    MAP_EXT_TO_TYPE("proxy.total-time", ELEMENT_TYPE_PROXY_TOTAL_TIME);
                    MAP_EXT_TO_TYPE("proxy.request-bytes", ELEMENT_TYPE_PROXY_REQUEST_BYTES);
                    MAP_EXT_TO_TYPE("proxy.request-bytes-header", ELEMENT_TYPE_PROXY_REQUEST_BYTES_HEADER);
                    MAP_EXT_TO_TYPE("proxy.request-bytes-body", ELEMENT_TYPE_PROXY_REQUEST_BYTES_BODY);
                    MAP_EXT_TO_TYPE("proxy.response-bytes", ELEMENT_TYPE_PROXY_RESPONSE_BYTES);
                    MAP_EXT_TO_TYPE("proxy.response-bytes-header", ELEMENT_TYPE_PROXY_RESPONSE_BYTES_HEADER);
                    MAP_EXT_TO_TYPE("proxy.response-bytes-body", ELEMENT_TYPE_PROXY_RESPONSE_BYTES_BODY);
                    MAP_EXT_TO_TYPE("proxy.ssl.protocol-version", ELEMENT_TYPE_PROXY_SSL_PROTOCOL_VERSION);
                    MAP_EXT_TO_TYPE("proxy.ssl.session-reused", ELEMENT_TYPE_PROXY_SSL_SESSION_REUSED);
                    MAP_EXT_TO_TYPE("proxy.ssl.cipher", ELEMENT_TYPE_PROXY_SSL_CIPHER);
                    MAP_EXT_TO_TYPE("proxy.ssl.cipher-bits", ELEMENT_TYPE_PROXY_SSL_CIPHER_BITS);
                    MAP_EXT_TO_PROTO("http1.request-index", http1.request_index);
                    MAP_EXT_TO_PROTO("http2.stream-id", http2.stream_id);
                    MAP_EXT_TO_PROTO("http2.priority.received", http2.priority_received);
                    MAP_EXT_TO_PROTO("http2.priority.received.exclusive", http2.priority_received_exclusive);
                    MAP_EXT_TO_PROTO("http2.priority.received.parent", http2.priority_received_parent);
                    MAP_EXT_TO_PROTO("http2.priority.received.weight", http2.priority_received_weight);
                    MAP_EXT_TO_PROTO("http2.priority.actual", http2.priority_actual);
                    MAP_EXT_TO_PROTO("http2.priority.actual.parent", http2.priority_actual_parent);
                    MAP_EXT_TO_PROTO("http2.priority.actual.weight", http2.priority_actual_weight);
                    MAP_EXT_TO_PROTO("http3.stream-id", http3.stream_id);
                    MAP_EXT_TO_PROTO("http3.quic-stats", http3.quic_stats);
                    MAP_EXT_TO_PROTO("http3.quic-version", http3.quic_version);
                    MAP_EXT_TO_PROTO("cc.name", transport.cc_name);
                    MAP_EXT_TO_PROTO("delivery-rate", transport.delivery_rate);
                    MAP_EXT_TO_PROTO("ssl.protocol-version", ssl.protocol_version);
                    MAP_EXT_TO_PROTO("ssl.session-reused", ssl.session_reused);
                    MAP_EXT_TO_PROTO("ssl.cipher", ssl.cipher);
                    MAP_EXT_TO_PROTO("ssl.cipher-bits", ssl.cipher_bits);
                    MAP_EXT_TO_PROTO("ssl.session-id", ssl.session_id);
                    MAP_EXT_TO_PROTO("ssl.server-name", ssl.server_name);
                    MAP_EXT_TO_PROTO("ssl.negotiated-protocol", ssl.negotiated_protocol);
                    { /* not found */
                        h2o_iovec_t name = strdup_lowercased(pt, quote_end - pt);
                        NEW_ELEMENT(ELEMENT_TYPE_EXTENDED_VAR);
                        LAST_ELEMENT()->data.name = name;
                    }
                MAP_EXT_Found:
#undef MAP_EXT_TO_TYPE
#undef MAP_EXT_TO_PROTO
                    break;
                default:
                    sprintf(errbuf, "failed to compile log format: header name is not followed by either `i`, `o`, `x`, `e`");
                    goto Error;
                }
                pt = quote_end + 2;
                continue;
            } else {
                unsigned type = NUM_ELEMENT_TYPES;
                switch (*pt++) {
#define TYPE_MAP(ch, ty)                                                                                                           \
    case ch:                                                                                                                       \
        type = ty;                                                                                                                 \
        break
                    TYPE_MAP('A', ELEMENT_TYPE_LOCAL_ADDR);
                    TYPE_MAP('b', ELEMENT_TYPE_BYTES_SENT);
                    TYPE_MAP('H', ELEMENT_TYPE_PROTOCOL);
                    TYPE_MAP('h', ELEMENT_TYPE_REMOTE_ADDR);
                    TYPE_MAP('l', ELEMENT_TYPE_LOGNAME);
                    TYPE_MAP('m', ELEMENT_TYPE_METHOD);
                    TYPE_MAP('p', ELEMENT_TYPE_LOCAL_PORT);
                    TYPE_MAP('e', ELEMENT_TYPE_ENV_VAR);
                    TYPE_MAP('q', ELEMENT_TYPE_QUERY);
                    TYPE_MAP('r', ELEMENT_TYPE_REQUEST_LINE);
                    TYPE_MAP('s', ELEMENT_TYPE_STATUS);
                    TYPE_MAP('t', ELEMENT_TYPE_TIMESTAMP);
                    TYPE_MAP('U', ELEMENT_TYPE_URL_PATH);
                    TYPE_MAP('u', ELEMENT_TYPE_REMOTE_USER);
                    TYPE_MAP('V', ELEMENT_TYPE_AUTHORITY);
                    TYPE_MAP('v', ELEMENT_TYPE_HOSTCONF);
#undef TYPE_MAP
                default:
                    sprintf(errbuf, "failed to compile log format: unknown escape sequence: %%%c", pt[-1]);
                    goto Error;
                }
                NEW_ELEMENT(type);
                LAST_ELEMENT()->original_response = log_original;
                continue;
            }
        }
        /* emit current char */
        if (logconf->elements.size == 0)
            NEW_ELEMENT(ELEMENT_TYPE_EMPTY);
        LAST_ELEMENT()->suffix.base[LAST_ELEMENT()->suffix.len++] = *pt++;
    }

    /* emit end-of-line */
    if (logconf->elements.size == 0)
        NEW_ELEMENT(ELEMENT_TYPE_EMPTY);
    LAST_ELEMENT()->suffix.base[LAST_ELEMENT()->suffix.len++] = '\n';

#undef NEW_ELEMENT
#undef LAST_ELEMENT

    if (escape == H2O_LOGCONF_ESCAPE_JSON) {
        if (!determine_magicquote_nodes(logconf, errbuf))
            goto Error;
    }

    return logconf;

Error:
    h2o_logconf_dispose(logconf);
    return NULL;
}


// Source: logconf.c
// Lines 148-376
