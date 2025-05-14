// Source: curl/lib/formdata.c
// Lines 126-130
static struct FormInfo *AddFormInfo(char *value,
                                    char *contenttype,
                                    struct FormInfo *parent_form_info)
{
  struct FormInfo *form_info;
  form_info = calloc(1, sizeof(struct FormInfo));
  if(form_info) {
    if(value)
      form_info->value = value;
    if(contenttype)
      form_info->contenttype = contenttype;
    form_info->flags = HTTPPOST_FILENAME;
  }
  else
    return NULL;

  if(parent_form_info) {
    /* now, point our 'more' to the original 'more' */
    form_info->more = parent_form_info->more;

    /* then move the original 'more' to point to ourselves */
    parent_form_info->more = form_info;
  }

  return form_info;
}
