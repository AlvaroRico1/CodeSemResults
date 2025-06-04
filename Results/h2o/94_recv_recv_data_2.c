static inline yrmcds_error recv_data(yrmcds* c) {
    if( (c->capacity - c->used) < RECV_SIZE ) {
        size_t new_capacity = c->capacity * 2;
        char* new_buffer = (char*)realloc(c->recvbuf, new_capacity);
        if( new_buffer == NULL )
            return YRMCDS_OUT_OF_MEMORY;
        c->recvbuf = new_buffer;
        c->capacity = new_capacity;
    }


// Source: recv.c
// Lines 22-30
