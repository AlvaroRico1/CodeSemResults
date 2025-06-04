    const char *msg = "";
    size_t msg_len = 0;
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
          }
          break;
        }
        default:
          --out_fields;
      }
    }

    if (have_message) {
      char internal_buff[LOG_BUFF_MAX];
      size_t buff_size = sizeof(internal_buff);
      char *buff_line = internal_buff;

      const char format[] = "%Y-%m-%d %X";
      time_t t(time(nullptr));
      tm tm(*localtime(&t));
      std::locale loc(std::cout.getloc());
      std::ostringstream sout;
      const std::time_put<char> &tput =
          std::use_facet<std::time_put<char>>(loc);
      tput.put(sout.rdbuf(), sout, '\0', &tm, &format[0], &format[11]);
      std::string time_string = sout.str().c_str();

      (void)snprintf(buff_line, buff_size, "%s [%.*s] [MY-%06u] [Keyring] %.*s",
                     time_string.c_str(), (int)label_len, label, errcode,
                     (int)msg_len, msg);
      std::cout << buff_line << std::endl;
      if (line_buffer) delete[] line_buffer;
      return out_fields;
    }
    return 0;
  }


// Source: keyring_log_builtins_definition.cc
// Lines 211-279
