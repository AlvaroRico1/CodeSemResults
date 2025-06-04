static CURLcode ossl_connect_step1(struct Curl_easy *data,
                                   struct connectdata *conn, int sockindex)
{
  CURLcode result = CURLE_OK;
  char *ciphers;
  SSL_METHOD_QUAL SSL_METHOD *req_method = NULL;
  X509_LOOKUP *lookup = NULL;
  curl_socket_t sockfd = conn->sock[sockindex];
  struct ssl_connect_data *connssl = &conn->ssl[sockindex];
  ctx_option_t ctx_options = 0;
  void *ssl_sessionid = NULL;

#ifdef SSL_CTRL_SET_TLSEXT_HOSTNAME
  bool sni;
  const char * const hostname = SSL_HOST_NAME();

#ifdef ENABLE_IPV6
  struct in6_addr addr;
#else
  struct in_addr addr;
#endif
#endif
  const long int ssl_version = SSL_CONN_CONFIG(version);
#ifdef USE_OPENSSL_SRP
  const enum CURL_TLSAUTH ssl_authtype = SSL_SET_OPTION(authtype);
#endif
  char * const ssl_cert = SSL_SET_OPTION(primary.clientcert);
  const struct curl_blob *ssl_cert_blob = SSL_SET_OPTION(primary.cert_blob);
  const struct curl_blob *ca_info_blob = SSL_CONN_CONFIG(ca_info_blob);
  const char * const ssl_cert_type = SSL_SET_OPTION(cert_type);
  const char * const ssl_cafile =
    /* CURLOPT_CAINFO_BLOB overrides CURLOPT_CAINFO */
    (ca_info_blob ? NULL : SSL_CONN_CONFIG(CAfile));
  const char * const ssl_capath = SSL_CONN_CONFIG(CApath);
  const bool verifypeer = SSL_CONN_CONFIG(verifypeer);
  const char * const ssl_crlfile = SSL_SET_OPTION(CRLfile);
  char error_buffer[256];
  struct ssl_backend_data *backend = connssl->backend;
  bool imported_native_ca = false;

  DEBUGASSERT(ssl_connect_1 == connssl->connecting_state);

  /* Make funny stuff to get random input */
  result = ossl_seed(data);
  if(result)
    return result;

  SSL_SET_OPTION_LVALUE(certverifyresult) = !X509_V_OK;

  /* check to see if we've been told to use an explicit SSL/TLS version */

  switch(ssl_version) {
  case CURL_SSLVERSION_DEFAULT:
  case CURL_SSLVERSION_TLSv1:
  case CURL_SSLVERSION_TLSv1_0:
  case CURL_SSLVERSION_TLSv1_1:
  case CURL_SSLVERSION_TLSv1_2:
  case CURL_SSLVERSION_TLSv1_3:
    /* it will be handled later with the context options */
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
    req_method = TLS_client_method();
#else
    req_method = SSLv23_client_method();
#endif
    use_sni(TRUE);
    break;
  case CURL_SSLVERSION_SSLv2:
    failf(data, "No SSLv2 support");
    return CURLE_NOT_BUILT_IN;
  case CURL_SSLVERSION_SSLv3:
    failf(data, "No SSLv3 support");
    return CURLE_NOT_BUILT_IN;
  default:
    failf(data, "Unrecognized parameter passed via CURLOPT_SSLVERSION");
    return CURLE_SSL_CONNECT_ERROR;
  }

  DEBUGASSERT(!backend->ctx);
  backend->ctx = SSL_CTX_new(req_method);

  if(!backend->ctx) {
    failf(data, "SSL: couldn't create a context: %s",
          ossl_strerror(ERR_peek_error(), error_buffer, sizeof(error_buffer)));
    return CURLE_OUT_OF_MEMORY;
  }

#ifdef SSL_MODE_RELEASE_BUFFERS
  SSL_CTX_set_mode(backend->ctx, SSL_MODE_RELEASE_BUFFERS);
#endif

#ifdef SSL_CTRL_SET_MSG_CALLBACK
  if(data->set.fdebug && data->set.verbose) {
    /* the SSL trace callback is only used for verbose logging */
    SSL_CTX_set_msg_callback(backend->ctx, ossl_trace);
    SSL_CTX_set_msg_callback_arg(backend->ctx, conn);
    set_logger(conn, data);
  }
#endif

  /* OpenSSL contains code to work around lots of bugs and flaws in various
     SSL-implementations. SSL_CTX_set_options() is used to enabled those
     work-arounds. The man page for this option states that SSL_OP_ALL enables
     all the work-arounds and that "It is usually safe to use SSL_OP_ALL to
     enable the bug workaround options if compatibility with somewhat broken
     implementations is desired."

     The "-no_ticket" option was introduced in OpenSSL 0.9.8j. It's a flag to
     disable "rfc4507bis session ticket support".  rfc4507bis was later turned
     into the proper RFC5077 it seems: https://tools.ietf.org/html/rfc5077

     The enabled extension concerns the session management. I wonder how often
     libcurl stops a connection and then resumes a TLS session. Also, sending
     the session data is some overhead. I suggest that you just use your
     proposed patch (which explicitly disables TICKET).

     If someone writes an application with libcurl and OpenSSL who wants to
     enable the feature, one can do this in the SSL callback.

     SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG option enabling allowed proper
     interoperability with web server Netscape Enterprise Server 2.0.1 which
     was released back in 1996.

     Due to CVE-2010-4180, option SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG has
     become ineffective as of OpenSSL 0.9.8q and 1.0.0c. In order to mitigate
     CVE-2010-4180 when using previous OpenSSL versions we no longer enable
     this option regardless of OpenSSL version and SSL_OP_ALL definition.

     OpenSSL added a work-around for a SSL 3.0/TLS 1.0 CBC vulnerability
     (https://www.openssl.org/~bodo/tls-cbc.txt). In 0.9.6e they added a bit to
     SSL_OP_ALL that _disables_ that work-around despite the fact that
     SSL_OP_ALL is documented to do "rather harmless" workarounds. In order to
     keep the secure work-around, the SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS bit
     must not be set.
  */

  ctx_options = SSL_OP_ALL;

#ifdef SSL_OP_NO_TICKET
  ctx_options |= SSL_OP_NO_TICKET;
#endif

#ifdef SSL_OP_NO_COMPRESSION
  ctx_options |= SSL_OP_NO_COMPRESSION;
#endif

#ifdef SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG
  /* mitigate CVE-2010-4180 */
  ctx_options &= ~SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG;
#endif

#ifdef SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS
  /* unless the user explicitly asks to allow the protocol vulnerability we
     use the work-around */
  if(!SSL_SET_OPTION(enable_beast))
    ctx_options &= ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS;
#endif

  switch(ssl_version) {
    case CURL_SSLVERSION_SSLv2:
    case CURL_SSLVERSION_SSLv3:
      return CURLE_NOT_BUILT_IN;

    /* "--tlsv<x.y>" options mean TLS >= version <x.y> */
    case CURL_SSLVERSION_DEFAULT:
    case CURL_SSLVERSION_TLSv1: /* TLS >= version 1.0 */
    case CURL_SSLVERSION_TLSv1_0: /* TLS >= version 1.0 */
    case CURL_SSLVERSION_TLSv1_1: /* TLS >= version 1.1 */
    case CURL_SSLVERSION_TLSv1_2: /* TLS >= version 1.2 */
    case CURL_SSLVERSION_TLSv1_3: /* TLS >= version 1.3 */
      /* asking for any TLS version as the minimum, means no SSL versions
        allowed */
      ctx_options |= SSL_OP_NO_SSLv2;
      ctx_options |= SSL_OP_NO_SSLv3;

#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) /* 1.1.0 */
      result = set_ssl_version_min_max(backend->ctx, conn);
#else
      result = set_ssl_version_min_max_legacy(&ctx_options, data, conn,
                                              sockindex);
#endif
      if(result != CURLE_OK)
        return result;
      break;

    default:
      failf(data, "Unrecognized parameter passed via CURLOPT_SSLVERSION");
      return CURLE_SSL_CONNECT_ERROR;
  }

  SSL_CTX_set_options(backend->ctx, ctx_options);

#ifdef HAS_NPN
  if(conn->bits.tls_enable_npn)
    SSL_CTX_set_next_proto_select_cb(backend->ctx, select_next_proto_cb, data);
#endif

#ifdef HAS_ALPN
  if(conn->bits.tls_enable_alpn) {
    int cur = 0;
    unsigned char protocols[128];

#ifdef USE_HTTP2
    if(data->state.httpwant >= CURL_HTTP_VERSION_2
#ifndef CURL_DISABLE_PROXY
       && (!SSL_IS_PROXY() || !conn->bits.tunnel_proxy)
#endif
      ) {
      protocols[cur++] = ALPN_H2_LENGTH;

      memcpy(&protocols[cur], ALPN_H2, ALPN_H2_LENGTH);
      cur += ALPN_H2_LENGTH;
      infof(data, "ALPN, offering %s", ALPN_H2);
    }
#endif

    protocols[cur++] = ALPN_HTTP_1_1_LENGTH;
    memcpy(&protocols[cur], ALPN_HTTP_1_1, ALPN_HTTP_1_1_LENGTH);
    cur += ALPN_HTTP_1_1_LENGTH;
    infof(data, "ALPN, offering %s", ALPN_HTTP_1_1);

    /* expects length prefixed preference ordered list of protocols in wire
     * format
     */
    if(SSL_CTX_set_alpn_protos(backend->ctx, protocols, cur)) {
      failf(data, "Error setting ALPN");
      return CURLE_SSL_CONNECT_ERROR;
    }
  }
#endif

  if(ssl_cert || ssl_cert_blob || ssl_cert_type) {
    if(!result &&
       !cert_stuff(data, backend->ctx,
                   ssl_cert, ssl_cert_blob, ssl_cert_type,
                   SSL_SET_OPTION(key), SSL_SET_OPTION(key_blob),
                   SSL_SET_OPTION(key_type), SSL_SET_OPTION(key_passwd)))
      result = CURLE_SSL_CERTPROBLEM;
    if(result)
      /* failf() is already done in cert_stuff() */
      return result;
  }

  ciphers = SSL_CONN_CONFIG(cipher_list);
  if(!ciphers)
    ciphers = (char *)DEFAULT_CIPHER_SELECTION;
  if(ciphers) {
    if(!SSL_CTX_set_cipher_list(backend->ctx, ciphers)) {
      failf(data, "failed setting cipher list: %s", ciphers);
      return CURLE_SSL_CIPHER;
    }
    infof(data, "Cipher selection: %s", ciphers);
  }

#ifdef HAVE_SSL_CTX_SET_CIPHERSUITES
  {
    char *ciphers13 = SSL_CONN_CONFIG(cipher_list13);
    if(ciphers13) {
      if(!SSL_CTX_set_ciphersuites(backend->ctx, ciphers13)) {
        failf(data, "failed setting TLS 1.3 cipher suite: %s", ciphers13);
        return CURLE_SSL_CIPHER;
      }
      infof(data, "TLS 1.3 cipher selection: %s", ciphers13);
    }
  }
#endif

#ifdef HAVE_SSL_CTX_SET_POST_HANDSHAKE_AUTH
  /* OpenSSL 1.1.1 requires clients to opt-in for PHA */
  SSL_CTX_set_post_handshake_auth(backend->ctx, 1);
#endif

#ifdef HAVE_SSL_CTX_SET_EC_CURVES
  {
    char *curves = SSL_CONN_CONFIG(curves);
    if(curves) {
      if(!SSL_CTX_set1_curves_list(backend->ctx, curves)) {
        failf(data, "failed setting curves list: '%s'", curves);
        return CURLE_SSL_CIPHER;
      }
    }
  }
#endif

#ifdef USE_OPENSSL_SRP
  if(ssl_authtype == CURL_TLSAUTH_SRP) {
    char * const ssl_username = SSL_SET_OPTION(username);

    infof(data, "Using TLS-SRP username: %s", ssl_username);

    if(!SSL_CTX_set_srp_username(backend->ctx, ssl_username)) {
      failf(data, "Unable to set SRP user name");
      return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    if(!SSL_CTX_set_srp_password(backend->ctx, SSL_SET_OPTION(password))) {
      failf(data, "failed setting SRP password");
      return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    if(!SSL_CONN_CONFIG(cipher_list)) {
      infof(data, "Setting cipher list SRP");

      if(!SSL_CTX_set_cipher_list(backend->ctx, "SRP")) {
        failf(data, "failed setting SRP cipher list");
        return CURLE_SSL_CIPHER;
      }
    }
  }
#endif


#if defined(USE_WIN32_CRYPTO)
  /* Import certificates from the Windows root certificate store if requested.
     https://stackoverflow.com/questions/9507184/
     https://github.com/d3x0r/SACK/blob/master/src/netlib/ssl_layer.c#L1037
     https://tools.ietf.org/html/rfc5280 */
  if((SSL_CONN_CONFIG(verifypeer) || SSL_CONN_CONFIG(verifyhost)) &&
     (SSL_SET_OPTION(native_ca_store))) {
    X509_STORE *store = SSL_CTX_get_cert_store(backend->ctx);
    HCERTSTORE hStore = CertOpenSystemStore(0, TEXT("ROOT"));

    if(hStore) {
      PCCERT_CONTEXT pContext = NULL;
      /* The array of enhanced key usage OIDs will vary per certificate and is
         declared outside of the loop so that rather than malloc/free each
         iteration we can grow it with realloc, when necessary. */
      CERT_ENHKEY_USAGE *enhkey_usage = NULL;
      DWORD enhkey_usage_size = 0;

      /* This loop makes a best effort to import all valid certificates from
         the MS root store. If a certificate cannot be imported it is skipped.
         'result' is used to store only hard-fail conditions (such as out of
         memory) that cause an early break. */
      result = CURLE_OK;
      for(;;) {
        X509 *x509;
        FILETIME now;
        BYTE key_usage[2];
        DWORD req_size;
        const unsigned char *encoded_cert;
#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
        char cert_name[256];
#endif

        pContext = CertEnumCertificatesInStore(hStore, pContext);
        if(!pContext)
          break;

#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
        if(!CertGetNameStringA(pContext, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0,
                               NULL, cert_name, sizeof(cert_name))) {
          strcpy(cert_name, "Unknown");
        }
        infof(data, "SSL: Checking cert %s\"\n", cert_name);
#endif

        encoded_cert = (const unsigned char *)pContext->pbCertEncoded;
        if(!encoded_cert)
          continue;

        GetSystemTimeAsFileTime(&now);
        if(CompareFileTime(&pContext->pCertInfo->NotBefore, &now) > 0 ||
           CompareFileTime(&now, &pContext->pCertInfo->NotAfter) > 0)
          continue;

        /* If key usage exists check for signing attribute */
        if(CertGetIntendedKeyUsage(pContext->dwCertEncodingType,
                                   pContext->pCertInfo,
                                   key_usage, sizeof(key_usage))) {
          if(!(key_usage[0] & CERT_KEY_CERT_SIGN_KEY_USAGE))
            continue;
        }
        else if(GetLastError())
          continue;

        /* If enhanced key usage exists check for server auth attribute.
         *
         * Note "In a Microsoft environment, a certificate might also have EKU
         * extended properties that specify valid uses for the certificate."
         * The call below checks both, and behavior varies depending on what is
         * found. For more details see CertGetEnhancedKeyUsage doc.
         */
        if(CertGetEnhancedKeyUsage(pContext, 0, NULL, &req_size)) {
          if(req_size && req_size > enhkey_usage_size) {
            void *tmp = realloc(enhkey_usage, req_size);

            if(!tmp) {
              failf(data, "SSL: Out of memory allocating for OID list");
              result = CURLE_OUT_OF_MEMORY;
              break;
            }

            enhkey_usage = (CERT_ENHKEY_USAGE *)tmp;
            enhkey_usage_size = req_size;
          }

          if(CertGetEnhancedKeyUsage(pContext, 0, enhkey_usage, &req_size)) {
            if(!enhkey_usage->cUsageIdentifier) {
              /* "If GetLastError returns CRYPT_E_NOT_FOUND, the certificate is
                 good for all uses. If it returns zero, the certificate has no
                 valid uses." */
              if((HRESULT)GetLastError() != CRYPT_E_NOT_FOUND)
                continue;
            }
            else {
              DWORD i;
              bool found = false;

              for(i = 0; i < enhkey_usage->cUsageIdentifier; ++i) {
                if(!strcmp("1.3.6.1.5.5.7.3.1" /* OID server auth */,
                           enhkey_usage->rgpszUsageIdentifier[i])) {
                  found = true;
                  break;
                }
              }

              if(!found)
                continue;
            }
          }
          else
            continue;
        }
        else
          continue;

        x509 = d2i_X509(NULL, &encoded_cert, pContext->cbCertEncoded);
        if(!x509)
          continue;

        /* Try to import the certificate. This may fail for legitimate reasons
           such as duplicate certificate, which is allowed by MS but not
           OpenSSL. */
        if(X509_STORE_add_cert(store, x509) == 1) {
#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
          infof(data, "SSL: Imported cert \"%s\"", cert_name);
#endif
          imported_native_ca = true;
        }
        X509_free(x509);
      }

      free(enhkey_usage);
      CertFreeCertificateContext(pContext);
      CertCloseStore(hStore, 0);

      if(result)
        return result;
    }
    if(imported_native_ca)
      infof(data, "successfully imported Windows CA store");
    else
      infof(data, "error importing Windows CA store, continuing anyway");
  }
#endif

  if(ca_info_blob) {
    result = load_cacert_from_memory(backend->ctx, ca_info_blob);
    if(result) {
      if(result == CURLE_OUT_OF_MEMORY ||
         (verifypeer && !imported_native_ca)) {
        failf(data, "error importing CA certificate blob");
        return result;
      }
      /* Only warn if no certificate verification is required. */
      infof(data, "error importing CA certificate blob, continuing anyway");
    }
  }

#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
  /* OpenSSL 3.0.0 has deprecated SSL_CTX_load_verify_locations */
  {
    if(ssl_cafile) {
      if(!SSL_CTX_load_verify_file(backend->ctx, ssl_cafile)) {
        if(verifypeer && !imported_native_ca) {
          /* Fail if we insist on successfully verifying the server. */
          failf(data, "error setting certificate file: %s", ssl_cafile);
          return CURLE_SSL_CACERT_BADFILE;
        }
        /* Continue with warning if certificate verification isn't required. */
        infof(data, "error setting certificate file, continuing anyway");
      }
      infof(data, " CAfile: %s", ssl_cafile);
    }
    if(ssl_capath) {
      if(!SSL_CTX_load_verify_dir(backend->ctx, ssl_capath)) {
        if(verifypeer && !imported_native_ca) {
          /* Fail if we insist on successfully verifying the server. */
          failf(data, "error setting certificate path: %s", ssl_capath);
          return CURLE_SSL_CACERT_BADFILE;
        }
        /* Continue with warning if certificate verification isn't required. */
        infof(data, "error setting certificate path, continuing anyway");
      }
      infof(data, " CApath: %s", ssl_capath);
    }
  }
#else
  if(ssl_cafile || ssl_capath) {
    /* tell SSL where to find CA certificates that are used to verify
       the server's certificate. */
    if(!SSL_CTX_load_verify_locations(backend->ctx, ssl_cafile, ssl_capath)) {
      if(verifypeer && !imported_native_ca) {
        /* Fail if we insist on successfully verifying the server. */
        failf(data, "error setting certificate verify locations:"
              "  CAfile: %s CApath: %s",
              ssl_cafile ? ssl_cafile : "none",
              ssl_capath ? ssl_capath : "none");
        return CURLE_SSL_CACERT_BADFILE;
      }
      /* Just continue with a warning if no strict certificate verification
         is required. */
      infof(data, "error setting certificate verify locations,"
            " continuing anyway:");
    }
    else {
      /* Everything is fine. */
      infof(data, "successfully set certificate verify locations:");
    }
    infof(data, " CAfile: %s", ssl_cafile ? ssl_cafile : "none");
    infof(data, " CApath: %s", ssl_capath ? ssl_capath : "none");
  }
#endif

#ifdef CURL_CA_FALLBACK
  if(verifypeer &&
     !ca_info_blob && !ssl_cafile && !ssl_capath && !imported_native_ca) {
    /* verifying the peer without any CA certificates won't
       work so use openssl's built-in default as fallback */
    SSL_CTX_set_default_verify_paths(backend->ctx);
  }
#endif

  if(ssl_crlfile) {
    /* tell OpenSSL where to find CRL file that is used to check certificate
     * revocation */
    lookup = X509_STORE_add_lookup(SSL_CTX_get_cert_store(backend->ctx),
                                 X509_LOOKUP_file());
    if(!lookup ||
       (!X509_load_crl_file(lookup, ssl_crlfile, X509_FILETYPE_PEM)) ) {
      failf(data, "error loading CRL file: %s", ssl_crlfile);
      return CURLE_SSL_CRL_BADFILE;
    }
    /* Everything is fine. */
    infof(data, "successfully loaded CRL file:");
    X509_STORE_set_flags(SSL_CTX_get_cert_store(backend->ctx),
                         X509_V_FLAG_CRL_CHECK|X509_V_FLAG_CRL_CHECK_ALL);

    infof(data, "  CRLfile: %s", ssl_crlfile);
  }

  if(verifypeer) {
    /* Try building a chain using issuers in the trusted store first to avoid
       problems with server-sent legacy intermediates.  Newer versions of
       OpenSSL do alternate chain checking by default but we do not know how to
       determine that in a reliable manner.
       https://rt.openssl.org/Ticket/Display.html?id=3621&user=guest&pass=guest
    */
#if defined(X509_V_FLAG_TRUSTED_FIRST)
    X509_STORE_set_flags(SSL_CTX_get_cert_store(backend->ctx),
                         X509_V_FLAG_TRUSTED_FIRST);
#endif
#ifdef X509_V_FLAG_PARTIAL_CHAIN
    if(!SSL_SET_OPTION(no_partialchain) && !ssl_crlfile) {
      /* Have intermediate certificates in the trust store be treated as
         trust-anchors, in the same way as self-signed root CA certificates
         are. This allows users to verify servers using the intermediate cert
         only, instead of needing the whole chain.

         Due to OpenSSL bug https://github.com/openssl/openssl/issues/5081 we
         cannot do partial chains with a CRL check.
      */
      X509_STORE_set_flags(SSL_CTX_get_cert_store(backend->ctx),
                           X509_V_FLAG_PARTIAL_CHAIN);
    }
#endif
  }

  /* OpenSSL always tries to verify the peer, this only says whether it should
   * fail to connect if the verification fails, or if it should continue
   * anyway. In the latter case the result of the verification is checked with
   * SSL_get_verify_result() below. */
  SSL_CTX_set_verify(backend->ctx,
                     verifypeer ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, NULL);

  /* Enable logging of secrets to the file specified in env SSLKEYLOGFILE. */
#ifdef HAVE_KEYLOG_CALLBACK
  if(Curl_tls_keylog_enabled()) {
    SSL_CTX_set_keylog_callback(backend->ctx, ossl_keylog_callback);
  }
#endif

  /* Enable the session cache because it's a prerequisite for the "new session"
   * callback. Use the "external storage" mode to prevent OpenSSL from creating
   * an internal session cache.
   */
  SSL_CTX_set_session_cache_mode(backend->ctx,
      SSL_SESS_CACHE_CLIENT | SSL_SESS_CACHE_NO_INTERNAL);
  SSL_CTX_sess_set_new_cb(backend->ctx, ossl_new_session_cb);

  /* give application a chance to interfere with SSL set up. */
  if(data->set.ssl.fsslctx) {
    Curl_set_in_callback(data, true);
    result = (*data->set.ssl.fsslctx)(data, backend->ctx,
                                      data->set.ssl.fsslctxp);
    Curl_set_in_callback(data, false);
    if(result) {
      failf(data, "error signaled by ssl ctx callback");
      return result;
    }
  }

  /* Let's make an SSL structure */
  if(backend->handle)
    SSL_free(backend->handle);
  backend->handle = SSL_new(backend->ctx);
  if(!backend->handle) {
    failf(data, "SSL: couldn't create a context (handle)!");
    return CURLE_OUT_OF_MEMORY;
  }

#if (OPENSSL_VERSION_NUMBER >= 0x0090808fL) && !defined(OPENSSL_NO_TLSEXT) && \
    !defined(OPENSSL_NO_OCSP)
  if(SSL_CONN_CONFIG(verifystatus))
    SSL_set_tlsext_status_type(backend->handle, TLSEXT_STATUSTYPE_ocsp);
#endif

#if defined(OPENSSL_IS_BORINGSSL) && defined(ALLOW_RENEG)
  SSL_set_renegotiate_mode(backend->handle, ssl_renegotiate_freely);
#endif

  SSL_set_connect_state(backend->handle);

  backend->server_cert = 0x0;
#ifdef SSL_CTRL_SET_TLSEXT_HOSTNAME
  if((0 == Curl_inet_pton(AF_INET, hostname, &addr)) &&
#ifdef ENABLE_IPV6
     (0 == Curl_inet_pton(AF_INET6, hostname, &addr)) &&
#endif
     sni) {
    size_t nlen = strlen(hostname);
    if((long)nlen >= data->set.buffer_size)
      /* this is seriously messed up */
      return CURLE_SSL_CONNECT_ERROR;

    /* RFC 6066 section 3 says the SNI field is case insensitive, but browsers
       send the data lowercase and subsequently there are now numerous servers
       out there that don't work unless the name is lowercased */
    Curl_strntolower(data->state.buffer, hostname, nlen);
    data->state.buffer[nlen] = 0;
    if(!SSL_set_tlsext_host_name(backend->handle, data->state.buffer))
      infof(data, "WARNING: failed to configure server name indication (SNI) "
            "TLS extension");
  }
#endif

  ossl_associate_connection(data, conn, sockindex);

  Curl_ssl_sessionid_lock(data);
  if(!Curl_ssl_getsessionid(data, conn, SSL_IS_PROXY() ? TRUE : FALSE,
                            &ssl_sessionid, NULL, sockindex)) {
    /* we got a session id, use it! */
    if(!SSL_set_session(backend->handle, ssl_sessionid)) {
      Curl_ssl_sessionid_unlock(data);
      failf(data, "SSL: SSL_set_session failed: %s",
            ossl_strerror(ERR_get_error(), error_buffer,
                          sizeof(error_buffer)));
      return CURLE_SSL_CONNECT_ERROR;
    }
    /* Informational message */
    infof(data, "SSL re-using session ID");
  }
  Curl_ssl_sessionid_unlock(data);

#ifndef CURL_DISABLE_PROXY
  if(conn->proxy_ssl[sockindex].use) {
    BIO *const bio = BIO_new(BIO_f_ssl());
    SSL *handle = conn->proxy_ssl[sockindex].backend->handle;
    DEBUGASSERT(ssl_connection_complete == conn->proxy_ssl[sockindex].state);
    DEBUGASSERT(handle != NULL);
    DEBUGASSERT(bio != NULL);
    BIO_set_ssl(bio, handle, FALSE);
    SSL_set_bio(backend->handle, bio, bio);
  }
  else
#endif
    if(!SSL_set_fd(backend->handle, (int)sockfd)) {
    /* pass the raw socket into the SSL layers */
    failf(data, "SSL: SSL_set_fd failed: %s",
          ossl_strerror(ERR_get_error(), error_buffer, sizeof(error_buffer)));
    return CURLE_SSL_CONNECT_ERROR;
  }

  connssl->connecting_state = ssl_connect_2;

  return CURLE_OK;
}


// Source: openssl.c
// Lines 2588-3282
