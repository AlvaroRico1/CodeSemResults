// Source: curl/lib/smtp.c
// Lines 251-254
static void smtp_get_message(char *buffer, char **outptr)
{
  size_t len = strlen(buffer);
  char *message = NULL;

  if(len > 4) {
    /* Find the start of the message */
    len -= 4;
    for(message = buffer + 4; *message == ' ' || *message == '\t';
        message++, len--)
      ;

    /* Find the end of the message */
    for(; len--;)
      if(message[len] != '\r' && message[len] != '\n' && message[len] != ' ' &&
         message[len] != '\t')
        break;

    /* Terminate the message */
    if(++len) {
      message[len] = '\0';
    }
  }
  else
    /* junk input => zero length output */
    message = &buffer[len];

  *outptr = message;
}
