// Source: curl/lib/formdata.c
// Lines 207-653
CURLFORMcode FormAdd(struct curl_httppost **httppost,
                     struct curl_httppost **last_post,
                     va_list params)
{
  struct FormInfo *first_form, *current_form, *form = NULL;
  CURLFORMcode return_value = CURL_FORMADD_OK;
  const char *prevtype = NULL;
  struct curl_httppost *post = NULL;
  CURLformoption option;
  struct curl_forms *forms = NULL;
  char *array_value = NULL; /* value read from an array */

  /* This is a state variable, that if TRUE means that we're parsing an
     array that we got passed to us. If FALSE we're parsing the input
     va_list arguments. */
  bool array_state = FALSE;

  /*
   * We need to allocate the first struct to fill in.
   */
  first_form = calloc(1, sizeof(struct FormInfo));
  if(!first_form)
    return CURL_FORMADD_MEMORY;

  current_form = first_form;

  /*
   * Loop through all the options set. Break if we have an error to report.
   */
  while(return_value == CURL_FORMADD_OK) {

    /* first see if we have more parts of the array param */
    if(array_state && forms) {
      /* get the upcoming option from the given array */
      option = forms->option;
      array_value = (char *)forms->value;

      forms++; /* advance this to next entry */
      if(CURLFORM_END == option) {
        /* end of array state */
        array_state = FALSE;
        continue;
      }
    }
    else {
      /* This is not array-state, get next option */
      option = va_arg(params, CURLformoption);
      if(CURLFORM_END == option)
        break;
    }

    switch(option) {
    case CURLFORM_ARRAY:
      if(array_state)
        /* we don't support an array from within an array */
        return_value = CURL_FORMADD_ILLEGAL_ARRAY;
      else {
        forms = va_arg(params, struct curl_forms *);
        if(forms)
          array_state = TRUE;
        else
          return_value = CURL_FORMADD_NULL;
      }
      break;

      /*
       * Set the Name property.
       */
    case CURLFORM_PTRNAME:
#ifdef CURL_DOES_CONVERSIONS
      /* Treat CURLFORM_PTR like CURLFORM_COPYNAME so that libcurl will copy
       * the data in all cases so that we'll have safe memory for the eventual
       * conversion.
       */
#else
      current_form->flags |= HTTPPOST_PTRNAME; /* fall through */
#endif
      /* FALLTHROUGH */
    case CURLFORM_COPYNAME:
      if(current_form->name)
        return_value = CURL_FORMADD_OPTION_TWICE;
      else {
        char *name = array_state?
          array_value:va_arg(params, char *);
        if(name)
          current_form->name = name; /* store for the moment */
        else
          return_value = CURL_FORMADD_NULL;
      }
      break;
    case CURLFORM_NAMELENGTH:
      if(current_form->namelength)
        return_value = CURL_FORMADD_OPTION_TWICE;
      else
        current_form->namelength =
          array_state?(size_t)array_value:(size_t)va_arg(params, long);
      break;

      /*
       * Set the contents property.
       */
    case CURLFORM_PTRCONTENTS:
      current_form->flags |= HTTPPOST_PTRCONTENTS;
      /* FALLTHROUGH */
    case CURLFORM_COPYCONTENTS:
      if(current_form->value)
        return_value = CURL_FORMADD_OPTION_TWICE;
      else {
        char *value =
          array_state?array_value:va_arg(params, char *);
        if(value)
          current_form->value = value; /* store for the moment */
        else
          return_value = CURL_FORMADD_NULL;
      }
      break;
    case CURLFORM_CONTENTSLENGTH:
      current_form->contentslength =
        array_state?(size_t)array_value:(size_t)va_arg(params, long);
      break;

    case CURLFORM_CONTENTLEN:
      current_form->flags |= CURL_HTTPPOST_LARGE;
      current_form->contentslength =
        array_state?(curl_off_t)(size_t)array_value:va_arg(params, curl_off_t);
      break;

      /* Get contents from a given file name */
    case CURLFORM_FILECONTENT:
      if(current_form->flags & (HTTPPOST_PTRCONTENTS|HTTPPOST_READFILE))
        return_value = CURL_FORMADD_OPTION_TWICE;
      else {
        const char *filename = array_state?
          array_value:va_arg(params, char *);
        if(filename) {
          current_form->value = strdup(filename);
          if(!current_form->value)
            return_value = CURL_FORMADD_MEMORY;
          else {
            current_form->flags |= HTTPPOST_READFILE;
            current_form->value_alloc = TRUE;
          }
        }
        else
          return_value = CURL_FORMADD_NULL;
      }
      break;

      /* We upload a file */
    case CURLFORM_FILE:
      {
        const char *filename = array_state?array_value:
          va_arg(params, char *);

        if(current_form->value) {
          if(current_form->flags & HTTPPOST_FILENAME) {
            if(filename) {
              char *fname = strdup(filename);
              if(!fname)
                return_value = CURL_FORMADD_MEMORY;
              else {
                form = AddFormInfo(fname, NULL, current_form);
                if(!form) {
                  free(fname);
                  return_value = CURL_FORMADD_MEMORY;
                }
                else {
                  form->value_alloc = TRUE;
                  current_form = form;
                  form = NULL;
                }
              }
            }
            else
              return_value = CURL_FORMADD_NULL;
          }
          else
            return_value = CURL_FORMADD_OPTION_TWICE;
        }
        else {
          if(filename) {
            current_form->value = strdup(filename);
            if(!current_form->value)
              return_value = CURL_FORMADD_MEMORY;
            else {
              current_form->flags |= HTTPPOST_FILENAME;
              current_form->value_alloc = TRUE;
            }
          }
          else
            return_value = CURL_FORMADD_NULL;
        }
        break;
      }

    case CURLFORM_BUFFERPTR:
      current_form->flags |= HTTPPOST_PTRBUFFER|HTTPPOST_BUFFER;
      if(current_form->buffer)
        return_value = CURL_FORMADD_OPTION_TWICE;
      else {
        char *buffer =
          array_state?array_value:va_arg(params, char *);
        if(buffer) {
          current_form->buffer = buffer; /* store for the moment */
          current_form->value = buffer; /* make it non-NULL to be accepted
                                           as fine */
        }
        else
          return_value = CURL_FORMADD_NULL;
      }
      break;

    case CURLFORM_BUFFERLENGTH:
      if(current_form->bufferlength)
        return_value = CURL_FORMADD_OPTION_TWICE;
      else
        current_form->bufferlength =
          array_state?(size_t)array_value:(size_t)va_arg(params, long);
      break;

    case CURLFORM_STREAM:
      current_form->flags |= HTTPPOST_CALLBACK;
      if(current_form->userp)
        return_value = CURL_FORMADD_OPTION_TWICE;
      else {
        char *userp =
          array_state?array_value:va_arg(params, char *);
        if(userp) {
          current_form->userp = userp;
          current_form->value = userp; /* this isn't strictly true but we
                                          derive a value from this later on
                                          and we need this non-NULL to be
                                          accepted as a fine form part */
        }
        else
          return_value = CURL_FORMADD_NULL;
      }
      break;

    case CURLFORM_CONTENTTYPE:
      {
        const char *contenttype =
          array_state?array_value:va_arg(params, char *);
        if(current_form->contenttype) {
          if(current_form->flags & HTTPPOST_FILENAME) {
            if(contenttype) {
              char *type = strdup(contenttype);
              if(!type)
                return_value = CURL_FORMADD_MEMORY;
              else {
                form = AddFormInfo(NULL, type, current_form);
                if(!form) {
                  free(type);
                  return_value = CURL_FORMADD_MEMORY;
                }
                else {
                  form->contenttype_alloc = TRUE;
                  current_form = form;
                  form = NULL;
                }
              }
            }
            else
              return_value = CURL_FORMADD_NULL;
          }
          else
            return_value = CURL_FORMADD_OPTION_TWICE;
        }
        else {
          if(contenttype) {
            current_form->contenttype = strdup(contenttype);
            if(!current_form->contenttype)
              return_value = CURL_FORMADD_MEMORY;
            else
              current_form->contenttype_alloc = TRUE;
          }
          else
            return_value = CURL_FORMADD_NULL;
        }
        break;
      }
    case CURLFORM_CONTENTHEADER:
      {
        /* this "cast increases required alignment of target type" but
           we consider it OK anyway */
        struct curl_slist *list = array_state?
          (struct curl_slist *)(void *)array_value:
          va_arg(params, struct curl_slist *);

        if(current_form->contentheader)
          return_value = CURL_FORMADD_OPTION_TWICE;
        else
          current_form->contentheader = list;

        break;
      }
    case CURLFORM_FILENAME:
    case CURLFORM_BUFFER:
      {
        const char *filename = array_state?array_value:
          va_arg(params, char *);
        if(current_form->showfilename)
          return_value = CURL_FORMADD_OPTION_TWICE;
        else {
          current_form->showfilename = strdup(filename);
          if(!current_form->showfilename)
            return_value = CURL_FORMADD_MEMORY;
          else
            current_form->showfilename_alloc = TRUE;
        }
        break;
      }
    default:
      return_value = CURL_FORMADD_UNKNOWN_OPTION;
      break;
    }
  }

  if(CURL_FORMADD_OK != return_value) {
    /* On error, free allocated fields for all nodes of the FormInfo linked
       list without deallocating nodes. List nodes are deallocated later on */
    struct FormInfo *ptr;
    for(ptr = first_form; ptr != NULL; ptr = ptr->more) {
      if(ptr->name_alloc) {
        Curl_safefree(ptr->name);
        ptr->name_alloc = FALSE;
      }
      if(ptr->value_alloc) {
        Curl_safefree(ptr->value);
        ptr->value_alloc = FALSE;
      }
      if(ptr->contenttype_alloc) {
        Curl_safefree(ptr->contenttype);
        ptr->contenttype_alloc = FALSE;
      }
      if(ptr->showfilename_alloc) {
        Curl_safefree(ptr->showfilename);
        ptr->showfilename_alloc = FALSE;
      }
    }
  }

  if(CURL_FORMADD_OK == return_value) {
    /* go through the list, check for completeness and if everything is
     * alright add the HttpPost item otherwise set return_value accordingly */

    post = NULL;
    for(form = first_form;
        form != NULL;
        form = form->more) {
      if(((!form->name || !form->value) && !post) ||
         ( (form->contentslength) &&
           (form->flags & HTTPPOST_FILENAME) ) ||
         ( (form->flags & HTTPPOST_FILENAME) &&
           (form->flags & HTTPPOST_PTRCONTENTS) ) ||

         ( (!form->buffer) &&
           (form->flags & HTTPPOST_BUFFER) &&
           (form->flags & HTTPPOST_PTRBUFFER) ) ||

         ( (form->flags & HTTPPOST_READFILE) &&
           (form->flags & HTTPPOST_PTRCONTENTS) )
        ) {
        return_value = CURL_FORMADD_INCOMPLETE;
        break;
      }
      if(((form->flags & HTTPPOST_FILENAME) ||
          (form->flags & HTTPPOST_BUFFER)) &&
         !form->contenttype) {
        char *f = (form->flags & HTTPPOST_BUFFER)?
          form->showfilename : form->value;
        char const *type;
        type = Curl_mime_contenttype(f);
        if(!type)
          type = prevtype;
        if(!type)
          type = FILE_CONTENTTYPE_DEFAULT;

        /* our contenttype is missing */
        form->contenttype = strdup(type);
        if(!form->contenttype) {
          return_value = CURL_FORMADD_MEMORY;
          break;
        }
        form->contenttype_alloc = TRUE;
      }
      if(form->name && form->namelength) {
        /* Name should not contain nul bytes. */
        size_t i;
        for(i = 0; i < form->namelength; i++)
          if(!form->name[i]) {
            return_value = CURL_FORMADD_NULL;
            break;
          }
        if(return_value != CURL_FORMADD_OK)
          break;
      }
      if(!(form->flags & HTTPPOST_PTRNAME) &&
         (form == first_form) ) {
        /* Note that there's small risk that form->name is NULL here if the
           app passed in a bad combo, so we better check for that first. */
        if(form->name) {
          /* copy name (without strdup; possibly not null-terminated) */
          form->name = Curl_memdup(form->name, form->namelength?
                                   form->namelength:
                                   strlen(form->name) + 1);
        }
        if(!form->name) {
          return_value = CURL_FORMADD_MEMORY;
          break;
        }
        form->name_alloc = TRUE;
      }
      if(!(form->flags & (HTTPPOST_FILENAME | HTTPPOST_READFILE |
                          HTTPPOST_PTRCONTENTS | HTTPPOST_PTRBUFFER |
                          HTTPPOST_CALLBACK)) && form->value) {
        /* copy value (without strdup; possibly contains null characters) */
        size_t clen  = (size_t) form->contentslength;
        if(!clen)
          clen = strlen(form->value) + 1;

        form->value = Curl_memdup(form->value, clen);

        if(!form->value) {
          return_value = CURL_FORMADD_MEMORY;
          break;
        }
        form->value_alloc = TRUE;
      }
      post = AddHttpPost(form->name, form->namelength,
                         form->value, form->contentslength,
                         form->buffer, form->bufferlength,
                         form->contenttype, form->flags,
                         form->contentheader, form->showfilename,
                         form->userp,
                         post, httppost,
                         last_post);

      if(!post) {
        return_value = CURL_FORMADD_MEMORY;
        break;
      }

      if(form->contenttype)
        prevtype = form->contenttype;
    }
    if(CURL_FORMADD_OK != return_value) {
      /* On error, free allocated fields for nodes of the FormInfo linked
         list which are not already owned by the httppost linked list
         without deallocating nodes. List nodes are deallocated later on */
      struct FormInfo *ptr;
      for(ptr = form; ptr != NULL; ptr = ptr->more) {
        if(ptr->name_alloc) {
          Curl_safefree(ptr->name);
          ptr->name_alloc = FALSE;
        }
        if(ptr->value_alloc) {
          Curl_safefree(ptr->value);
          ptr->value_alloc = FALSE;
        }
        if(ptr->contenttype_alloc) {
          Curl_safefree(ptr->contenttype);
          ptr->contenttype_alloc = FALSE;
        }
        if(ptr->showfilename_alloc) {
          Curl_safefree(ptr->showfilename);
          ptr->showfilename_alloc = FALSE;
        }
      }
    }
