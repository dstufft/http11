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


%%{
    machine http_parser;

    action mark {
        parser->state->mark = calculate_offset(fpc, data);
    }

    action request_method {
        if (parser->request_method != NULL) {
            parser->request_method(
                create_pointer(data, parser->state->mark),
                calculate_length(fpc, data, parser->state->mark)
            );
        }
    }

    CRLF = ( "\r\n" | "\n" ) ;
    SP = " " ;

    tchar = ( "!" | "#" | "$" | "%" | "&" | "'" | "*" | "+" | "-" | "." | "^" |
              "_" | "`" | "|" | "~" | digit | alpha ) ;
    token = tchar+ ;


    method = token >mark %request_method ;
    request_target = ( any -- CRLF )+ ;
    http_version = "HTTP" "/" digit "." digit ;

    request_line = method SP request_target SP http_version CRLF ;
    http_message = ( request_line ) CRLF ;

main := http_message;

}%%


%% write data;

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
    %% access parser->state->;
    %% write init;

    parser->state->mark = 0;

    parser->finished = false;
    parser->error = 0;
}

size_t HTTPParser_execute(HTTPParser *parser, const char *data, size_t len, size_t off) {
    const char *p = data + off;
    const char *pe = data + len;

    %% access parser->state->;
    %% write exec;

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
