void lremCommand(client *c) {
    robj *subject, *obj;
    obj = c->argv[3];
    long toremove;
    long removed = 0;

    if (sdslen(obj->ptr) > LIST_MAX_ITEM_SIZE) {
        addReplyError(c, "Element too large");
        return;
    }


// Source: t_list.c
// Lines 692-701
