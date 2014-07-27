
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


struct _HTTPParserState {
  int cs;
  int mark;
  int header_name_start;
  int header_name_end;
  int header_value_start;
  int header_value_end;
};


static int calculate_offset(const char *fpc,
                            const char *buf) {
    return fpc - buf;
}


static int calculate_length(const char *fpc,
                            const char *buf,
                            const int offset) {
    return fpc - buf - offset;
}


static const char *create_pointer(const char *buf,
                                  const int offset) {
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
                                    int (*callback)(const char *, size_t)) {
    if (callback != NULL) {
        parser->error = callback(
            create_pointer(buf, parser->state->mark),
            calculate_length(fpc, buf, parser->state->mark)
        );
    }
}


static void handle_status_code_callback(HTTPParser *parser,
                                        const char *fpc,
                                        const char *buf,
                                        int (*callback)(const unsigned short))
{
    int length = calculate_length(fpc, buf, parser->state->mark);
    char s[length + 1];
    unsigned long int code;

    if (callback != NULL) {
        strncpy(s, create_pointer(buf, parser->state->mark), length);
        s[length] = '\0';

        errno = 0;
        code = strtoul(s, NULL, 10);
        if (errno) {
            parser->error = 1;
            return;
        }

        parser->error = callback((const unsigned short)code);
    }
}

static void handle_header_callback(HTTPParser *parser, const char *buf)
{
    const char *name = create_pointer(buf, parser->state->header_name_start);
    const char *value = create_pointer(buf, parser->state->header_value_start);
    size_t namelen = parser->state->header_name_end - parser->state->header_name_start;
    size_t valuelen = parser->state->header_value_end - parser->state->header_value_start;

    const char *found;
    const char *found_sp;
    const char *found_htab;
    const char *src;
    char *dest;
    char *newvalue;
    size_t left;
    size_t newvaluelen = 0;
    bool first = true;

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
                parser->error = 1;
                return;
            }

            while (found != NULL && left > 0) {
                if (!first) {
                    /* If this is not the first time through the loop, then add
                       add s space to our new value. We do this here because
                       we *only* want to add this, if there is another bit to
                       memcpy. */
                    *dest = ' ';
                    dest++;
                    newvaluelen++;
                } else {
                    first = false;
                }

                /* Copy everything to the left of our "\r\n " */
                memcpy(dest, src, found - src);

                /* Record how much bigger our newvalue is now */
                newvaluelen += (found - src);

                /* Decrement how much of our value is left to search */
                left -= ((found - src) + 3);

                /* Move our dest pointer to the end of the just copied data */
                dest += (found - src);

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
                *dest = ' ';
                dest++;
                newvaluelen++;

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
}



#line 327 "http11/c/http11.rl"




#line 238 "http11/c/http11.c"
static const int http_parser_start = 1;
static const int http_parser_first_final = 43;
static const int http_parser_error = 0;

static const int http_parser_en_main = 1;


#line 331 "http11/c/http11.rl"


HTTPParser *HTTPParser_create() {
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

    return parser;

    error:
        HTTPParser_destroy(parser);
        return NULL;
}


void HTTPParser_init(HTTPParser *parser) {
    
#line 364 "http11/c/http11.rl"
    
#line 282 "http11/c/http11.c"
	{
	 parser->state->cs = http_parser_start;
	}

#line 365 "http11/c/http11.rl"

    parser->state->mark = 0;
    parser->state->header_name_start = 0;
    parser->state->header_name_end = 0;
    parser->state->header_value_start = 0;
    parser->state->header_value_end = 0;

    parser->finished = false;
    parser->error = 0;
}


size_t HTTPParser_execute(HTTPParser *parser,
                          const char *buf,
                          size_t length,
                          size_t offset) {
    const char *p = buf + offset;
    const char *pe = buf + length;

    
#line 385 "http11/c/http11.rl"
    
#line 310 "http11/c/http11.c"
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
		case 43: goto st43;
		case 15: goto st15;
		case 16: goto st16;
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
		case 33: goto tr0;
		case 72: goto tr2;
		case 124: goto tr0;
		case 126: goto tr0;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr0;
		} else if ( (*p) >= 35 )
			goto tr0;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr0;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto tr0;
	goto st0;
st0:
 parser->state->cs = 0;
	goto _out;
tr0:
#line 232 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 411 "http11/c/http11.c"
	switch( (*p) ) {
		case 32: goto tr3;
		case 33: goto st2;
		case 124: goto st2;
		case 126: goto st2;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st2;
		} else if ( (*p) >= 35 )
			goto st2;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st2;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st2;
		} else
			goto st2;
	} else
		goto st2;
	goto st0;
tr3:
#line 252 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->request_method);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 449 "http11/c/http11.c"
	if ( (*p) == 10 )
		goto st0;
	goto tr5;
tr5:
#line 232 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 463 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto tr7;
	}
	goto st4;
tr7:
#line 259 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->request_uri);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 482 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto tr7;
		case 72: goto tr8;
	}
	goto st4;
tr8:
#line 232 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
#line 499 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto tr7;
		case 84: goto st7;
	}
	goto st4;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto tr7;
		case 84: goto st8;
	}
	goto st4;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto tr7;
		case 80: goto st9;
	}
	goto st4;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto tr7;
		case 47: goto st10;
	}
	goto st4;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto tr7;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st11;
	goto st4;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto tr7;
		case 46: goto st12;
	}
	goto st4;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto tr7;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st13;
	goto st4;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	switch( (*p) ) {
		case 10: goto tr16;
		case 13: goto tr17;
		case 32: goto tr7;
	}
	goto st4;
tr16:
#line 266 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->http_version);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st14;
tr57:
#line 232 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
#line 280 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->reason_phrase);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st14;
tr60:
#line 280 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->reason_phrase);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
#line 613 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st43;
		case 13: goto st15;
		case 33: goto tr20;
		case 124: goto tr20;
		case 126: goto tr20;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr20;
		} else if ( (*p) >= 35 )
			goto tr20;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr20;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr20;
		} else
			goto tr20;
	} else
		goto tr20;
	goto st0;
tr37:
#line 287 "http11/c/http11.rl"
	{
        handle_header_callback(parser, buf);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st43;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
#line 652 "http11/c/http11.c"
	goto st0;
tr38:
#line 287 "http11/c/http11.rl"
	{
        handle_header_callback(parser, buf);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st15;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
#line 667 "http11/c/http11.c"
	if ( (*p) == 10 )
		goto st43;
	goto st0;
tr20:
#line 236 "http11/c/http11.rl"
	{
        parser->state->header_name_start = calculate_offset(p, buf);
    }
	goto st16;
tr39:
#line 287 "http11/c/http11.rl"
	{
        handle_header_callback(parser, buf);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
#line 236 "http11/c/http11.rl"
	{
        parser->state->header_name_start = calculate_offset(p, buf);
    }
	goto st16;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
#line 694 "http11/c/http11.c"
	switch( (*p) ) {
		case 33: goto st16;
		case 58: goto tr22;
		case 124: goto st16;
		case 126: goto st16;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st16;
		} else if ( (*p) >= 35 )
			goto st16;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st16;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st16;
		} else
			goto st16;
	} else
		goto st16;
	goto st0;
tr22:
#line 240 "http11/c/http11.rl"
	{
        parser->state->header_name_end = calculate_offset(p, buf);
    }
	goto st17;
tr24:
#line 244 "http11/c/http11.rl"
	{
        parser->state->header_value_start = calculate_offset(p, buf);
    }
#line 248 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf);
    }
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 739 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto tr24;
		case 10: goto tr25;
		case 13: goto tr26;
		case 32: goto tr24;
		case 127: goto st0;
	}
	if ( 0 <= (*p) && (*p) <= 31 )
		goto st0;
	goto tr23;
tr23:
#line 244 "http11/c/http11.rl"
	{
        parser->state->header_value_start = calculate_offset(p, buf);
    }
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 760 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto tr28;
		case 10: goto tr29;
		case 13: goto tr30;
		case 32: goto tr28;
		case 127: goto st0;
	}
	if ( 0 <= (*p) && (*p) <= 31 )
		goto st0;
	goto st18;
tr28:
#line 248 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf);
    }
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 781 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto st19;
		case 10: goto st22;
		case 13: goto st23;
		case 32: goto st19;
		case 127: goto st0;
	}
	if ( 0 <= (*p) && (*p) <= 31 )
		goto st0;
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	switch( (*p) ) {
		case 9: goto tr35;
		case 10: goto tr29;
		case 13: goto tr30;
		case 32: goto tr35;
		case 127: goto st0;
	}
	if ( 0 <= (*p) && (*p) <= 31 )
		goto st0;
	goto st18;
tr35:
#line 248 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf);
    }
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 816 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto st21;
		case 10: goto st22;
		case 13: goto st23;
		case 32: goto st21;
	}
	goto st0;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	switch( (*p) ) {
		case 10: goto tr37;
		case 13: goto tr38;
		case 33: goto tr39;
		case 124: goto tr39;
		case 126: goto tr39;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr39;
		} else if ( (*p) >= 35 )
			goto tr39;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr39;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr39;
		} else
			goto tr39;
	} else
		goto tr39;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	if ( (*p) == 10 )
		goto st22;
	goto st0;
tr25:
#line 244 "http11/c/http11.rl"
	{
        parser->state->header_value_start = calculate_offset(p, buf);
    }
#line 248 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf);
    }
	goto st24;
tr29:
#line 248 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf);
    }
	goto st24;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
#line 880 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto st25;
		case 10: goto tr37;
		case 13: goto tr38;
		case 32: goto st25;
		case 33: goto tr39;
		case 124: goto tr39;
		case 126: goto tr39;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr39;
		} else if ( (*p) >= 35 )
			goto tr39;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr39;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr39;
		} else
			goto tr39;
	} else
		goto tr39;
	goto st0;
tr41:
#line 248 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf);
    }
	goto st25;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
#line 918 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto tr41;
		case 10: goto tr29;
		case 13: goto tr30;
		case 32: goto tr41;
		case 127: goto st0;
	}
	if ( 0 <= (*p) && (*p) <= 31 )
		goto st0;
	goto st18;
tr26:
#line 244 "http11/c/http11.rl"
	{
        parser->state->header_value_start = calculate_offset(p, buf);
    }
#line 248 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf);
    }
	goto st26;
tr30:
#line 248 "http11/c/http11.rl"
	{
        parser->state->header_value_end = calculate_offset(p, buf);
    }
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
#line 949 "http11/c/http11.c"
	if ( (*p) == 10 )
		goto st24;
	goto st0;
tr17:
#line 266 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->http_version);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
#line 966 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st14;
		case 32: goto tr7;
	}
	goto st4;
tr2:
#line 232 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
#line 982 "http11/c/http11.c"
	switch( (*p) ) {
		case 32: goto tr3;
		case 33: goto st2;
		case 84: goto st29;
		case 124: goto st2;
		case 126: goto st2;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st2;
		} else if ( (*p) >= 35 )
			goto st2;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st2;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st2;
		} else
			goto st2;
	} else
		goto st2;
	goto st0;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
	switch( (*p) ) {
		case 32: goto tr3;
		case 33: goto st2;
		case 84: goto st30;
		case 124: goto st2;
		case 126: goto st2;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st2;
		} else if ( (*p) >= 35 )
			goto st2;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st2;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st2;
		} else
			goto st2;
	} else
		goto st2;
	goto st0;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	switch( (*p) ) {
		case 32: goto tr3;
		case 33: goto st2;
		case 80: goto st31;
		case 124: goto st2;
		case 126: goto st2;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st2;
		} else if ( (*p) >= 35 )
			goto st2;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st2;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st2;
		} else
			goto st2;
	} else
		goto st2;
	goto st0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	switch( (*p) ) {
		case 32: goto tr3;
		case 33: goto st2;
		case 47: goto st32;
		case 124: goto st2;
		case 126: goto st2;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st2;
		} else if ( (*p) >= 35 )
			goto st2;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st2;
		} else if ( (*p) >= 65 )
			goto st2;
	} else
		goto st2;
	goto st0;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st33;
	goto st0;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	if ( (*p) == 46 )
		goto st34;
	goto st0;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st35;
	goto st0;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	if ( (*p) == 32 )
		goto tr51;
	goto st0;
tr51:
#line 266 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->http_version);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
#line 1133 "http11/c/http11.c"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr52;
	goto st0;
tr52:
#line 232 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
#line 1147 "http11/c/http11.c"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st38;
	goto st0;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st39;
	goto st0;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
	if ( (*p) == 32 )
		goto tr55;
	goto st0;
tr55:
#line 273 "http11/c/http11.rl"
	{
        handle_status_code_callback(parser, p, buf, parser->status_code);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st40;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
#line 1178 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto tr57;
		case 13: goto tr58;
		case 127: goto st0;
	}
	if ( (*p) > 8 ) {
		if ( 11 <= (*p) && (*p) <= 31 )
			goto st0;
	} else if ( (*p) >= 0 )
		goto st0;
	goto tr56;
tr56:
#line 232 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st41;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
#line 1200 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto tr60;
		case 13: goto tr61;
		case 127: goto st0;
	}
	if ( (*p) > 8 ) {
		if ( 11 <= (*p) && (*p) <= 31 )
			goto st0;
	} else if ( (*p) >= 0 )
		goto st0;
	goto st41;
tr58:
#line 232 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
#line 280 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->reason_phrase);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st42;
tr61:
#line 280 "http11/c/http11.rl"
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
#line 1238 "http11/c/http11.c"
	if ( (*p) == 10 )
		goto st14;
	goto st0;
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
	_test_eof43:  parser->state->cs = 43; goto _test_eof; 
	_test_eof15:  parser->state->cs = 15; goto _test_eof; 
	_test_eof16:  parser->state->cs = 16; goto _test_eof; 
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

	_test_eof: {}
	_out: {}
	}

#line 386 "http11/c/http11.rl"

    if (parser->state->cs == http_parser_error || parser->state->cs >= http_parser_first_final ) {
        parser->finished = true;

        if (parser->state->cs == http_parser_error && !parser->error) {
            parser->error = 1;
        }
    }

    return 1;
}


void HTTPParser_destroy(HTTPParser *parser) {
    if (parser) {
        free(parser->state);
        free(parser);
    }
}
