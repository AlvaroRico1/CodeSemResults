char *curl_version(void)
{
  static char out[300];
  char *outp;
  size_t outlen;
  const char *src[VERSION_PARTS];
#ifdef USE_SSL
  char ssl_version[200];
#endif
#ifdef HAVE_LIBZ
  char z_version[40];
#endif
#ifdef HAVE_BROTLI
  char br_version[40] = "brotli/";
#endif
#ifdef HAVE_ZSTD
  char zst_version[40] = "zstd/";
#endif
#ifdef USE_ARES
  char cares_version[40];
#endif
#if defined(USE_LIBIDN2)
  char idn_version[40];
#endif
#ifdef USE_LIBPSL
  char psl_version[40];
#endif
#if defined(HAVE_ICONV) && defined(CURL_DOES_CONVERSIONS)
  char iconv_version[40]="iconv";
#endif
#ifdef USE_SSH
  char ssh_version[40];
#endif
#ifdef USE_NGHTTP2
  char h2_version[40];
#endif
#ifdef ENABLE_QUIC
  char h3_version[40];
#endif
#ifdef USE_LIBRTMP
  char rtmp_version[40];
#endif
#ifdef USE_HYPER
  char hyper_buf[30];
#endif
#ifdef USE_GSASL
  char gsasl_buf[30];
#endif
#ifdef USE_OPENLDAP
  char ldap_buf[30];
#endif
  int i = 0;
  int j;

#ifdef DEBUGBUILD
  /* Override version string when environment variable CURL_VERSION is set */
  const char *debugversion = getenv("CURL_VERSION");
  if(debugversion) {
    strncpy(out, debugversion, sizeof(out)-1);
    out[sizeof(out)-1] = '\0';
    return out;
  }
#endif

  src[i++] = LIBCURL_NAME "/" LIBCURL_VERSION;
#ifdef USE_SSL
  Curl_ssl_version(ssl_version, sizeof(ssl_version));
  src[i++] = ssl_version;
#endif
#ifdef HAVE_LIBZ
  msnprintf(z_version, sizeof(z_version), "zlib/%s", zlibVersion());
  src[i++] = z_version;
#endif
#ifdef HAVE_BROTLI
  brotli_version(&br_version[7], sizeof(br_version) - 7);
  src[i++] = br_version;
#endif
#ifdef HAVE_ZSTD
  zstd_version(&zst_version[5], sizeof(zst_version) - 5);
  src[i++] = zst_version;
#endif
#ifdef USE_ARES
  msnprintf(cares_version, sizeof(cares_version),
            "c-ares/%s", ares_version(NULL));
  src[i++] = cares_version;
#endif
#ifdef USE_LIBIDN2
  msnprintf(idn_version, sizeof(idn_version),
            "libidn2/%s", idn2_check_version(NULL));
  src[i++] = idn_version;
#elif defined(USE_WIN32_IDN)
  src[i++] = (char *)"WinIDN";
#endif

#ifdef USE_LIBPSL
  msnprintf(psl_version, sizeof(psl_version), "libpsl/%s", psl_get_version());
  src[i++] = psl_version;
#endif
#if defined(HAVE_ICONV) && defined(CURL_DOES_CONVERSIONS)
#ifdef _LIBICONV_VERSION
  msnprintf(iconv_version, sizeof(iconv_version), "iconv/%d.%d",
            _LIBICONV_VERSION >> 8, _LIBICONV_VERSION & 255);
#else
  /* version unknown, let the default stand */
#endif /* _LIBICONV_VERSION */
  src[i++] = iconv_version;
#endif
#ifdef USE_SSH
  Curl_ssh_version(ssh_version, sizeof(ssh_version));
  src[i++] = ssh_version;
#endif
#ifdef USE_NGHTTP2
  Curl_http2_ver(h2_version, sizeof(h2_version));
  src[i++] = h2_version;
#endif
#ifdef ENABLE_QUIC
  Curl_quic_ver(h3_version, sizeof(h3_version));
  src[i++] = h3_version;
#endif
#ifdef USE_LIBRTMP
  {
    char suff[2];
    if(RTMP_LIB_VERSION & 0xff) {
      suff[0] = (RTMP_LIB_VERSION & 0xff) + 'a' - 1;
      suff[1] = '\0';
    }
    else
      suff[0] = '\0';

    msnprintf(rtmp_version, sizeof(rtmp_version), "librtmp/%d.%d%s",
              RTMP_LIB_VERSION >> 16, (RTMP_LIB_VERSION >> 8) & 0xff,
              suff);
    src[i++] = rtmp_version;
  }
#endif
#ifdef USE_HYPER
  msnprintf(hyper_buf, sizeof(hyper_buf), "Hyper/%s", hyper_version());
  src[i++] = hyper_buf;
#endif
#ifdef USE_GSASL
  msnprintf(gsasl_buf, sizeof(gsasl_buf), "libgsasl/%s",
            gsasl_check_version(NULL));
  src[i++] = gsasl_buf;
#endif
#ifdef USE_OPENLDAP
  {
    LDAPAPIInfo api;
    api.ldapai_info_version = LDAP_API_INFO_VERSION;

    if(ldap_get_option(NULL, LDAP_OPT_API_INFO, &api) == LDAP_OPT_SUCCESS) {
      unsigned int patch = api.ldapai_vendor_version % 100;
      unsigned int major = api.ldapai_vendor_version / 10000;
      unsigned int minor =
        ((api.ldapai_vendor_version - major * 10000) - patch) / 100;
      msnprintf(ldap_buf, sizeof(ldap_buf), "%s/%u.%u.%u",
                api.ldapai_vendor_name, major, minor, patch);
      src[i++] = ldap_buf;
      ldap_memfree(api.ldapai_vendor_name);
      ber_memvfree((void **)api.ldapai_extensions);
    }
  }
#endif

  DEBUGASSERT(i <= VERSION_PARTS);

  outp = &out[0];
  outlen = sizeof(out);
  for(j = 0; j < i; j++) {
    size_t n = strlen(src[j]);
    /* we need room for a space, the string and the final zero */
    if(outlen <= (n + 2))
      break;
    if(j) {
      /* prepend a space if not the first */
      *outp++ = ' ';
      outlen--;
    }
    memcpy(outp, src[j], n);
    outp += n;
    outlen -= n;
  }
  *outp = 0;

  return out;
}


// Source: version.c
// Lines 111-295
