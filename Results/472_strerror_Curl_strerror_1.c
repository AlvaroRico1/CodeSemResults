// Source: curl/lib/strerror.c
// Lines 715-721
const char *Curl_strerror(int err, char *buf, size_t buflen)
{
#ifdef PRESERVE_WINDOWS_ERROR_CODE
  DWORD old_win_err = GetLastError();
#endif
  int old_errno = errno;
  char *p;
  size_t max;

  if(!buflen)
    return NULL;

#ifndef WIN32
  DEBUGASSERT(err >= 0);
#endif

  max = buflen - 1;
  *buf = '\0';

  /* !checksrc! disable STRERROR 2 */
#if defined(WIN32) || defined(_WIN32_WCE)
#if defined(WIN32)
  /* 'sys_nerr' is the maximum errno number, it is not widely portable */
  if(err >= 0 && err < sys_nerr)
    strncpy(buf, strerror(err), max);
  else
#endif
  {
    if(
#ifdef USE_WINSOCK
       !get_winsock_error(err, buf, max) &&
#endif
       !get_winapi_error((DWORD)err, buf, max))
      msnprintf(buf, max, "Unknown error %d (%#x)", err, err);
  }
#else /* not Windows coming up */

#if defined(HAVE_STRERROR_R) && defined(HAVE_POSIX_STRERROR_R)
 /*
  * The POSIX-style strerror_r() may set errno to ERANGE if insufficient
  * storage is supplied via 'strerrbuf' and 'buflen' to hold the generated
  * message string, or EINVAL if 'errnum' is not a valid error number.
  */
  if(0 != strerror_r(err, buf, max)) {
    if('\0' == buf[0])
      msnprintf(buf, max, "Unknown error %d", err);
  }
#elif defined(HAVE_STRERROR_R) && defined(HAVE_GLIBC_STRERROR_R)
 /*
  * The glibc-style strerror_r() only *might* use the buffer we pass to
  * the function, but it always returns the error message as a pointer,
  * so we must copy that string unconditionally (if non-NULL).
  */
  {
    char buffer[256];
    char *msg = strerror_r(err, buffer, sizeof(buffer));
    if(msg)
      strncpy(buf, msg, max);
    else
      msnprintf(buf, max, "Unknown error %d", err);
  }
#elif defined(HAVE_STRERROR_R) && defined(HAVE_VXWORKS_STRERROR_R)
 /*
  * The vxworks-style strerror_r() does use the buffer we pass to the function.
  * The buffer size should be at least NAME_MAX (256)
  */
  {
    char buffer[256];
    if(OK == strerror_r(err, buffer))
      strncpy(buf, buffer, max);
    else
      msnprintf(buf, max, "Unknown error %d", err);
  }
#else
  {
    const char *msg = strerror(err);
    if(msg)
      strncpy(buf, msg, max);
    else
      msnprintf(buf, max, "Unknown error %d", err);
  }
#endif

#endif /* end of not Windows */

  buf[max] = '\0'; /* make sure the string is null-terminated */

  /* strip trailing '\r\n' or '\n'. */
  p = strrchr(buf, '\n');
  if(p && (p - buf) >= 2)
    *p = '\0';
  p = strrchr(buf, '\r');
  if(p && (p - buf) >= 1)
    *p = '\0';

  if(errno != old_errno)
    errno = old_errno;

#ifdef PRESERVE_WINDOWS_ERROR_CODE
  if(old_win_err != GetLastError())
    SetLastError(old_win_err);
#endif

  return buf;
}
