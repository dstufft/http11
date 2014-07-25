
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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "http11.h"

int calculate_offset(const char *fpc, const char *buf) {
    return fpc - buf;
}

int calculate_length(const char *fpc, const char *buf, const int offset) {
    return fpc - buf - offset;
}

const char *create_pointer(const char *buf, const int offset) {
    return buf + offset;
}



#line 66 "http11/c/http11.rl"




#line 42 "http11/c/http11.c"
static const int http_parser_start = 1;
static const int http_parser_first_final = 17;
static const int http_parser_error = 0;

static const int http_parser_en_main = 1;


#line 70 "http11/c/http11.rl"

struct _HTTPParserState {
  int cs;
  int mark;
};


HTTPParser *HTTPParser_create() {
    HTTPParser *parser = malloc(sizeof(*parser));

    if (!parser) {
        return NULL;
    }

    parser->state = malloc(sizeof *parser->state);

    if (!parser->state) {
        HTTPParser_destroy(parser);
        return NULL;
    }

    return parser;
}


void HTTPParser_init(HTTPParser *parser) {
    
#line 97 "http11/c/http11.rl"
    
#line 80 "http11/c/http11.c"
	{
	 parser->state->cs = http_parser_start;
	}

#line 98 "http11/c/http11.rl"

    parser->state->mark = 0;

    parser->finished = false;
    parser->error = 0;
}

size_t HTTPParser_execute(HTTPParser *parser, const char *data, size_t len, size_t off) {
    const char *p = data + off;
    const char *pe = data + len;

    
#line 110 "http11/c/http11.rl"
    
#line 100 "http11/c/http11.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch (  parser->state->cs )
	{
case 1:
	switch( (*p) ) {
		case 33: goto tr0;
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
#line 36 "http11/c/http11.rl"
	{
        parser->state->mark = calculate_offset(p, data);
    }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 143 "http11/c/http11.c"
	switch( (*p) ) {
		case 32: goto tr2;
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
tr2:
#line 40 "http11/c/http11.rl"
	{
        if (parser->request_method != NULL) {
            parser->request_method(
                create_pointer(data, parser->state->mark),
                calculate_length(p, data, parser->state->mark)
            );
        }
    }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 183 "http11/c/http11.c"
	if ( (*p) == 10 )
		goto st0;
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto st5;
	}
	goto st4;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto st5;
		case 72: goto st6;
	}
	goto st4;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto st5;
		case 84: goto st7;
	}
	goto st4;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto st5;
		case 84: goto st8;
	}
	goto st4;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto st5;
		case 80: goto st9;
	}
	goto st4;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto st5;
		case 47: goto st10;
	}
	goto st4;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto st5;
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
		case 32: goto st5;
		case 46: goto st12;
	}
	goto st4;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto st5;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st13;
	goto st4;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	switch( (*p) ) {
		case 10: goto st14;
		case 13: goto st16;
		case 32: goto st5;
	}
	goto st4;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 10: goto st17;
		case 13: goto st15;
	}
	goto st0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( (*p) == 10 )
		goto st17;
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	switch( (*p) ) {
		case 10: goto st14;
		case 32: goto st5;
	}
	goto st4;
	}
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
	_test_eof17:  parser->state->cs = 17; goto _test_eof; 
	_test_eof15:  parser->state->cs = 15; goto _test_eof; 
	_test_eof16:  parser->state->cs = 16; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 111 "http11/c/http11.rl"

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
