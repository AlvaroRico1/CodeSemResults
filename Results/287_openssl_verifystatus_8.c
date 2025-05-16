static CURLcode verifystatus(struct Curl_easy *data,
                             struct ssl_connect_data *connssl)
{
  int i, ocsp_status;
  unsigned char *status;
  const unsigned char *p;
  CURLcode result = CURLE_OK;
  OCSP_RESPONSE *rsp = NULL;
  OCSP_BASICRESP *br = NULL;
  X509_STORE     *st = NULL;
  STACK_OF(X509) *ch = NULL;
  struct ssl_backend_data *backend = connssl->backend;
  X509 *cert;
  OCSP_CERTID *id = NULL;
  int cert_status, crl_reason;
  ASN1_GENERALIZEDTIME *rev, *thisupd, *nextupd;
  int ret;

  long len = SSL_get_tlsext_status_ocsp_resp(backend->handle, &status);

  if(!status) {
    failf(data, "No OCSP response received");
    result = CURLE_SSL_INVALIDCERTSTATUS;
    goto end;
  }
  p = status;
  rsp = d2i_OCSP_RESPONSE(NULL, &p, len);
  if(!rsp) {
    failf(data, "Invalid OCSP response");
    result = CURLE_SSL_INVALIDCERTSTATUS;
    goto end;
  }

  ocsp_status = OCSP_response_status(rsp);
  if(ocsp_status != OCSP_RESPONSE_STATUS_SUCCESSFUL) {
    failf(data, "Invalid OCSP response status: %s (%d)",
          OCSP_response_status_str(ocsp_status), ocsp_status);
    result = CURLE_SSL_INVALIDCERTSTATUS;
    goto end;
  }

  br = OCSP_response_get1_basic(rsp);
  if(!br) {
    failf(data, "Invalid OCSP response");
    result = CURLE_SSL_INVALIDCERTSTATUS;
    goto end;
  }

  ch = SSL_get_peer_cert_chain(backend->handle);
  st = SSL_CTX_get_cert_store(backend->ctx);

#if ((OPENSSL_VERSION_NUMBER <= 0x1000201fL) /* Fixed after 1.0.2a */ || \
     (defined(LIBRESSL_VERSION_NUMBER) &&                               \
      LIBRESSL_VERSION_NUMBER <= 0x2040200fL))
  /* The authorized responder cert in the OCSP response MUST be signed by the
     peer cert's issuer (see RFC6960 section 4.2.2.2). If that's a root cert,
     no problem, but if it's an intermediate cert OpenSSL has a bug where it
     expects this issuer to be present in the chain embedded in the OCSP
     response. So we add it if necessary. */

  /* First make sure the peer cert chain includes both a peer and an issuer,
     and the OCSP response contains a responder cert. */
  if(sk_X509_num(ch) >= 2 && sk_X509_num(br->certs) >= 1) {
    X509 *responder = sk_X509_value(br->certs, sk_X509_num(br->certs) - 1);

    /* Find issuer of responder cert and add it to the OCSP response chain */
    for(i = 0; i < sk_X509_num(ch); i++) {
      X509 *issuer = sk_X509_value(ch, i);
      if(X509_check_issued(issuer, responder) == X509_V_OK) {
        if(!OCSP_basic_add1_cert(br, issuer)) {
          failf(data, "Could not add issuer cert to OCSP response");
          result = CURLE_SSL_INVALIDCERTSTATUS;
          goto end;
        }
      }
    }
  }
#endif

  if(OCSP_basic_verify(br, ch, st, 0) <= 0) {
    failf(data, "OCSP response verification failed");
    result = CURLE_SSL_INVALIDCERTSTATUS;
    goto end;
  }

  /* Compute the certificate's ID */
  cert = SSL_get_peer_certificate(backend->handle);
  if(!cert) {
    failf(data, "Error getting peer certificate");
    result = CURLE_SSL_INVALIDCERTSTATUS;
    goto end;
  }

  for(i = 0; i < sk_X509_num(ch); i++) {
    X509 *issuer = sk_X509_value(ch, i);
    if(X509_check_issued(issuer, cert) == X509_V_OK) {
      id = OCSP_cert_to_id(EVP_sha1(), cert, issuer);
      break;
    }
  }
  X509_free(cert);

  if(!id) {
    failf(data, "Error computing OCSP ID");
    result = CURLE_SSL_INVALIDCERTSTATUS;
    goto end;
  }

  /* Find the single OCSP response corresponding to the certificate ID */
  ret = OCSP_resp_find_status(br, id, &cert_status, &crl_reason, &rev,
                              &thisupd, &nextupd);
  OCSP_CERTID_free(id);
  if(ret != 1) {
    failf(data, "Could not find certificate ID in OCSP response");
    result = CURLE_SSL_INVALIDCERTSTATUS;
    goto end;
  }

  /* Validate the corresponding single OCSP response */
  if(!OCSP_check_validity(thisupd, nextupd, 300L, -1L)) {
    failf(data, "OCSP response has expired");
    result = CURLE_SSL_INVALIDCERTSTATUS;
    goto end;
  }

  infof(data, "SSL certificate status: %s (%d)",
        OCSP_cert_status_str(cert_status), cert_status);

  switch(cert_status) {
  case V_OCSP_CERTSTATUS_GOOD:
    break;

  case V_OCSP_CERTSTATUS_REVOKED:
    result = CURLE_SSL_INVALIDCERTSTATUS;
    failf(data, "SSL certificate revocation reason: %s (%d)",
          OCSP_crl_reason_str(crl_reason), crl_reason);
    goto end;

  case V_OCSP_CERTSTATUS_UNKNOWN:
  default:
    result = CURLE_SSL_INVALIDCERTSTATUS;
    goto end;
  }

end:
  if(br)
    OCSP_BASICRESP_free(br);
  OCSP_RESPONSE_free(rsp);

  return result;
}


// Source: openssl.c
// Lines 1843-1993
