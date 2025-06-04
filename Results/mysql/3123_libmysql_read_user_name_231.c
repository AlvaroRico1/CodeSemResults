void read_user_name(char *name) {
  DBUG_TRACE;
  if (geteuid() == 0)
    (void)my_stpcpy(name, "root"); /* allow use of surun */
  else {
#ifdef HAVE_GETPWUID
    struct passwd *skr;
    const char *str;
    if ((str = getlogin()) == nullptr) {
      if ((skr = getpwuid(geteuid())) != nullptr)
        str = skr->pw_name;
      else if (!(str = getenv("USER")) && !(str = getenv("LOGNAME")) &&
               !(str = getenv("LOGIN")))
        str = "UNKNOWN_USER";
    }
    (void)strmake(name, str, USERNAME_LENGTH);
#elif HAVE_CUSERID
    (void)cuserid(name);
#else
    my_stpcpy(name, "UNKNOWN_USER");
#endif
  }


// Source: libmysql.cc
// Lines 333-354
