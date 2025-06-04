char *get_tty_password(const char *opt_message) {
#ifdef HAVE_GETPASS
  char *passbuff;
#else  /* ! HAVE_GETPASS */
  TERMIO org, tmp;
#endif /* HAVE_GETPASS */
  char buff[80];

  DBUG_TRACE;

#ifdef HAVE_GETPASS
  passbuff = getpass(opt_message ? opt_message : "Enter password: ");

  /* copy the password to buff and clear original (static) buffer */
  strncpy(buff, passbuff, sizeof(buff) - 1);
  buff[sizeof(buff) - 1] = 0;
#ifdef _PASSWORD_LEN
  memset(passbuff, 0, _PASSWORD_LEN);
#endif
#else
  if (isatty(fileno(stdout))) {
    fputs(opt_message ? opt_message : "Enter password: ", stdout);
    fflush(stdout);
  }
#if defined(HAVE_TERMIOS_H)
  tcgetattr(fileno(stdin), &org);
  tmp = org;
  tmp.c_lflag &= ~(ECHO | ISIG | ICANON);
  tmp.c_cc[VMIN] = 1;
  tmp.c_cc[VTIME] = 0;
  tcsetattr(fileno(stdin), TCSADRAIN, &tmp);
  get_password(buff, sizeof(buff) - 1, fileno(stdin), isatty(fileno(stdout)));
  tcsetattr(fileno(stdin), TCSADRAIN, &org);
#elif defined(HAVE_TERMIO_H)
  ioctl(fileno(stdin), (int)TCGETA, &org);
  tmp = org;
  tmp.c_lflag &= ~(ECHO | ISIG | ICANON);
  tmp.c_cc[VMIN] = 1;
  tmp.c_cc[VTIME] = 0;
  ioctl(fileno(stdin), (int)TCSETA, &tmp);
  get_password(buff, sizeof(buff) - 1, fileno(stdin), isatty(fileno(stdout)));
  ioctl(fileno(stdin), (int)TCSETA, &org);
#else
  gtty(fileno(stdin), &org);
  tmp = org;
  tmp.sg_flags &= ~ECHO;
  tmp.sg_flags |= RAW;
  stty(fileno(stdin), &tmp);
  get_password(buff, sizeof(buff) - 1, fileno(stdin), isatty(fileno(stdout)));
  stty(fileno(stdin), &org);
#endif
  if (isatty(fileno(stdout))) fputc('\n', stdout);
#endif /* HAVE_GETPASS */

  /*
    If the password is 79 bytes or longer, terminate the password by
    setting the last but one character to the null character.
  */
  buff[sizeof(buff) - 1] = '\0';
  return my_strdup(PSI_NOT_INSTRUMENTED, buff, MYF(MY_FAE));
}


// Source: get_password.cc
// Lines 145-205
