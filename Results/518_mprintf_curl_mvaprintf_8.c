// Source: curl/lib/mprintf.c
// Lines 1076-1077
char *curl_mvaprintf(const char *format, va_list ap_save)
{
  int retcode;
  struct asprintf info;
  struct dynbuf dyn;
  info.b = &dyn;
  Curl_dyn_init(info.b, DYN_APRINTF);
  info.fail = 0;

  retcode = dprintf_formatf(&info, alloc_addbyter, format, ap_save);
  if((-1 == retcode) || info.fail) {
    Curl_dyn_free(info.b);
    return NULL;
  }
  if(Curl_dyn_len(info.b))
    return Curl_dyn_ptr(info.b);
  return strdup("");
}
