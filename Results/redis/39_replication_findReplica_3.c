static client *findReplica(char *host, int port) {
    listIter li;
    listNode *ln;
    client *replica;

    listRewind(server.slaves,&li);
    while((ln = listNext(&li))) {
        replica = ln->value;
        char ip[NET_IP_STR_LEN], *replicaip = replica->slave_addr;

        if (!replicaip) {
            if (connPeerToString(replica->conn, ip, sizeof(ip), NULL) == -1)
                continue;
            replicaip = ip;
        }

        if (!strcasecmp(host, replicaip) &&
                (port == replica->slave_listening_port))
            return replica;
    }


// Source: replication.c
// Lines 3525-3544
