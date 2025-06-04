static double getopt_double(const char *arg, const struct my_option *optp,
                            int *err) {
  double num;
  int error;
  const char *end = arg + 1000; /* Big enough as *arg is \0 terminated */
  num = my_strtod(arg, &end, &error);
  if (end[0] != 0 || error) {
    my_getopt_error_reporter(ERROR_LEVEL, EE_INVALID_DECIMAL_VALUE_FOR_OPTION,
                             optp->name);
    *err = EXIT_ARGUMENT_INVALID;
    return 0.0;
  }
  return getopt_double_limit_value(num, optp, nullptr);
}


// Source: my_getopt.cc
// Lines 1253-1266
