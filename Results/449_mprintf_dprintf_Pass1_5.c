// Source: curl/lib/mprintf.c
// Lines 229-231
static int dprintf_Pass1(const char *format, struct va_stack *vto,
                         char **endpos, va_list arglist)
{
  char *fmt = (char *)format;
  int param_num = 0;
  long this_param;
  long width;
  long precision;
  int flags;
  long max_param = 0;
  long i;

  while(*fmt) {
    if(*fmt++ == '%') {
      if(*fmt == '%') {
        fmt++;
        continue; /* while */
      }

      flags = FLAGS_NEW;

      /* Handle the positional case (N$) */

      param_num++;

      this_param = dprintf_DollarString(fmt, &fmt);
      if(0 == this_param)
        /* we got no positional, get the next counter */
        this_param = param_num;

      if(this_param > max_param)
        max_param = this_param;

      /*
       * The parameter with number 'i' should be used. Next, we need
       * to get SIZE and TYPE of the parameter. Add the information
       * to our array.
       */

      width = 0;
      precision = 0;

      /* Handle the flags */

      while(dprintf_IsQualifierNoDollar(fmt)) {
#if defined(MP_HAVE_INT_EXTENSIONS)
        if(!strncmp(fmt, "I32", 3)) {
          flags |= FLAGS_LONG;
          fmt += 3;
        }
        else if(!strncmp(fmt, "I64", 3)) {
          flags |= FLAGS_LONGLONG;
          fmt += 3;
        }
        else
#endif

        switch(*fmt++) {
        case ' ':
          flags |= FLAGS_SPACE;
          break;
        case '+':
          flags |= FLAGS_SHOWSIGN;
          break;
        case '-':
          flags |= FLAGS_LEFT;
          flags &= ~FLAGS_PAD_NIL;
          break;
        case '#':
          flags |= FLAGS_ALT;
          break;
        case '.':
          if('*' == *fmt) {
            /* The precision is picked from a specified parameter */

            flags |= FLAGS_PRECPARAM;
            fmt++;
            param_num++;

            i = dprintf_DollarString(fmt, &fmt);
            if(i)
              precision = i;
            else
              precision = param_num;

            if(precision > max_param)
              max_param = precision;
          }
          else {
            flags |= FLAGS_PREC;
            precision = strtol(fmt, &fmt, 10);
          }
          break;
        case 'h':
          flags |= FLAGS_SHORT;
          break;
#if defined(MP_HAVE_INT_EXTENSIONS)
        case 'I':
#if (SIZEOF_CURL_OFF_T > SIZEOF_LONG)
          flags |= FLAGS_LONGLONG;
#else
          flags |= FLAGS_LONG;
#endif
          break;
#endif
        case 'l':
          if(flags & FLAGS_LONG)
            flags |= FLAGS_LONGLONG;
          else
            flags |= FLAGS_LONG;
          break;
        case 'L':
          flags |= FLAGS_LONGDOUBLE;
          break;
        case 'q':
          flags |= FLAGS_LONGLONG;
          break;
        case 'z':
          /* the code below generates a warning if -Wunreachable-code is
             used */
#if (SIZEOF_SIZE_T > SIZEOF_LONG)
          flags |= FLAGS_LONGLONG;
#else
          flags |= FLAGS_LONG;
#endif
          break;
        case 'O':
#if (SIZEOF_CURL_OFF_T > SIZEOF_LONG)
          flags |= FLAGS_LONGLONG;
#else
          flags |= FLAGS_LONG;
#endif
          break;
        case '0':
          if(!(flags & FLAGS_LEFT))
            flags |= FLAGS_PAD_NIL;
          /* FALLTHROUGH */
        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          flags |= FLAGS_WIDTH;
          width = strtol(fmt-1, &fmt, 10);
          break;
        case '*':  /* Special case */
          flags |= FLAGS_WIDTHPARAM;
          param_num++;

          i = dprintf_DollarString(fmt, &fmt);
          if(i)
            width = i;
          else
            width = param_num;
          if(width > max_param)
            max_param = width;
          break;
        case '\0':
          fmt--;
        default:
          break;
        }
      } /* switch */

      /* Handle the specifier */

      i = this_param - 1;

      if((i < 0) || (i >= MAX_PARAMETERS))
        /* out of allowed range */
        return 1;

      switch (*fmt) {
      case 'S':
        flags |= FLAGS_ALT;
        /* FALLTHROUGH */
      case 's':
        vto[i].type = FORMAT_STRING;
        break;
      case 'n':
        vto[i].type = FORMAT_INTPTR;
        break;
      case 'p':
        vto[i].type = FORMAT_PTR;
        break;
      case 'd': case 'i':
        vto[i].type = FORMAT_INT;
        break;
      case 'u':
        vto[i].type = FORMAT_INT;
        flags |= FLAGS_UNSIGNED;
        break;
      case 'o':
        vto[i].type = FORMAT_INT;
        flags |= FLAGS_OCTAL;
        break;
      case 'x':
        vto[i].type = FORMAT_INT;
        flags |= FLAGS_HEX|FLAGS_UNSIGNED;
        break;
      case 'X':
        vto[i].type = FORMAT_INT;
        flags |= FLAGS_HEX|FLAGS_UPPER|FLAGS_UNSIGNED;
        break;
      case 'c':
        vto[i].type = FORMAT_INT;
        flags |= FLAGS_CHAR;
        break;
      case 'f':
        vto[i].type = FORMAT_DOUBLE;
        break;
      case 'e':
        vto[i].type = FORMAT_DOUBLE;
        flags |= FLAGS_FLOATE;
        break;
      case 'E':
        vto[i].type = FORMAT_DOUBLE;
        flags |= FLAGS_FLOATE|FLAGS_UPPER;
        break;
      case 'g':
        vto[i].type = FORMAT_DOUBLE;
        flags |= FLAGS_FLOATG;
        break;
      case 'G':
        vto[i].type = FORMAT_DOUBLE;
        flags |= FLAGS_FLOATG|FLAGS_UPPER;
        break;
      default:
        vto[i].type = FORMAT_UNKNOWN;
        break;
      } /* switch */

      vto[i].flags = flags;
      vto[i].width = width;
      vto[i].precision = precision;

      if(flags & FLAGS_WIDTHPARAM) {
        /* we have the width specified from a parameter, so we make that
           parameter's info setup properly */
        long k = width - 1;
        if((k < 0) || (k >= MAX_PARAMETERS))
          /* out of allowed range */
          return 1;
        vto[i].width = k;
        vto[k].type = FORMAT_WIDTH;
        vto[k].flags = FLAGS_NEW;
        /* can't use width or precision of width! */
        vto[k].width = 0;
        vto[k].precision = 0;
      }
      if(flags & FLAGS_PRECPARAM) {
        /* we have the precision specified from a parameter, so we make that
           parameter's info setup properly */
        long k = precision - 1;
        if((k < 0) || (k >= MAX_PARAMETERS))
          /* out of allowed range */
          return 1;
        vto[i].precision = k;
        vto[k].type = FORMAT_WIDTH;
        vto[k].flags = FLAGS_NEW;
        /* can't use width or precision of width! */
        vto[k].width = 0;
        vto[k].precision = 0;
      }
      *endpos++ = fmt + ((*fmt == '\0') ? 0 : 1); /* end of this sequence */
    }
  }

  /* Read the arg list parameters into our data list */
  for(i = 0; i<max_param; i++) {
    /* Width/precision arguments must be read before the main argument
       they are attached to */
    if(vto[i].flags & FLAGS_WIDTHPARAM) {
      vto[vto[i].width].data.num.as_signed =
        (mp_intmax_t)va_arg(arglist, int);
    }
    if(vto[i].flags & FLAGS_PRECPARAM) {
      vto[vto[i].precision].data.num.as_signed =
        (mp_intmax_t)va_arg(arglist, int);
    }

    switch(vto[i].type) {
    case FORMAT_STRING:
      vto[i].data.str = va_arg(arglist, char *);
      break;

    case FORMAT_INTPTR:
    case FORMAT_UNKNOWN:
    case FORMAT_PTR:
      vto[i].data.ptr = va_arg(arglist, void *);
      break;

    case FORMAT_INT:
#ifdef HAVE_LONG_LONG_TYPE
      if((vto[i].flags & FLAGS_LONGLONG) && (vto[i].flags & FLAGS_UNSIGNED))
        vto[i].data.num.as_unsigned =
          (mp_uintmax_t)va_arg(arglist, mp_uintmax_t);
      else if(vto[i].flags & FLAGS_LONGLONG)
        vto[i].data.num.as_signed =
          (mp_intmax_t)va_arg(arglist, mp_intmax_t);
      else
#endif
      {
        if((vto[i].flags & FLAGS_LONG) && (vto[i].flags & FLAGS_UNSIGNED))
          vto[i].data.num.as_unsigned =
            (mp_uintmax_t)va_arg(arglist, unsigned long);
        else if(vto[i].flags & FLAGS_LONG)
          vto[i].data.num.as_signed =
            (mp_intmax_t)va_arg(arglist, long);
        else if(vto[i].flags & FLAGS_UNSIGNED)
          vto[i].data.num.as_unsigned =
            (mp_uintmax_t)va_arg(arglist, unsigned int);
        else
          vto[i].data.num.as_signed =
            (mp_intmax_t)va_arg(arglist, int);
      }
      break;

    case FORMAT_DOUBLE:
      vto[i].data.dnum = va_arg(arglist, double);
      break;

    case FORMAT_WIDTH:
      /* Argument has been read. Silently convert it into an integer
       * for later use
       */
      vto[i].type = FORMAT_INT;
      break;

    default:
      break;
    }
  }

  return 0;

}
