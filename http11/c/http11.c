
#line 1 "http11/c/http11.rl"
/* Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http11.h"

#define STATUS_CODE_LEN 3


struct _HTTPParserState {
  int cs;
  int mark;
  int header_name_end;
  int header_value_start;
  int header_value_end;

  char *tmp;
  size_t tmplen;
};


static int calculate_offset(const char *fpc,
                            const char *buf)
{
    return fpc - buf;
}


static int calculate_length(const char *fpc,
                            const char *buf,
                            const int offset)
{
    return fpc - buf - offset;
}


static const char *create_pointer(const char *buf,
                                  const int offset)
{
    return buf + offset;
}


static char * strnstr_(const char *s, const char *find, size_t slen)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0') {
        len = strlen(find);
        do {
            do {
                if (slen-- < 1 || (sc = *s++) == '\0')
                    return (NULL);
            } while (sc != c);
            if (len > slen)
                return (NULL);
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}


static void handle_element_callback(HTTPParser *parser,
                                    const char *fpc,
                                    const char *buf,
                                    int (*callback)(const char *, size_t))
{
    if (callback != NULL) {
        parser->error = callback(
            create_pointer(buf, parser->state->mark),
            calculate_length(fpc, buf, parser->state->mark)
        );
    }

    parser->state->mark = -1;
}


static void handle_status_code_callback(HTTPParser *parser,
                                        const char *fpc,
                                        const char *buf,
                                        int (*callback)(const unsigned short))
{
    char s[STATUS_CODE_LEN + 1];
    unsigned long int code;

    if (callback != NULL) {
        strncpy(s, create_pointer(buf, parser->state->mark), STATUS_CODE_LEN);
        s[STATUS_CODE_LEN] = '\0';

        errno = 0;
        code = strtoul(s, NULL, 10);
        if (errno) {
            parser->error = EINVALIDMSG;
            return;
        }

        parser->error = callback((const unsigned short)code);
    }

    parser->state->mark = -1;
}


static void handle_header_callback(HTTPParser *parser, const char *buf)
{
    const char *name = create_pointer(buf, parser->state->mark);
    const char *value = create_pointer(
        buf,
        parser->state->mark + parser->state->header_value_start
    );
    size_t namelen = parser->state->header_name_end;
    size_t valuelen = parser->state->header_value_end - parser->state->header_value_start;

    const char *found;
    const char *found_sp;
    const char *found_htab;
    const char *src;
    char *dest;
    char *newvalue;
    size_t left;
    size_t newvaluelen = 0;
    bool output = false;

    if (parser->http_header != NULL) {
        /* Determine if we have a \r\n inside of our header value, if we do
           then we have an obs-fold and we need to collapse it down to a
           single space, if we don't then we can do a zero copy callback. */
        found_sp = strnstr_(value, "\r\n ", valuelen);
        found_htab = strnstr_(value, "\r\n\t", valuelen);
        if (found_sp != NULL && found_htab != NULL) {
            found = found_sp < found_htab ? found_sp : found_htab;
        } else if (found_sp != NULL && found_htab == NULL) {
            found = found_sp;
        } else if (found_sp == NULL && found_htab != NULL) {
            found = found_htab;
        } else {
            found = NULL;
        }

        if (found != NULL) {
            newvalue = malloc(valuelen);
            src = value;
            dest = newvalue;
            left = valuelen;

            if (newvalue == NULL) {
                parser->error = ENOMEM;
                return;
            }

            while (found != NULL && left > 0) {
                if ((found - src) > 0) {
                    if (output) {
                        /* If we've already had output, then go ahead and add
                           a space. */
                        *dest = ' ';
                        dest++;
                        newvaluelen++;
                    }

                    /* Copy everything to the left of our "\r\n " */
                    memcpy(dest, src, found - src);

                    /* Record how much bigger our newvalue is now */
                    newvaluelen += (found - src);

                    /* Move our dest pointer to the end of the copied data */
                    dest += (found - src);

                    output = true;
                }

                /* Decrement how much of our value is left to search */
                left -= ((found - src) + 3);

                /* Move our src pointer to just past the "\r\n " */
                src = found + 3;

                /* Look for any blocks of whitespace past the obs-fold and omit
                   them from our new value. */
                while (src < value + valuelen) {
                    if (strncmp(src, " ", 1) && strncmp(src, "\t", 1))
                        break;
                    src++;
                    left--;
                }

                /* Look for another "\r\n " */
                found_sp = strnstr_(src, "\r\n ", left);
                found_htab = strnstr_(src, "\r\n\t", left);
                if (found_sp != NULL && found_htab != NULL) {
                    found = found_sp < found_htab ? found_sp : found_htab;
                } else if (found_sp != NULL && found_htab == NULL) {
                    found = found_sp;
                } else if (found_sp == NULL && found_htab != NULL) {
                    found = found_htab;
                } else {
                    found = NULL;
                }
            }

            /* Copy anything left over in our value */
            if (left > 0) {
                if (output) {
                    *dest = ' ';
                    dest++;
                    newvaluelen++;
                }

                memcpy(dest, src, left);
                newvaluelen += left;
            }

            /* Call our callback finally with our new unfolded value. */
            parser->error = parser->http_header(name, namelen, newvalue, newvaluelen);

            /* Free the memory that we added earlier. */
            free(newvalue);
        } else {
            /* Do the better, more optimized version */
            parser->error = parser->http_header(name, namelen, value, valuelen);
        }
    }

    parser->state->mark = -1;
}



#line 356 "http11/c/http11.rl"




#line 256 "http11/c/http11.c"
static const int http_parser_start = 1;
static const int http_parser_first_final = 45;
static const int http_parser_error = 0;

static const int http_parser_en_main = 1;


#line 360 "http11/c/http11.rl"


HTTPParser *HTTPParser_create()
{
    HTTPParser *parser = malloc(sizeof(*parser));

    if (!parser)
        goto error;

    parser->state = malloc(sizeof(*parser->state));

    if (!parser->state)
        goto error;

    /* We default all of our callbacks to NULL, we do this here instead of in
       HTTPParser_init because we don't want to wipe out callbacks on each
       request. */
    parser->request_method = NULL;
    parser->request_uri = NULL;
    parser->http_version = NULL;
    parser->status_code = NULL;
    parser->reason_phrase = NULL;
    parser->http_header = NULL;

    /* Done here so we can tell the difference between uninitialized and
       initialized in HTTPParser_init */
    parser->state->tmp = NULL;

    return parser;

    error:
        HTTPParser_destroy(parser);
        return NULL;
}


void HTTPParser_init(HTTPParser *parser)
{
    
#line 399 "http11/c/http11.rl"
    
#line 306 "http11/c/http11.c"
	{
	 parser->state->cs = http_parser_start;
	}

#line 400 "http11/c/http11.rl"

    parser->state->mark = -1;
    parser->state->header_name_end = 0;
    parser->state->header_value_start = 0;
    parser->state->header_value_end = 0;

    parser->state->tmplen = 0;

    if (parser->state->tmp != NULL) {
        free(parser->state->tmp);
        parser->state->tmp = NULL;
    }

    parser->finished = false;
    parser->error = 0;
}


size_t HTTPParser_execute(HTTPParser *parser,
                          const char *buf,
                          size_t offset,
                          size_t length)
{
    char *rtmp;
    const char *p;
    const char *pe;
    const char *eof = NULL;

    if (buf == NULL) {
        p = pe = eof;
    } else {
        /* If we have anything stored in our temp buffer, then we want to use
           that buffer combined with the new buffer instead of just using the
           new buffer. */
        if (parser->state->tmp != NULL) {
            parser->state->tmplen += (length - offset);

            /* Resize our temp buffer to also hold the additional data */
            rtmp = realloc(parser->state->tmp, parser->state->tmplen);
            if (rtmp == NULL) {
                /* TODO: Do we really need to finish the parser if we can't
                         realloc? Another call with the same data might succeed
                         I think? */
                parser->finished = true;
                parser->error = ENOMEM;
                return 0;
            }
            parser->state->tmp = rtmp;

            /* Copy the data from the new buffer into our temporary buffer. */
            memcpy(
                parser->state->tmp + (parser->state->tmplen - (length - offset)),
                buf + offset,
                length - offset
            );

            /* Point the buf to our new buffer now, and point the mark to the
               beginning. */
            buf = parser->state->tmp;
            parser->state->mark = 0;

            /* Adjust our length and offset to match the new buffer. */
            offset = parser->state->tmplen - (length - offset);
            length = parser->state->tmplen;
        }

        p = buf + offset;
        pe = buf + length;
    }

    
#line 471 "http11/c/http11.rl"
    
#line 385 "http11/c/http11.c"
	{
	if ( p == pe )
		goto _test_eof;
	goto _resume;

_again:
	switch (  parser->state->cs ) {
		case 1: goto st1;
		case 0: goto st0;
		case 2: goto st2;
		case 3: goto st3;
		case 4: goto st4;
		case 5: goto st5;
		case 6: goto st6;
		case 7: goto st7;
		case 8: goto st8;
		case 9: goto st9;
		case 10: goto st10;
		case 11: goto st11;
		case 12: goto st12;
		case 13: goto st13;
		case 14: goto st14;
		case 15: goto st15;
		case 16: goto st16;
		case 45: goto st45;
		case 17: goto st17;
		case 18: goto st18;
		case 19: goto st19;
		case 20: goto st20;
		case 21: goto st21;
		case 22: goto st22;
		case 23: goto st23;
		case 24: goto st24;
		case 25: goto st25;
		case 26: goto st26;
		case 27: goto st27;
		case 28: goto st28;
		case 29: goto st29;
		case 30: goto st30;
		case 31: goto st31;
		case 32: goto st32;
		case 33: goto st33;
		case 34: goto st34;
		case 35: goto st35;
		case 36: goto st36;
		case 37: goto st37;
		case 38: goto st38;
		case 39: goto st39;
		case 40: goto st40;
		case 41: goto st41;
		case 42: goto st42;
		case 43: goto st43;
		case 44: goto st44;
	default: break;
	}

	if ( ++p == pe )
		goto _test_eof;
_resume:
	switch (  parser->state->cs )
	{
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
	switch( (*p) ) {
		case 10: goto st2;
		case 13: goto st3;
		case 33: goto tr3;
		case 72: goto tr4;
		case 124: goto tr3;
		case 126: goto tr3;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr3;
		} else if ( (*p) >= 35 )
			goto tr3;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr3;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else
			goto tr3;
	} else
		goto tr3;
	goto st0;
tr12:
#line 310 "http11/c/http11.rl"
	{
        parser->error = EBADVERSION;
    }
	goto st0;
#line 483 "http11/c/http11.c"
st0:
 parser->state->cs = 0;
	goto _out;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 10: goto st2;
		case 13: goto st3;
		case 33: goto tr3;
		case 124: goto tr3;
		case 126: goto tr3;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr3;
		} else if ( (*p) >= 35 )
			goto tr3;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr3;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else
			goto tr3;
	} else
		goto tr3;
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	if ( (*p) == 10 )
		goto st2;
	goto st0;
tr3:
#line 250 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st4;
tr50:
#line 310 "http11/c/http11.rl"
	{
        parser->error = EBADVERSION;
    }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 539 "http11/c/http11.c"
	switch( (*p) ) {
		case 32: goto tr5;
		case 33: goto st4;
		case 124: goto st4;
		case 126: goto st4;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st4;
		} else if ( (*p) >= 35 )
			goto st4;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st4;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st4;
		} else
			goto st4;
	} else
		goto st4;
	goto st0;
tr5:
#line 266 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->request_method);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st5;
tr49:
#line 266 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->request_method);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
#line 310 "http11/c/http11.rl"
	{
        parser->error = EBADVERSION;
    }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 590 "http11/c/http11.c"
	if ( (*p) == 10 )
		goto st0;
	goto tr7;
tr7:
#line 250 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st6;
tr11:
#line 310 "http11/c/http11.rl"
	{
        parser->error = EBADVERSION;
    }
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
#line 610 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto st7;
	}
	goto st6;
tr13:
#line 310 "http11/c/http11.rl"
	{
        parser->error = EBADVERSION;
    }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 626 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto st7;
		case 72: goto tr10;
	}
	goto st6;
tr10:
#line 273 "http11/c/http11.rl"
	{
        /* We use fpc -1 here because otherwise this will catch the SP in the
           buffer. */
        handle_element_callback(parser, p - 1, buf, parser->request_uri);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
#line 250 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 652 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto tr12;
		case 32: goto tr13;
		case 84: goto st9;
	}
	goto tr11;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	switch( (*p) ) {
		case 10: goto tr12;
		case 32: goto tr13;
		case 84: goto st10;
	}
	goto tr11;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	switch( (*p) ) {
		case 10: goto tr12;
		case 32: goto tr13;
		case 80: goto st11;
	}
	goto tr11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	switch( (*p) ) {
		case 10: goto tr12;
		case 32: goto tr13;
		case 47: goto st12;
	}
	goto tr11;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	switch( (*p) ) {
		case 10: goto tr12;
		case 32: goto tr13;
		case 49: goto st13;
	}
	goto tr11;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	switch( (*p) ) {
		case 10: goto tr12;
		case 32: goto tr13;
		case 46: goto st14;
	}
	goto tr11;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 10: goto tr12;
		case 32: goto tr13;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st15;
	goto tr11;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	switch( (*p) ) {
		case 10: goto tr21;
		case 13: goto tr22;
		case 32: goto tr13;
	}
	goto tr11;
tr21:
#line 310 "http11/c/http11.rl"
	{
        parser->error = EBADVERSION;
    }
#line 282 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->http_version);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st16;
tr62:
#line 296 "http11/c/http11.rl"
	{
        handle_status_code_callback(parser, p, buf, parser->status_code);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st16;
tr66:
#line 250 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
#line 289 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->reason_phrase);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st16;
tr69:
#line 289 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->reason_phrase);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st16;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
#line 778 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto tr23;
		case 13: goto st17;
		case 33: goto tr25;
		case 124: goto tr25;
		case 126: goto tr25;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr25;
		} else if ( (*p) >= 35 )
			goto tr25;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr25;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr25;
		} else
			goto tr25;
	} else
		goto tr25;
	goto st0;
tr23:
#line 314 "http11/c/http11.rl"
	{
        {p++;  parser->state->cs = 45; goto _out;}
    }
	goto st45;
tr42:
#line 303 "http11/c/http11.rl"
	{
        handle_header_callback(parser, buf);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
#line 314 "http11/c/http11.rl"
	{
        {p++;  parser->state->cs = 45; goto _out;}
    }
	goto st45;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
#line 827 "http11/c/http11.c"
	goto st0;
tr43:
#line 303 "http11/c/http11.rl"
	{
        handle_header_callback(parser, buf);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 842 "http11/c/http11.c"
	if ( (*p) == 10 )
		goto tr23;
	goto st0;
tr25:
#line 250 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st18;
tr44:
#line 303 "http11/c/http11.rl"
	{
        handle_header_callback(parser, buf);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
#line 250 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 869 "http11/c/http11.c"
	switch( (*p) ) {
		case 33: goto st18;
		case 58: goto tr27;
		case 124: goto st18;
		case 126: goto st18;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st18;
		} else if ( (*p) >= 35 )
			goto st18;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st18;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st18;
		} else
			goto st18;
	} else
		goto st18;
	goto st0;
tr27:
#line 254 "http11/c/http11.rl"
	{
        parser->state->header_name_end = calculate_offset(p, buf) - parser->state->mark;
    }
	goto st19;
tr29:
#line 258 "http11/c/http11.rl"
	{
        parser->state->header_value_start = calculate_offset(p, buf) - parser->state->mark;
    }
#line 262 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf) - parser->state->mark;
    }
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 914 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto tr29;
		case 10: goto tr30;
		case 13: goto tr31;
		case 32: goto tr29;
		case 127: goto st0;
	}
	if ( 0 <= (*p) && (*p) <= 31 )
		goto st0;
	goto tr28;
tr28:
#line 258 "http11/c/http11.rl"
	{
        parser->state->header_value_start = calculate_offset(p, buf) - parser->state->mark;
    }
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 935 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto tr33;
		case 10: goto tr34;
		case 13: goto tr35;
		case 32: goto tr33;
		case 127: goto st0;
	}
	if ( 0 <= (*p) && (*p) <= 31 )
		goto st0;
	goto st20;
tr33:
#line 262 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf) - parser->state->mark;
    }
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 956 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto st21;
		case 10: goto st24;
		case 13: goto st25;
		case 32: goto st21;
		case 127: goto st0;
	}
	if ( 0 <= (*p) && (*p) <= 31 )
		goto st0;
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	switch( (*p) ) {
		case 9: goto tr40;
		case 10: goto tr34;
		case 13: goto tr35;
		case 32: goto tr40;
		case 127: goto st0;
	}
	if ( 0 <= (*p) && (*p) <= 31 )
		goto st0;
	goto st20;
tr40:
#line 262 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf) - parser->state->mark;
    }
	goto st23;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
#line 991 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto st23;
		case 10: goto st24;
		case 13: goto st25;
		case 32: goto st23;
	}
	goto st0;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	switch( (*p) ) {
		case 10: goto tr42;
		case 13: goto tr43;
		case 33: goto tr44;
		case 124: goto tr44;
		case 126: goto tr44;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr44;
		} else if ( (*p) >= 35 )
			goto tr44;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr44;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr44;
		} else
			goto tr44;
	} else
		goto tr44;
	goto st0;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	if ( (*p) == 10 )
		goto st24;
	goto st0;
tr30:
#line 258 "http11/c/http11.rl"
	{
        parser->state->header_value_start = calculate_offset(p, buf) - parser->state->mark;
    }
#line 262 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf) - parser->state->mark;
    }
	goto st26;
tr34:
#line 262 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf) - parser->state->mark;
    }
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
#line 1055 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto st27;
		case 10: goto tr42;
		case 13: goto tr43;
		case 32: goto st27;
		case 33: goto tr44;
		case 124: goto tr44;
		case 126: goto tr44;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr44;
		} else if ( (*p) >= 35 )
			goto tr44;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr44;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr44;
		} else
			goto tr44;
	} else
		goto tr44;
	goto st0;
tr46:
#line 262 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf) - parser->state->mark;
    }
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
#line 1093 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto tr46;
		case 10: goto tr34;
		case 13: goto tr35;
		case 32: goto tr46;
		case 127: goto st0;
	}
	if ( 0 <= (*p) && (*p) <= 31 )
		goto st0;
	goto st20;
tr31:
#line 258 "http11/c/http11.rl"
	{
        parser->state->header_value_start = calculate_offset(p, buf) - parser->state->mark;
    }
#line 262 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf) - parser->state->mark;
    }
	goto st28;
tr35:
#line 262 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf) - parser->state->mark;
    }
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
#line 1124 "http11/c/http11.c"
	if ( (*p) == 10 )
		goto st26;
	goto st0;
tr22:
#line 310 "http11/c/http11.rl"
	{
        parser->error = EBADVERSION;
    }
#line 282 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->http_version);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 1145 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st16;
		case 32: goto st7;
	}
	goto st6;
tr4:
#line 250 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st30;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
#line 1161 "http11/c/http11.c"
	switch( (*p) ) {
		case 32: goto tr49;
		case 33: goto tr50;
		case 84: goto st31;
		case 124: goto tr50;
		case 126: goto tr50;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr50;
		} else if ( (*p) >= 35 )
			goto tr50;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr50;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr50;
		} else
			goto tr50;
	} else
		goto tr50;
	goto tr12;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	switch( (*p) ) {
		case 32: goto tr49;
		case 33: goto tr50;
		case 84: goto st32;
		case 124: goto tr50;
		case 126: goto tr50;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr50;
		} else if ( (*p) >= 35 )
			goto tr50;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr50;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr50;
		} else
			goto tr50;
	} else
		goto tr50;
	goto tr12;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	switch( (*p) ) {
		case 32: goto tr49;
		case 33: goto tr50;
		case 80: goto st33;
		case 124: goto tr50;
		case 126: goto tr50;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr50;
		} else if ( (*p) >= 35 )
			goto tr50;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr50;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr50;
		} else
			goto tr50;
	} else
		goto tr50;
	goto tr12;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	switch( (*p) ) {
		case 32: goto tr49;
		case 33: goto tr50;
		case 47: goto st34;
		case 124: goto tr50;
		case 126: goto tr50;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr50;
		} else if ( (*p) >= 35 )
			goto tr50;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr50;
		} else if ( (*p) >= 65 )
			goto tr50;
	} else
		goto tr50;
	goto tr12;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	if ( (*p) == 49 )
		goto st35;
	goto tr12;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	if ( (*p) == 46 )
		goto st36;
	goto tr12;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st37;
	goto tr12;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
	if ( (*p) == 32 )
		goto tr58;
	goto tr12;
tr58:
#line 310 "http11/c/http11.rl"
	{
        parser->error = EBADVERSION;
    }
#line 282 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->http_version);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
#line 1316 "http11/c/http11.c"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr59;
	goto st0;
tr59:
#line 250 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st39;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
#line 1330 "http11/c/http11.c"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st40;
	goto st0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st41;
	goto st0;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
	switch( (*p) ) {
		case 10: goto tr62;
		case 13: goto tr63;
		case 32: goto tr64;
	}
	goto st0;
tr63:
#line 296 "http11/c/http11.rl"
	{
        handle_status_code_callback(parser, p, buf, parser->status_code);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st42;
tr67:
#line 250 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
#line 289 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->reason_phrase);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st42;
tr70:
#line 289 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->reason_phrase);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st42;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
#line 1386 "http11/c/http11.c"
	if ( (*p) == 10 )
		goto st16;
	goto st0;
tr64:
#line 296 "http11/c/http11.rl"
	{
        handle_status_code_callback(parser, p, buf, parser->status_code);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st43;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
#line 1403 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto tr66;
		case 13: goto tr67;
		case 127: goto st0;
	}
	if ( (*p) > 8 ) {
		if ( 11 <= (*p) && (*p) <= 31 )
			goto st0;
	} else if ( (*p) >= 0 )
		goto st0;
	goto tr65;
tr65:
#line 250 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st44;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
#line 1425 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto tr69;
		case 13: goto tr70;
		case 127: goto st0;
	}
	if ( (*p) > 8 ) {
		if ( 11 <= (*p) && (*p) <= 31 )
			goto st0;
	} else if ( (*p) >= 0 )
		goto st0;
	goto st44;
	}
	_test_eof1:  parser->state->cs = 1; goto _test_eof; 
	_test_eof2:  parser->state->cs = 2; goto _test_eof; 
	_test_eof3:  parser->state->cs = 3; goto _test_eof; 
	_test_eof4:  parser->state->cs = 4; goto _test_eof; 
	_test_eof5:  parser->state->cs = 5; goto _test_eof; 
	_test_eof6:  parser->state->cs = 6; goto _test_eof; 
	_test_eof7:  parser->state->cs = 7; goto _test_eof; 
	_test_eof8:  parser->state->cs = 8; goto _test_eof; 
	_test_eof9:  parser->state->cs = 9; goto _test_eof; 
	_test_eof10:  parser->state->cs = 10; goto _test_eof; 
	_test_eof11:  parser->state->cs = 11; goto _test_eof; 
	_test_eof12:  parser->state->cs = 12; goto _test_eof; 
	_test_eof13:  parser->state->cs = 13; goto _test_eof; 
	_test_eof14:  parser->state->cs = 14; goto _test_eof; 
	_test_eof15:  parser->state->cs = 15; goto _test_eof; 
	_test_eof16:  parser->state->cs = 16; goto _test_eof; 
	_test_eof45:  parser->state->cs = 45; goto _test_eof; 
	_test_eof17:  parser->state->cs = 17; goto _test_eof; 
	_test_eof18:  parser->state->cs = 18; goto _test_eof; 
	_test_eof19:  parser->state->cs = 19; goto _test_eof; 
	_test_eof20:  parser->state->cs = 20; goto _test_eof; 
	_test_eof21:  parser->state->cs = 21; goto _test_eof; 
	_test_eof22:  parser->state->cs = 22; goto _test_eof; 
	_test_eof23:  parser->state->cs = 23; goto _test_eof; 
	_test_eof24:  parser->state->cs = 24; goto _test_eof; 
	_test_eof25:  parser->state->cs = 25; goto _test_eof; 
	_test_eof26:  parser->state->cs = 26; goto _test_eof; 
	_test_eof27:  parser->state->cs = 27; goto _test_eof; 
	_test_eof28:  parser->state->cs = 28; goto _test_eof; 
	_test_eof29:  parser->state->cs = 29; goto _test_eof; 
	_test_eof30:  parser->state->cs = 30; goto _test_eof; 
	_test_eof31:  parser->state->cs = 31; goto _test_eof; 
	_test_eof32:  parser->state->cs = 32; goto _test_eof; 
	_test_eof33:  parser->state->cs = 33; goto _test_eof; 
	_test_eof34:  parser->state->cs = 34; goto _test_eof; 
	_test_eof35:  parser->state->cs = 35; goto _test_eof; 
	_test_eof36:  parser->state->cs = 36; goto _test_eof; 
	_test_eof37:  parser->state->cs = 37; goto _test_eof; 
	_test_eof38:  parser->state->cs = 38; goto _test_eof; 
	_test_eof39:  parser->state->cs = 39; goto _test_eof; 
	_test_eof40:  parser->state->cs = 40; goto _test_eof; 
	_test_eof41:  parser->state->cs = 41; goto _test_eof; 
	_test_eof42:  parser->state->cs = 42; goto _test_eof; 
	_test_eof43:  parser->state->cs = 43; goto _test_eof; 
	_test_eof44:  parser->state->cs = 44; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch (  parser->state->cs ) {
	case 1: 
	case 2: 
	case 3: 
	case 4: 
	case 5: 
	case 6: 
	case 7: 
	case 15: 
	case 16: 
	case 17: 
	case 18: 
	case 19: 
	case 20: 
	case 21: 
	case 22: 
	case 23: 
	case 24: 
	case 25: 
	case 26: 
	case 27: 
	case 28: 
	case 29: 
	case 37: 
	case 38: 
	case 39: 
	case 40: 
	case 41: 
	case 42: 
	case 43: 
	case 44: 
#line 318 "http11/c/http11.rl"
	{
        parser->error = EEOF;
        { parser->state->cs = (http_parser_error); goto _again;}
    }
	break;
	case 8: 
	case 9: 
	case 10: 
	case 11: 
	case 12: 
	case 13: 
	case 14: 
	case 30: 
	case 31: 
	case 32: 
	case 33: 
	case 34: 
	case 35: 
	case 36: 
#line 310 "http11/c/http11.rl"
	{
        parser->error = EBADVERSION;
    }
#line 318 "http11/c/http11.rl"
	{
        parser->error = EEOF;
        { parser->state->cs = (http_parser_error); goto _again;}
    }
	break;
#line 1548 "http11/c/http11.c"
	}
	}

	_out: {}
	}

#line 472 "http11/c/http11.rl"

    if (parser->state->cs == http_parser_error || parser->state->cs >= http_parser_first_final ) {
        parser->finished = true;

        if (parser->state->cs == http_parser_error && !parser->error) {
            parser->error = EINVALIDMSG;
        }

        /* We've finished parsing the request, if we have a tmp buffer
           allocated then we want to free it. */
        if (parser->state->tmp != NULL) {
            free(parser->state->tmp);
            parser->state->tmp = NULL;
        }
    } else if (parser->state->mark >= 0) {
        /* If the parser isn't finished and we have anything marked then we
           need to save the trailing part of the buffer. */
        parser->state->tmplen = calculate_length(pe, buf, parser->state->mark);

        rtmp = realloc(parser->state->tmp, parser->state->tmplen);
        if (rtmp == NULL)
        {
            parser->finished = true;
            parser->error = ENOMEM;
            return (length - offset) - (pe - p);
        }
        parser->state->tmp = rtmp;

        memcpy(
            parser->state->tmp,
            create_pointer(buf, parser->state->mark),
            parser->state->tmplen
        );
    } else {
        /* If the parser isn't finished, but we have nothing marked, then there
           is nothing to save. If we have anything in our tmp buffer then we
           should free it as it's no longer needed. */
        if (parser->state->tmp != NULL) {
            free(parser->state->tmp);
            parser->state->tmp = NULL;
        }
    }

    return (length - offset) - (pe - p);
}


void HTTPParser_destroy(HTTPParser *parser)
{
    if (parser) {
        if (parser->state != NULL) {
            free(parser->state->tmp);
            free(parser->state);
        }
        free(parser);
    }
}
