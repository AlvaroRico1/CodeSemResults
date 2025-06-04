void STDCALL mysql_debug(const char *debug MY_ATTRIBUTE((unused))) {
#ifndef NDEBUG
  char *env;
  if (debug) {
    DBUG_PUSH(debug);
  } else if ((env = getenv("MYSQL_DEBUG"))) {
    DBUG_PUSH(env);
#if !defined(_WINVER) && !defined(WINVER)
    puts("\n-------------------------------------------------------");
    puts("MYSQL_DEBUG found. libmysql started with the following:");
    puts(env);
    puts("-------------------------------------------------------\n");
#else
    {
      char buff[80];
      buff[sizeof(buff) - 1] = 0;
      strxnmov(buff, sizeof(buff) - 1, "libmysql: ", env, NullS);
      MessageBox((HWND)0, "Debugging variable MYSQL_DEBUG used", buff, MB_OK);
    }
#endif
  }
#endif
}


// Source: libmysql.cc
// Lines 238-260
