void SubstituteAndAppend(std::string* output, const char* format,
                         const SubstituteArg& arg0, const SubstituteArg& arg1,
                         const SubstituteArg& arg2, const SubstituteArg& arg3,
                         const SubstituteArg& arg4, const SubstituteArg& arg5,
                         const SubstituteArg& arg6, const SubstituteArg& arg7,
                         const SubstituteArg& arg8, const SubstituteArg& arg9) {
  const SubstituteArg* const args_array[] = {
    &arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9, nullptr
  };

  // Determine total size needed.
  int size = 0;
  for (int i = 0; format[i] != '\0'; i++) {
    if (format[i] == '$') {
      if (ascii_isdigit(format[i+1])) {
        int index = format[i+1] - '0';
        if (args_array[index]->size() == -1) {
          GOOGLE_LOG(DFATAL)
            << "strings::Substitute format string invalid: asked for \"$"
            << index << "\", but only " << CountSubstituteArgs(args_array)
            << " args were given.  Full format string was: \""
            << CEscape(format) << "\".";
          return;
        }
        size += args_array[index]->size();
        ++i;  // Skip next char.
      } else if (format[i+1] == '$') {
        ++size;
        ++i;  // Skip next char.
      } else {
        GOOGLE_LOG(DFATAL)
          << "Invalid strings::Substitute() format string: \""
          << CEscape(format) << "\".";
        return;
      }
    } else {
      ++size;
    }
  }

  if (size == 0) return;

  // Build the string.
  int original_size = output->size();
  STLStringResizeUninitialized(output, original_size + size);
  char* target = string_as_array(output) + original_size;
  for (int i = 0; format[i] != '\0'; i++) {
    if (format[i] == '$') {
      if (ascii_isdigit(format[i+1])) {
        unsigned int index = format[i+1] - '0';
        assert(index < 10);
        const SubstituteArg* src = args_array[index];
        memcpy(target, src->data(), src->size());
        target += src->size();
        ++i;  // Skip next char.
      } else if (format[i+1] == '$') {
        *target++ = '$';
        ++i;  // Skip next char.
      }
    } else {
      *target++ = format[i];
    }
  }

  GOOGLE_DCHECK_EQ(target - output->data(), output->size());
}


// Source: substitute.cc
// Lines 67-132
