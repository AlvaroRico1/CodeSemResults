void h2o_time2str_rfc1123(char *buf, struct tm *gmt)
{
    char *p = buf;

    /* format: Fri, 19 Sep 2014 05:24:04 GMT */
    p = emit_wday(p, gmt->tm_wday);
    *p++ = ',';
    *p++ = ' ';
    p = emit_digits(p, gmt->tm_mday, 2);
    *p++ = ' ';
    p = emit_mon(p, gmt->tm_mon);
    *p++ = ' ';
    p = emit_digits(p, gmt->tm_year + 1900, 4);
    *p++ = ' ';
    p = emit_digits(p, gmt->tm_hour, 2);
    *p++ = ':';
    p = emit_digits(p, gmt->tm_min, 2);
    *p++ = ':';
    p = emit_digits(p, gmt->tm_sec, 2);
    memcpy(p, " GMT", 4);
    p += 4;
    *p = '\0';

    assert(p - buf == H2O_TIMESTR_RFC1123_LEN);
}


// Source: time.c
// Lines 52-76
