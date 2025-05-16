static CURLcode smtp_parse_url_path(struct Curl_easy *data)
{
  /* The SMTP struct is already initialised in smtp_connect() */
  struct connectdata *conn = data->conn;
  struct smtp_conn *smtpc = &conn->proto.smtpc;
  const char *path = &data->state.up.path[1]; /* skip leading path */
  char localhost[HOSTNAME_MAX + 1];

  /* Calculate the path if necessary */
  if(!*path) {
    if(!Curl_gethostname(localhost, sizeof(localhost)))
      path = localhost;
    else
      path = "localhost";
  }

  /* URL decode the path and use it as the domain in our EHLO */
  return Curl_urldecode(data, path, 0, &smtpc->domain, NULL,
                        REJECT_CTRL);
}


// Source: smtp.c
// Lines 1691-1710
