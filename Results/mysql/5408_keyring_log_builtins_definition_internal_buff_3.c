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


// Source: keyring_log_builtins_definition.cc
// Lines 257-277
