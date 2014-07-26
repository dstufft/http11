
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



#line 150 "http11/c/http11.rl"




#line 95 "http11/c/http11.c"
static const int http_parser_start = 1;
static const int http_parser_first_final = 32;
static const int http_parser_error = 0;

static const int http_parser_en_main = 1;


#line 154 "http11/c/http11.rl"


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

    return parser;

    error:
        HTTPParser_destroy(parser);
        return NULL;
}


void HTTPParser_init(HTTPParser *parser) {
    
#line 186 "http11/c/http11.rl"
    
#line 138 "http11/c/http11.c"
	{
	 parser->state->cs = http_parser_start;
	}

#line 187 "http11/c/http11.rl"

    parser->state->mark = 0;

    parser->finished = false;
    parser->error = 0;
}


size_t HTTPParser_execute(HTTPParser *parser,
                          const char *buf,
                          size_t length,
                          size_t offset) {
    const char *p = buf + offset;
    const char *pe = buf + length;

    
#line 203 "http11/c/http11.rl"
    
#line 162 "http11/c/http11.c"
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
		case 32: goto st32;
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
#line 89 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 252 "http11/c/http11.c"
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
#line 93 "http11/c/http11.rl"
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
#line 290 "http11/c/http11.c"
	if ( (*p) == 10 )
		goto st0;
	goto tr5;
tr5:
#line 89 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 304 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto tr7;
	}
	goto st4;
tr7:
#line 100 "http11/c/http11.rl"
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
#line 323 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto tr7;
		case 72: goto tr8;
	}
	goto st4;
tr8:
#line 89 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
#line 340 "http11/c/http11.c"
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
#line 107 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->http_version);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st14;
tr34:
#line 89 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
#line 121 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->reason_phrase);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st14;
tr37:
#line 121 "http11/c/http11.rl"
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
#line 454 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st32;
		case 13: goto st15;
	}
	goto st0;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( (*p) == 10 )
		goto st32;
	goto st0;
tr17:
#line 107 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->http_version);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st16;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
#line 485 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto st14;
		case 32: goto tr7;
	}
	goto st4;
tr2:
#line 89 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 501 "http11/c/http11.c"
	switch( (*p) ) {
		case 32: goto tr3;
		case 33: goto st2;
		case 84: goto st18;
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
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	switch( (*p) ) {
		case 32: goto tr3;
		case 33: goto st2;
		case 84: goto st19;
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
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	switch( (*p) ) {
		case 32: goto tr3;
		case 33: goto st2;
		case 80: goto st20;
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
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	switch( (*p) ) {
		case 32: goto tr3;
		case 33: goto st2;
		case 47: goto st21;
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
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st22;
	goto st0;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	if ( (*p) == 46 )
		goto st23;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st24;
	goto st0;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	if ( (*p) == 32 )
		goto tr28;
	goto st0;
tr28:
#line 107 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->http_version);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st25;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
#line 652 "http11/c/http11.c"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr29;
	goto st0;
tr29:
#line 89 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
#line 666 "http11/c/http11.c"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st27;
	goto st0;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st28;
	goto st0;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	if ( (*p) == 32 )
		goto tr32;
	goto st0;
tr32:
#line 114 "http11/c/http11.rl"
	{
        handle_status_code_callback(parser, p, buf, parser->status_code);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 697 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto tr34;
		case 13: goto tr35;
		case 127: goto st0;
	}
	if ( (*p) > 8 ) {
		if ( 11 <= (*p) && (*p) <= 31 )
			goto st0;
	} else if ( (*p) >= 0 )
		goto st0;
	goto tr33;
tr33:
#line 89 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
	goto st30;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
#line 719 "http11/c/http11.c"
	switch( (*p) ) {
		case 10: goto tr37;
		case 13: goto tr38;
		case 127: goto st0;
	}
	if ( (*p) > 8 ) {
		if ( 11 <= (*p) && (*p) <= 31 )
			goto st0;
	} else if ( (*p) >= 0 )
		goto st0;
	goto st30;
tr35:
#line 89 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, buf);
    }
#line 121 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->reason_phrase);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st31;
tr38:
#line 121 "http11/c/http11.rl"
	{
        handle_element_callback(parser, p, buf, parser->reason_phrase);

        if (parser->error)
            { parser->state->cs = (http_parser_error); goto _again;}
    }
	goto st31;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
#line 757 "http11/c/http11.c"
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
	_test_eof32:  parser->state->cs = 32; goto _test_eof; 
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

	_test_eof: {}
	_out: {}
	}

#line 204 "http11/c/http11.rl"

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
