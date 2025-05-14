// Source: curl/lib/mprintf.c
// Lines 1058-1061
int Curl_dyn_vprintf(struct dynbuf *dyn, const char *format, va_list ap_save)
{
  int retcode;
  struct asprintf info;
  info.b = dyn;
  info.fail = 0;

  retcode = dprintf_formatf(&info, alloc_addbyter, format, ap_save);
  if((-1 == retcode) || info.fail) {
    Curl_dyn_free(info.b);
    return 1;
  }
  return 0;
}
