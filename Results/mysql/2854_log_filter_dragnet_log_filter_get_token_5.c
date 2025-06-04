static int log_filter_get_token(const char **inp_readpos, const char **token,
                                size_t *len, uint types) {
  log_filter_skip_white(inp_readpos);

  *token = *inp_readpos;
  *len = 0;

  // get (quoted) argument
  if ((types & LOG_FILTER_TOKEN_ARG) &&
      ((**inp_readpos == '\"') || (**inp_readpos == '\''))) {
    // Remember what quotation mark was used to start quotation
    const char *delim = *inp_readpos;

    for (++(*inp_readpos); (**inp_readpos != '\0') && (**inp_readpos != *delim);
         (*inp_readpos)++) {
      // skip escaped characters
      if ((**inp_readpos == '\\') && (*(*inp_readpos + 1) != '\0'))
        ++(*inp_readpos);
    }

    // If all went well, opening quotation mark == closing one
    if (**inp_readpos == *delim)
      ++(*inp_readpos);
    else {
      // on failure, rewind
      *inp_readpos = *token;
      return -1;
    }
  }


// Source: log_filter_dragnet.cc
// Lines 636-664
