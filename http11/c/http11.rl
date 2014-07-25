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


static void handle_callback(HTTPParser *parser,
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


%%{
    machine http_parser;

    action mark {
        parser->state->mark = calculate_offset(fpc, buf);
    }

    action request_method {
        handle_callback(parser, fpc, buf, parser->request_method);

        if (parser->error)
            fgoto *http_parser_error;
    }

    action request_uri {
        handle_callback(parser, fpc, buf, parser->request_uri);

        if (parser->error)
            fgoto *http_parser_error;
    }

    action http_version {
        handle_callback(parser, fpc, buf, parser->http_version);

        if (parser->error)
            fgoto *http_parser_error;
    }

    CRLF = ( "\r\n" | "\n" ) ;
    SP = " " ;

    tchar = ( "!" | "#" | "$" | "%" | "&" | "'" | "*" | "+" | "-" | "." | "^" |
              "_" | "`" | "|" | "~" | digit | alpha ) ;
    token = tchar+ ;


    method = token >mark %request_method ;
    request_target = ( any -- CRLF )+ >mark %request_uri ;
    http_version = ( "HTTP" "/" digit "." digit ) >mark %request_uri ;

    request_line = method SP request_target SP http_version CRLF ;
    http_message = ( request_line ) CRLF ;

main := http_message;

}%%


%% write data;


HTTPParser *HTTPParser_create() {
    HTTPParser *parser = malloc(sizeof(*parser));

    if (!parser)
        goto error;

    parser->state = malloc(sizeof *parser->state);

    if (!parser->state)
        goto error;

    return parser;

    error:
        HTTPParser_destroy(parser);
        return NULL;
}


void HTTPParser_init(HTTPParser *parser) {
    %% access parser->state->;
    %% write init;

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
