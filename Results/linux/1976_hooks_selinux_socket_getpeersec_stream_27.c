static int selinux_socket_getpeersec_stream(struct socket *sock, char __user *optval,
					    int __user *optlen, unsigned len)
{
	int err = 0;
	char *scontext;
	u32 scontext_len;
	struct sk_security_struct *sksec = sock->sk->sk_security;
	u32 peer_sid = SECSID_NULL;

	if (sksec->sclass == SECCLASS_UNIX_STREAM_SOCKET ||
	    sksec->sclass == SECCLASS_TCP_SOCKET ||
	    sksec->sclass == SECCLASS_SCTP_SOCKET)
		peer_sid = sksec->peer_sid;
	if (peer_sid == SECSID_NULL)
		return -ENOPROTOOPT;

	err = security_sid_to_context(&selinux_state, peer_sid, &scontext,
				      &scontext_len);
	if (err)
		return err;

	if (scontext_len > len) {
		err = -ERANGE;
		goto out_len;
	}

	if (copy_to_user(optval, scontext, scontext_len))
		err = -EFAULT;

out_len:
	if (put_user(scontext_len, optlen))
		err = -EFAULT;
	kfree(scontext);
	return err;
}


// Source: hooks.c
// Lines 5005-5039
