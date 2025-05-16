static void connect_done(struct Curl_easy *data)
{
  struct connectdata *conn = data->conn;
  struct http_connect_state *s = conn->connect_state;
  if(s->tunnel_state != TUNNEL_EXIT) {
    s->tunnel_state = TUNNEL_EXIT;
    Curl_dyn_free(&s->rcvbuf);
    Curl_dyn_free(&s->req);

    /* retore the protocol pointer */
    data->req.p.http = s->prot_save;
    s->prot_save = NULL;
    infof(data, "CONNECT phase completed!");
  }
}


// Source: http_proxy.c
// Lines 201-215
