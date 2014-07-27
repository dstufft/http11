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
    char s[STATUS_CODE_LEN + 1];
    unsigned long int code;

    if (callback != NULL) {
        strncpy(s, create_pointer(buf, parser->state->mark), STATUS_CODE_LEN);
        s[STATUS_CODE_LEN] = '\0';

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
                parser->error = 1;
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
}


%%{
    machine http_parser;

    action mark {
        parser->state->mark = calculate_offset(fpc, buf);
    }

    action header_name_start {
        parser->state->header_name_start = calculate_offset(fpc, buf);
    }

    action header_name_end {
        parser->state->header_name_end = calculate_offset(fpc, buf);
    }

    action header_value_start {
        parser->state->header_value_start = calculate_offset(fpc, buf);
    }

    action header_value_end {
        parser->state->header_value_end = calculate_offset(fpc, buf);
    }

    action request_method {
        handle_element_callback(parser, fpc, buf, parser->request_method);

        if (parser->error)
            fgoto *http_parser_error;
    }

    action request_uri {
        handle_element_callback(parser, fpc, buf, parser->request_uri);

        if (parser->error)
            fgoto *http_parser_error;
    }

    action http_version {
        handle_element_callback(parser, fpc, buf, parser->http_version);

        if (parser->error)
            fgoto *http_parser_error;
    }

    action reason_phrase {
        handle_element_callback(parser, fpc, buf, parser->reason_phrase);

        if (parser->error)
            fgoto *http_parser_error;
    }

    action status_code {
        handle_status_code_callback(parser, fpc, buf, parser->status_code);

        if (parser->error)
            fgoto *http_parser_error;
    }

    action write_header {
        handle_header_callback(parser, buf);

        if (parser->error)
            fgoto *http_parser_error;
    }

    CRLF = ( "\r\n" | "\n" ) ;
    SP = " " ;
    VCHAR = graph ;
    HTAB = "\t" ;
    OWS = ( SP | HTAB )* ;

    tchar = ( "!" | "#" | "$" | "%" | "&" | "'" | "*" | "+" | "-" | "." | "^" |
              "_" | "`" | "|" | "~" | digit | alpha ) ;
    token = tchar+ ;
    obs_text = 0x80..0xFF ;
    obs_fold = CRLF ( SP | HTAB )+ ;

    method = token >mark %request_method ;
    request_target = ( any -- CRLF )+ >mark %request_uri ;
    http_version = ( "HTTP" "/" digit "." digit ) >mark %http_version ;
    status_code = digit{3} >mark %status_code ;
    reason_phrase = ( HTAB | SP | VCHAR | obs_text )* >mark %reason_phrase ;

    request_line = method SP request_target SP http_version CRLF ;
    status_line = http_version SP status_code SP reason_phrase CRLF ;
    start_line = ( request_line | status_line ) ;

    field_name = token >header_name_start %header_name_end ;
    field_vchar = ( VCHAR | obs_text ) ;
    field_content = field_vchar ( ( SP | HTAB )+ field_vchar )? ;
    field_value = ( field_content | obs_fold )* >header_value_start
                                                %header_value_end ;
    header_field = field_name ":" OWS field_value OWS CRLF %write_header ;

    http_message = start_line header_field* CRLF ;

main := http_message;

}%%


%% write data;


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
    %% access parser->state->;
    %% write init;

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
