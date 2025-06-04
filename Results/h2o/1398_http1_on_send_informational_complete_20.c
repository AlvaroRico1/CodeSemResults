static void on_send_informational_complete(h2o_socket_t *sock, const char *err)
{
    struct st_h2o_http1_conn_t *conn = sock->data;
    if (err != NULL) {
        close_connection(conn, 1);
        return;
    }

    conn->_ostr_final.informational.write_inflight = 0;

    if (conn->_ostr_final.informational.pending_final.inbufs != NULL) {
        finalostream_send(&conn->_ostr_final.super, &conn->req, conn->_ostr_final.informational.pending_final.inbufs,
                          conn->_ostr_final.informational.pending_final.inbufcnt,
                          conn->_ostr_final.informational.pending_final.send_state);
        return;
    }

    if (conn->_ostr_final.informational.pending.size != 0)
        do_send_informational(conn);
}


// Source: http1.c
// Lines 1110-1129
