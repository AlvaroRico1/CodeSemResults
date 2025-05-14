// Source: curl/lib/netrc.c
// Lines 238-245
int Curl_parsenetrc(const char *host,
                    char **loginp,
                    char **passwordp,
                    bool *login_changed,
                    bool *password_changed,
                    char *netrcfile)
{
  int retcode = 1;
  char *filealloc = NULL;

  if(!netrcfile) {
    char *home = NULL;
    char *homea = curl_getenv("HOME"); /* portable environment reader */
    if(homea) {
      home = homea;
#if defined(HAVE_GETPWUID_R) && defined(HAVE_GETEUID)
    }
    else {
      struct passwd pw, *pw_res;
      char pwbuf[1024];
      if(!getpwuid_r(geteuid(), &pw, pwbuf, sizeof(pwbuf), &pw_res)
         && pw_res) {
        home = pw.pw_dir;
      }
#elif defined(HAVE_GETPWUID) && defined(HAVE_GETEUID)
    }
