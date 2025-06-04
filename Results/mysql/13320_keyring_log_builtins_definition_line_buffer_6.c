    char *line_buffer = nullptr;
    bool have_message = false;
    for (int c = 0; c < ll->count; ++c) {
      item_type = ll->item[c].type;
      ++out_fields;
      switch (item_type) {
        case LOG_ITEM_LOG_PRIO:
          prio = (enum loglevel)ll->item[c].data.data_integer;
          label = log_label_from_prio(prio);
          label_len = strlen(label);
          break;
        case LOG_ITEM_SQL_ERRCODE:
          errcode = (unsigned int)ll->item[c].data.data_integer;
          break;
        case LOG_ITEM_LOG_MESSAGE: {
          have_message = true;
          const char *nl;
          msg = ll->item[c].data.data_string.str;
          msg_len = ll->item[c].data.data_string.length;
          if ((nl = (const char *)memchr(msg, '\n', msg_len)) != nullptr) {
            if (line_buffer != nullptr) delete[] line_buffer;
            line_buffer = new char[msg_len + 1]();
            if (line_buffer == nullptr) {
              msg =
                  "The submitted error message contains a newline, "
                  "and a buffer to sanitize it for the traditional "
                  "log could not be allocated.";
              msg_len = strlen(msg);
            } else {
              memcpy(line_buffer, msg, msg_len);
              line_buffer[msg_len] = '\0';
              char *nl2 = line_buffer;
              while ((nl2 = strchr(nl2, '\n')) != nullptr) *(nl2++) = ' ';
              msg = line_buffer;
            }


// Source: keyring_log_builtins_definition.cc
// Lines 213-247
