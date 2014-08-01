
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


#define MARK_START(fpc, buf, mark) (fpc - buf - mark)
#define MARK_LEN(fpc, buf, item, mark) (fpc - buf - item - mark)


struct _HTTPParserState {
  int cs;
  int mark;

  int method;
  int method_len;
  int uri;
  int uri_len;
  int http_version;
  int http_version_len;
  int status_code;
  int status_code_len;
  int reason_phrase;
  int reason_phrase_len;
  int field_name;
  int field_name_len;
  int field_value;
  int field_value_len;

  char *tmp;
  size_t tmplen;
};


static const int find_obs_fold(const char *buf, const int len)
{
    int i = 0;

    for (; i < (len - 2); i++) {
        if (strncmp(buf + i, "\r\n ", 3) == 0
                || strncmp(buf + i, "\r\n\t", 3) == 0
                || strncmp(buf + i, "\n ", 2) == 0
                || strncmp(buf + i, "\n\t", 2) == 0) {
            return i;
        }
    }

    return -1;
}


static void collapse_obs_fold(char *buf, int *len)
{
    int find;
    int rlen;
    int rkeep;
    int i;

    find = find_obs_fold(buf, *len);
    while(find >= 0) {
        if (*(buf + find) == '\r') {
            rlen = 3;
        } else {
            rlen = 2;
        }

        /* Find any additional whitespace we need to remove. */
        for (i = find + rlen; i < *len; i++) {
            if (*(buf + i) == ' ' || *(buf + i) == '\t') {
                rlen++;
            } else {
                break;
            }
        }

        if (find > 0) {
            /* Replace the first character in our find with a single SP if this
               isn't the first character in the value. */
            *(buf + find) = ' ';

            rkeep = 1;
        } else {
            rkeep = 0;
        }

        /* Copy the rest of the string onto the end of the buffer. */
        memmove(buf + find + rkeep, buf + find + rlen, (*len) - find - rlen);

        *len -= rlen - rkeep;

        /* start our find one character past what we just replaced. */
        find = find_obs_fold(buf + find + rkeep, (*len) - find);
    }
}


static const unsigned short buf2status_code(const char *buf, const int len)
{
    char s[4];
    unsigned long int code;

    strncpy(s, buf, 3);
    s[3] = '\0';

    code = strtoul(s, NULL, 10);

    return (const unsigned short)code;
}



#line 400 "http11/c/http11.rl"




#line 136 "http11/c/http11.c"
static const int http_parser_start = 1;
static const int http_parser_first_final = 46;
static const int http_parser_error = 0;

static const int http_parser_en_main = 1;


#line 404 "http11/c/http11.rl"


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
    
#line 443 "http11/c/http11.rl"
    
#line 186 "http11/c/http11.c"
	{
	 parser->state->cs = http_parser_start;
	}

#line 444 "http11/c/http11.rl"

    parser->state->mark = -1;
    parser->state->method = -1;
    parser->state->method_len = 0;
    parser->state->uri = -1;
    parser->state->uri_len = -1;
    parser->state->http_version = -1;
    parser->state->http_version_len = -1;
    parser->state->status_code = -1;
    parser->state->status_code_len = -1;
    parser->state->reason_phrase = -1;
    parser->state->reason_phrase_len = -1;
    parser->state->field_name = -1;
    parser->state->field_name_len = -1;
    parser->state->field_value = -1;
    parser->state->field_value_len = -1;

    parser->state->tmplen = 0;

    if (parser->state->tmp != NULL) {
        free(parser->state->tmp);
        parser->state->tmp = NULL;
    }

    parser->finished = false;
    parser->error = 0;
    parser->type = -1;
}


size_t HTTPParser_execute(HTTPParser *parser,
                          const char *buf,
                          size_t offset,
                          size_t length)
{
    char *rtmp;
    char *field_value;
    int field_value_len;

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

    
#line 530 "http11/c/http11.rl"
    
#line 280 "http11/c/http11.c"
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
		case 46: goto st46;
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
		case 45: goto st45;
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
		case 10: goto tr0;
		case 13: goto tr2;
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
tr20:
#line 347 "http11/c/http11.rl"
	{
        parser->error = EBADVERSION;
    }
	goto st0;
#line 379 "http11/c/http11.c"
st0:
 parser->state->cs = 0;
	goto _out;
tr0:
#line 130 "http11/c/http11.rl"
	{
        parser->state->mark = p - buf;
    }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 393 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st2;
		case 13: goto st3;
		case 33: goto tr7;
		case 124: goto tr7;
		case 126: goto tr7;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr7;
		} else if ( (*p) >= 35 )
			goto tr7;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr7;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr7;
		} else
			goto tr7;
	} else
		goto tr7;
	goto st0;
tr2:
#line 130 "http11/c/http11.rl"
	{
        parser->state->mark = p - buf;
    }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 429 "http11/c/http11.c"
	if ( (*p) == 10 )
		goto st2;
	goto st0;
tr3:
#line 130 "http11/c/http11.rl"
	{
        parser->state->mark = p - buf;
    }
#line 134 "http11/c/http11.rl"
	{
        parser->state->method = MARK_START(p, buf, parser->state->mark);
    }
	goto st4;
tr7:
#line 134 "http11/c/http11.rl"
	{
        parser->state->method = MARK_START(p, buf, parser->state->mark);
    }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 453 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto tr8;
		case 32: goto tr8;
		case 33: goto st4;
		case 124: goto st4;
		case 126: goto st4;
	}
	if ( (*p) < 45 ) {
		if ( (*p) < 35 ) {
			if ( 11 <= (*p) && (*p) <= 13 )
				goto tr8;
		} else if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st4;
		} else
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
tr8:
#line 138 "http11/c/http11.rl"
	{
        parser->state->method_len = MARK_LEN(
            p, buf,
            parser->state->method,
            parser->state->mark
        );
    }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 496 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto st5;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st5;
	goto tr10;
tr10:
#line 146 "http11/c/http11.rl"
	{
        parser->state->uri = MARK_START(p, buf, parser->state->mark);
    }
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
#line 514 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto tr13;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto tr13;
	goto st6;
tr13:
#line 150 "http11/c/http11.rl"
	{
        parser->state->uri_len = MARK_LEN(
            p, buf,
            parser->state->uri,
            parser->state->mark
        );
    }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 536 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto st7;
		case 72: goto tr15;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st7;
	goto st6;
tr15:
#line 158 "http11/c/http11.rl"
	{
        parser->state->http_version = MARK_START(p, buf, parser->state->mark);
    }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 555 "http11/c/http11.c"
	if ( (*p) == 84 )
		goto st9;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 84 )
		goto st10;
	goto st0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	if ( (*p) == 80 )
		goto st11;
	goto st0;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	if ( (*p) == 47 )
		goto st12;
	goto st0;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	if ( (*p) == 49 )
		goto st13;
	goto tr20;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	if ( (*p) == 46 )
		goto st14;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st15;
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	switch( (*p) ) {
		case 10: goto tr24;
		case 13: goto tr25;
	}
	goto st0;
tr24:
#line 162 "http11/c/http11.rl"
	{
        parser->state->http_version_len = MARK_LEN(
            p, buf,
            parser->state->http_version,
            parser->state->mark
        );
    }
	goto st16;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
#line 624 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto tr26;
		case 13: goto tr27;
		case 33: goto tr28;
		case 124: goto tr28;
		case 126: goto tr28;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr28;
		} else if ( (*p) >= 35 )
			goto tr28;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr28;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr28;
		} else
			goto tr28;
	} else
		goto tr28;
	goto st0;
tr26:
#line 218 "http11/c/http11.rl"
	{
        /* Mark this as a request */
        parser->type = REQUEST;

        /* Actually call our callbacks. */
        if (parser->request_method != NULL) {
            parser->error = parser->request_method(
                buf + parser->state->mark + parser->state->method,
                parser->state->method_len
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        if (parser->request_uri != NULL) {
            parser->error = parser->request_uri(
                buf + parser->state->mark + parser->state->uri,
                parser->state->uri_len
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        if (parser->http_version != NULL) {
            parser->error = parser->http_version(
                buf + parser->state->mark + parser->state->http_version,
                parser->state->http_version_len
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        parser->state->mark = -1;
    }
#line 351 "http11/c/http11.rl"
	{
        {p++;  parser->state->cs = 46; goto _out;}
    }
	goto st46;
tr29:
#line 351 "http11/c/http11.rl"
	{
        {p++;  parser->state->cs = 46; goto _out;}
    }
	goto st46;
tr46:
#line 298 "http11/c/http11.rl"
	{
        if (parser->http_header != NULL) {
            if (find_obs_fold(
                    buf + parser->state->mark + parser->state->field_value,
                    parser->state->field_value_len) >= 0) {

                field_value = malloc(parser->state->field_value_len);
                if (field_value == NULL) {
                    parser->error = ENOMEM;
                    { parser->state->cs = (http_parser_error); goto _again;}
                }
                memcpy(
                    field_value,
                    buf + parser->state->mark + parser->state->field_value,
                    parser->state->field_value_len
                );

                field_value_len = parser->state->field_value_len;

                collapse_obs_fold(field_value, &field_value_len);

                parser->error = parser->http_header(
                    buf + parser->state->mark + parser->state->field_name,
                    parser->state->field_name_len,
                    field_value,
                    field_value_len
                );

                free(field_value);
            } else {
                parser->error = parser->http_header(
                    buf + parser->state->mark + parser->state->field_name,
                    parser->state->field_name_len,
                    buf + parser->state->mark + parser->state->field_value,
                    parser->state->field_value_len
                );
            }

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        parser->state->field_name = -1;
        parser->state->field_name_len = -1;
        parser->state->field_value = -1;
        parser->state->field_value_len = -1;
        parser->state->mark = -1;
    }
#line 351 "http11/c/http11.rl"
	{
        {p++;  parser->state->cs = 46; goto _out;}
    }
	goto st46;
tr74:
#line 256 "http11/c/http11.rl"
	{
        /* Mark this as a response */
        parser->type = RESPONSE;

        /* Actually call our callbacks. */
        if (parser->http_version != NULL) {
            parser->error = parser->http_version(
                buf + parser->state->mark + parser->state->http_version,
                parser->state->http_version_len
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        if (parser->status_code != NULL) {
            parser->error = parser->status_code(
                buf2status_code(
                    buf + parser->state->mark + parser->state->status_code,
                    parser->state->status_code_len
                )
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        if (parser->reason_phrase != NULL
                && parser->state->reason_phrase != -1
                && parser->state->reason_phrase_len != -1) {
            parser->error = parser->reason_phrase(
                buf + parser->state->mark + parser->state->reason_phrase,
                parser->state->reason_phrase_len
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        parser->state->mark = -1;
    }
#line 351 "http11/c/http11.rl"
	{
        {p++;  parser->state->cs = 46; goto _out;}
    }
	goto st46;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
#line 807 "http11/c/http11.c"
	goto st0;
tr27:
#line 218 "http11/c/http11.rl"
	{
        /* Mark this as a request */
        parser->type = REQUEST;

        /* Actually call our callbacks. */
        if (parser->request_method != NULL) {
            parser->error = parser->request_method(
                buf + parser->state->mark + parser->state->method,
                parser->state->method_len
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        if (parser->request_uri != NULL) {
            parser->error = parser->request_uri(
                buf + parser->state->mark + parser->state->uri,
                parser->state->uri_len
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        if (parser->http_version != NULL) {
            parser->error = parser->http_version(
                buf + parser->state->mark + parser->state->http_version,
                parser->state->http_version_len
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        parser->state->mark = -1;
    }
	goto st17;
tr47:
#line 298 "http11/c/http11.rl"
	{
        if (parser->http_header != NULL) {
            if (find_obs_fold(
                    buf + parser->state->mark + parser->state->field_value,
                    parser->state->field_value_len) >= 0) {

                field_value = malloc(parser->state->field_value_len);
                if (field_value == NULL) {
                    parser->error = ENOMEM;
                    { parser->state->cs = (http_parser_error); goto _again;}
                }
                memcpy(
                    field_value,
                    buf + parser->state->mark + parser->state->field_value,
                    parser->state->field_value_len
                );

                field_value_len = parser->state->field_value_len;

                collapse_obs_fold(field_value, &field_value_len);

                parser->error = parser->http_header(
                    buf + parser->state->mark + parser->state->field_name,
                    parser->state->field_name_len,
                    field_value,
                    field_value_len
                );

                free(field_value);
            } else {
                parser->error = parser->http_header(
                    buf + parser->state->mark + parser->state->field_name,
                    parser->state->field_name_len,
                    buf + parser->state->mark + parser->state->field_value,
                    parser->state->field_value_len
                );
            }

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        parser->state->field_name = -1;
        parser->state->field_name_len = -1;
        parser->state->field_value = -1;
        parser->state->field_value_len = -1;
        parser->state->mark = -1;
    }
	goto st17;
tr75:
#line 256 "http11/c/http11.rl"
	{
        /* Mark this as a response */
        parser->type = RESPONSE;

        /* Actually call our callbacks. */
        if (parser->http_version != NULL) {
            parser->error = parser->http_version(
                buf + parser->state->mark + parser->state->http_version,
                parser->state->http_version_len
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        if (parser->status_code != NULL) {
            parser->error = parser->status_code(
                buf2status_code(
                    buf + parser->state->mark + parser->state->status_code,
                    parser->state->status_code_len
                )
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        if (parser->reason_phrase != NULL
                && parser->state->reason_phrase != -1
                && parser->state->reason_phrase_len != -1) {
            parser->error = parser->reason_phrase(
                buf + parser->state->mark + parser->state->reason_phrase,
                parser->state->reason_phrase_len
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        parser->state->mark = -1;
    }
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 948 "http11/c/http11.c"
	if ( (*p) == 10 )
		goto tr29;
	goto st0;
tr28:
#line 218 "http11/c/http11.rl"
	{
        /* Mark this as a request */
        parser->type = REQUEST;

        /* Actually call our callbacks. */
        if (parser->request_method != NULL) {
            parser->error = parser->request_method(
                buf + parser->state->mark + parser->state->method,
                parser->state->method_len
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        if (parser->request_uri != NULL) {
            parser->error = parser->request_uri(
                buf + parser->state->mark + parser->state->uri,
                parser->state->uri_len
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        if (parser->http_version != NULL) {
            parser->error = parser->http_version(
                buf + parser->state->mark + parser->state->http_version,
                parser->state->http_version_len
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        parser->state->mark = -1;
    }
#line 130 "http11/c/http11.rl"
	{
        parser->state->mark = p - buf;
    }
#line 194 "http11/c/http11.rl"
	{
        parser->state->field_name = MARK_START(p, buf, parser->state->mark);
    }
	goto st18;
tr48:
#line 298 "http11/c/http11.rl"
	{
        if (parser->http_header != NULL) {
            if (find_obs_fold(
                    buf + parser->state->mark + parser->state->field_value,
                    parser->state->field_value_len) >= 0) {

                field_value = malloc(parser->state->field_value_len);
                if (field_value == NULL) {
                    parser->error = ENOMEM;
                    { parser->state->cs = (http_parser_error); goto _again;}
                }
                memcpy(
                    field_value,
                    buf + parser->state->mark + parser->state->field_value,
                    parser->state->field_value_len
                );

                field_value_len = parser->state->field_value_len;

                collapse_obs_fold(field_value, &field_value_len);

                parser->error = parser->http_header(
                    buf + parser->state->mark + parser->state->field_name,
                    parser->state->field_name_len,
                    field_value,
                    field_value_len
                );

                free(field_value);
            } else {
                parser->error = parser->http_header(
                    buf + parser->state->mark + parser->state->field_name,
                    parser->state->field_name_len,
                    buf + parser->state->mark + parser->state->field_value,
                    parser->state->field_value_len
                );
            }

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        parser->state->field_name = -1;
        parser->state->field_name_len = -1;
        parser->state->field_value = -1;
        parser->state->field_value_len = -1;
        parser->state->mark = -1;
    }
#line 130 "http11/c/http11.rl"
	{
        parser->state->mark = p - buf;
    }
#line 194 "http11/c/http11.rl"
	{
        parser->state->field_name = MARK_START(p, buf, parser->state->mark);
    }
	goto st18;
tr76:
#line 256 "http11/c/http11.rl"
	{
        /* Mark this as a response */
        parser->type = RESPONSE;

        /* Actually call our callbacks. */
        if (parser->http_version != NULL) {
            parser->error = parser->http_version(
                buf + parser->state->mark + parser->state->http_version,
                parser->state->http_version_len
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        if (parser->status_code != NULL) {
            parser->error = parser->status_code(
                buf2status_code(
                    buf + parser->state->mark + parser->state->status_code,
                    parser->state->status_code_len
                )
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        if (parser->reason_phrase != NULL
                && parser->state->reason_phrase != -1
                && parser->state->reason_phrase_len != -1) {
            parser->error = parser->reason_phrase(
                buf + parser->state->mark + parser->state->reason_phrase,
                parser->state->reason_phrase_len
            );

            if (parser->error)
                { parser->state->cs = (http_parser_error); goto _again;}
        }

        parser->state->mark = -1;
    }
#line 130 "http11/c/http11.rl"
	{
        parser->state->mark = p - buf;
    }
#line 194 "http11/c/http11.rl"
	{
        parser->state->field_name = MARK_START(p, buf, parser->state->mark);
    }
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 1115 "http11/c/http11.c"
	switch( (*p) ) {
		case 33: goto st18;
		case 58: goto tr31;
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
tr31:
#line 198 "http11/c/http11.rl"
	{
        parser->state->field_name_len = MARK_LEN(
            p, buf,
            parser->state->field_name,
            parser->state->mark
        );
    }
	goto st19;
tr33:
#line 206 "http11/c/http11.rl"
	{
        parser->state->field_value = MARK_START(p, buf, parser->state->mark);
    }
#line 210 "http11/c/http11.rl"
	{
        parser->state->field_value_len = MARK_LEN(
            p, buf,
            parser->state->field_value,
            parser->state->mark
        );
    }
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 1168 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto tr33;
		case 10: goto tr34;
		case 13: goto tr35;
		case 32: goto tr33;
		case 127: goto st0;
	}
	if ( 0 <= (*p) && (*p) <= 31 )
		goto st0;
	goto tr32;
tr32:
#line 206 "http11/c/http11.rl"
	{
        parser->state->field_value = MARK_START(p, buf, parser->state->mark);
    }
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 1189 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto tr37;
		case 10: goto tr38;
		case 13: goto tr39;
		case 32: goto tr37;
		case 127: goto st0;
	}
	if ( 0 <= (*p) && (*p) <= 31 )
		goto st0;
	goto st20;
tr37:
#line 210 "http11/c/http11.rl"
	{
        parser->state->field_value_len = MARK_LEN(
            p, buf,
            parser->state->field_value,
            parser->state->mark
        );
    }
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 1214 "http11/c/http11.c"
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
		case 9: goto tr44;
		case 10: goto tr38;
		case 13: goto tr39;
		case 32: goto tr44;
		case 127: goto st0;
	}
	if ( 0 <= (*p) && (*p) <= 31 )
		goto st0;
	goto st20;
tr44:
#line 210 "http11/c/http11.rl"
	{
        parser->state->field_value_len = MARK_LEN(
            p, buf,
            parser->state->field_value,
            parser->state->mark
        );
    }
	goto st23;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
#line 1253 "http11/c/http11.c"
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
		case 10: goto tr46;
		case 13: goto tr47;
		case 33: goto tr48;
		case 124: goto tr48;
		case 126: goto tr48;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr48;
		} else if ( (*p) >= 35 )
			goto tr48;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr48;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr48;
		} else
			goto tr48;
	} else
		goto tr48;
	goto st0;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	if ( (*p) == 10 )
		goto st24;
	goto st0;
tr34:
#line 206 "http11/c/http11.rl"
	{
        parser->state->field_value = MARK_START(p, buf, parser->state->mark);
    }
#line 210 "http11/c/http11.rl"
	{
        parser->state->field_value_len = MARK_LEN(
            p, buf,
            parser->state->field_value,
            parser->state->mark
        );
    }
	goto st26;
tr38:
#line 210 "http11/c/http11.rl"
	{
        parser->state->field_value_len = MARK_LEN(
            p, buf,
            parser->state->field_value,
            parser->state->mark
        );
    }
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
#line 1325 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto st27;
		case 10: goto tr46;
		case 13: goto tr47;
		case 32: goto st27;
		case 33: goto tr48;
		case 124: goto tr48;
		case 126: goto tr48;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr48;
		} else if ( (*p) >= 35 )
			goto tr48;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr48;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr48;
		} else
			goto tr48;
	} else
		goto tr48;
	goto st0;
tr50:
#line 210 "http11/c/http11.rl"
	{
        parser->state->field_value_len = MARK_LEN(
            p, buf,
            parser->state->field_value,
            parser->state->mark
        );
    }
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
#line 1367 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto tr50;
		case 10: goto tr38;
		case 13: goto tr39;
		case 32: goto tr50;
		case 127: goto st0;
	}
	if ( 0 <= (*p) && (*p) <= 31 )
		goto st0;
	goto st20;
tr35:
#line 206 "http11/c/http11.rl"
	{
        parser->state->field_value = MARK_START(p, buf, parser->state->mark);
    }
#line 210 "http11/c/http11.rl"
	{
        parser->state->field_value_len = MARK_LEN(
            p, buf,
            parser->state->field_value,
            parser->state->mark
        );
    }
	goto st28;
tr39:
#line 210 "http11/c/http11.rl"
	{
        parser->state->field_value_len = MARK_LEN(
            p, buf,
            parser->state->field_value,
            parser->state->mark
        );
    }
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
#line 1406 "http11/c/http11.c"
	if ( (*p) == 10 )
		goto st26;
	goto st0;
tr25:
#line 162 "http11/c/http11.rl"
	{
        parser->state->http_version_len = MARK_LEN(
            p, buf,
            parser->state->http_version,
            parser->state->mark
        );
    }
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 1424 "http11/c/http11.c"
	if ( (*p) == 10 )
		goto st16;
	goto st0;
tr4:
#line 130 "http11/c/http11.rl"
	{
        parser->state->mark = p - buf;
    }
#line 134 "http11/c/http11.rl"
	{
        parser->state->method = MARK_START(p, buf, parser->state->mark);
    }
#line 158 "http11/c/http11.rl"
	{
        parser->state->http_version = MARK_START(p, buf, parser->state->mark);
    }
	goto st30;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
#line 1446 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto tr8;
		case 32: goto tr8;
		case 33: goto st4;
		case 84: goto st31;
		case 124: goto st4;
		case 126: goto st4;
	}
	if ( (*p) < 45 ) {
		if ( (*p) < 35 ) {
			if ( 11 <= (*p) && (*p) <= 13 )
				goto tr8;
		} else if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st4;
		} else
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
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	switch( (*p) ) {
		case 9: goto tr8;
		case 32: goto tr8;
		case 33: goto st4;
		case 84: goto st32;
		case 124: goto st4;
		case 126: goto st4;
	}
	if ( (*p) < 45 ) {
		if ( (*p) < 35 ) {
			if ( 11 <= (*p) && (*p) <= 13 )
				goto tr8;
		} else if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st4;
		} else
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
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	switch( (*p) ) {
		case 9: goto tr8;
		case 32: goto tr8;
		case 33: goto st4;
		case 80: goto st33;
		case 124: goto st4;
		case 126: goto st4;
	}
	if ( (*p) < 45 ) {
		if ( (*p) < 35 ) {
			if ( 11 <= (*p) && (*p) <= 13 )
				goto tr8;
		} else if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st4;
		} else
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
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	switch( (*p) ) {
		case 9: goto tr8;
		case 32: goto tr8;
		case 33: goto st4;
		case 47: goto st34;
		case 124: goto st4;
		case 126: goto st4;
	}
	if ( (*p) < 42 ) {
		if ( (*p) > 13 ) {
			if ( 35 <= (*p) && (*p) <= 39 )
				goto st4;
		} else if ( (*p) >= 11 )
			goto tr8;
	} else if ( (*p) > 43 ) {
		if ( (*p) < 65 ) {
			if ( 45 <= (*p) && (*p) <= 57 )
				goto st4;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st4;
		} else
			goto st4;
	} else
		goto st4;
	goto st0;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	if ( (*p) == 49 )
		goto st35;
	goto tr20;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	if ( (*p) == 46 )
		goto st36;
	goto st0;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st37;
	goto st0;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
	switch( (*p) ) {
		case 9: goto tr60;
		case 32: goto tr60;
	}
	if ( 11 <= (*p) && (*p) <= 13 )
		goto tr60;
	goto st0;
tr60:
#line 162 "http11/c/http11.rl"
	{
        parser->state->http_version_len = MARK_LEN(
            p, buf,
            parser->state->http_version,
            parser->state->mark
        );
    }
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
#line 1618 "http11/c/http11.c"
	switch( (*p) ) {
		case 9: goto st38;
		case 32: goto st38;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr62;
	} else if ( (*p) >= 11 )
		goto st38;
	goto st0;
tr62:
#line 170 "http11/c/http11.rl"
	{
        parser->state->status_code = MARK_START(p, buf, parser->state->mark);
    }
	goto st39;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
#line 1639 "http11/c/http11.c"
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
		case 10: goto tr66;
		case 13: goto tr67;
		case 32: goto tr65;
	}
	if ( 9 <= (*p) && (*p) <= 12 )
		goto tr65;
	goto st0;
tr65:
#line 174 "http11/c/http11.rl"
	{
        parser->state->status_code_len = MARK_LEN(
            p, buf,
            parser->state->status_code,
            parser->state->mark
        );
    }
	goto st42;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
#line 1676 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st44;
		case 32: goto st42;
		case 127: goto st0;
	}
	if ( (*p) < 9 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto st0;
	} else if ( (*p) > 13 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto st0;
	} else
		goto st42;
	goto tr68;
tr68:
#line 182 "http11/c/http11.rl"
	{
        parser->state->reason_phrase = MARK_START(p, buf, parser->state->mark);
    }
	goto st43;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
#line 1701 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto tr72;
		case 13: goto tr73;
		case 127: goto st0;
	}
	if ( (*p) > 8 ) {
		if ( 11 <= (*p) && (*p) <= 31 )
			goto st0;
	} else if ( (*p) >= 0 )
		goto st0;
	goto st43;
tr66:
#line 174 "http11/c/http11.rl"
	{
        parser->state->status_code_len = MARK_LEN(
            p, buf,
            parser->state->status_code,
            parser->state->mark
        );
    }
	goto st44;
tr72:
#line 186 "http11/c/http11.rl"
	{
        parser->state->reason_phrase_len = MARK_LEN(
            p, buf,
            parser->state->reason_phrase,
            parser->state->mark
        );
    }
	goto st44;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
#line 1737 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto tr74;
		case 13: goto tr75;
		case 33: goto tr76;
		case 124: goto tr76;
		case 126: goto tr76;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr76;
		} else if ( (*p) >= 35 )
			goto tr76;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr76;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr76;
		} else
			goto tr76;
	} else
		goto tr76;
	goto st0;
tr67:
#line 174 "http11/c/http11.rl"
	{
        parser->state->status_code_len = MARK_LEN(
            p, buf,
            parser->state->status_code,
            parser->state->mark
        );
    }
	goto st45;
tr73:
#line 186 "http11/c/http11.rl"
	{
        parser->state->reason_phrase_len = MARK_LEN(
            p, buf,
            parser->state->reason_phrase,
            parser->state->mark
        );
    }
	goto st45;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
#line 1787 "http11/c/http11.c"
	if ( (*p) == 10 )
		goto st44;
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
	_test_eof15:  parser->state->cs = 15; goto _test_eof; 
	_test_eof16:  parser->state->cs = 16; goto _test_eof; 
	_test_eof46:  parser->state->cs = 46; goto _test_eof; 
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
	_test_eof45:  parser->state->cs = 45; goto _test_eof; 

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
	case 8: 
	case 9: 
	case 10: 
	case 11: 
	case 13: 
	case 14: 
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
	case 30: 
	case 31: 
	case 32: 
	case 33: 
	case 35: 
	case 36: 
	case 37: 
	case 38: 
	case 39: 
	case 40: 
	case 41: 
	case 42: 
	case 43: 
	case 44: 
	case 45: 
#line 355 "http11/c/http11.rl"
	{
        parser->error = EEOF;
        { parser->state->cs = (http_parser_error); goto _again;}
    }
	break;
	case 12: 
	case 34: 
#line 347 "http11/c/http11.rl"
	{
        parser->error = EBADVERSION;
    }
#line 355 "http11/c/http11.rl"
	{
        parser->error = EEOF;
        { parser->state->cs = (http_parser_error); goto _again;}
    }
	break;
#line 1904 "http11/c/http11.c"
	}
	}

	_out: {}
	}

#line 531 "http11/c/http11.rl"

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
        parser->state->tmplen = pe - buf - parser->state->mark;

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
            buf + parser->state->mark,
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
