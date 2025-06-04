static void log_filter_append_item_value(char *out_buf, size_t out_siz,
                                         log_item *li) {
  size_t len = log_bs->length(out_buf);  // used bytes
  size_t out_left = out_siz - len;
  char *out_writepos = out_buf + len;

  if (li->item_class == LOG_FLOAT)
    len =
        log_bs->substitute(out_writepos, out_left, "%lf", li->data.data_float);

  else if (li->item_class == LOG_INTEGER) {
    if (li->type == LOG_ITEM_LOG_PRIO) {
      switch (li->data.data_integer) {
        case ERROR_LEVEL:
          len = log_bs->substitute(out_writepos, out_left, "ERROR");
          break;
        case WARNING_LEVEL:
          len = log_bs->substitute(out_writepos, out_left, "WARNING");
          break;
        case INFORMATION_LEVEL:
          len = log_bs->substitute(out_writepos, out_left, "INFORMATION");
          break;
        default:
          /*
            We have no idea what this is (either breakage, or new
            severities were added to the server that we don't yet
            know about. That's OK though, we can still write the
            numeric value and thereby generate a valid config.
          */
          len = log_bs->substitute(out_writepos, out_left, "%lld",
                                   li->data.data_integer);
      }
    } else if (li->type == LOG_ITEM_SQL_ERRCODE) {
      len = log_bs->substitute(out_writepos, out_left, "MY-%06lld",
                               li->data.data_integer);
    } else {
      len = log_bs->substitute(out_writepos, out_left, "%lld",
                               li->data.data_integer);
    }
  }

  else if (log_bi->item_string_class(li->item_class) &&
           (li->data.data_string.str != nullptr)) {
    len = log_bs->substitute(out_writepos, out_left, "\"%.*s\"",
                             (int)li->data.data_string.length,
                             li->data.data_string.str);
  } else {
    // unknown item class
    log_filter_append(out_writepos, out_left, "???");
    return;
  }

  if (len >= out_left)           /* buffer exhausted. '\0' terminate */
    out_buf[out_siz - 1] = '\0'; /* purecov: inspected */
}


// Source: log_filter_dragnet.cc
// Lines 372-426
