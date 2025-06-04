              char *nl2 = line_buffer;
              while ((nl2 = strchr(nl2, '\n')) != nullptr) *(nl2++) = ' ';
              msg = line_buffer;
            }


// Source: keyring_log_builtins_definition.cc
// Lines 244-247
