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


%%{
    machine http_parser;

    action mark {
        parser->state->mark = fpc - buf;
    }

    action method {
        parser->state->method = MARK_START(fpc, buf, parser->state->mark);
    }

    action method_len {
        parser->state->method_len = MARK_LEN(
            fpc, buf,
            parser->state->method,
            parser->state->mark
        );
    }

    action uri {
        parser->state->uri = MARK_START(fpc, buf, parser->state->mark);
    }

    action uri_len {
        parser->state->uri_len = MARK_LEN(
            fpc, buf,
            parser->state->uri,
            parser->state->mark
        );
    }

    action http_version {
        parser->state->http_version = MARK_START(fpc, buf, parser->state->mark);
    }

    action http_version_len {
        parser->state->http_version_len = MARK_LEN(
            fpc, buf,
            parser->state->http_version,
            parser->state->mark
        );
    }

    action status_code {
        parser->state->status_code = MARK_START(fpc, buf, parser->state->mark);
    }

    action status_code_len {
        parser->state->status_code_len = MARK_LEN(
            fpc, buf,
            parser->state->status_code,
            parser->state->mark
        );
    }

    action reason_phrase {
        parser->state->reason_phrase = MARK_START(fpc, buf, parser->state->mark);
    }

    action reason_phrase_len {
        parser->state->reason_phrase_len = MARK_LEN(
            fpc, buf,
            parser->state->reason_phrase,
            parser->state->mark
        );
    }

    action field_name {
        parser->state->field_name = MARK_START(fpc, buf, parser->state->mark);
    }

    action field_name_len {
        parser->state->field_name_len = MARK_LEN(
            fpc, buf,
            parser->state->field_name,
            parser->state->mark
        );
    }

    action field_value {
        parser->state->field_value = MARK_START(fpc, buf, parser->state->mark);
    }

    action field_value_len {
        parser->state->field_value_len = MARK_LEN(
            fpc, buf,
            parser->state->field_value,
            parser->state->mark
        );
    }

    action request_line {
        if (parser->request_method != NULL) {
            parser->error = parser->request_method(
                buf + parser->state->mark + parser->state->method,
                parser->state->method_len
            );

            if (parser->error)
                fgoto *http_parser_error;
        }

        if (parser->request_uri != NULL) {
            parser->error = parser->request_uri(
                buf + parser->state->mark + parser->state->uri,
                parser->state->uri_len
            );

            if (parser->error)
                fgoto *http_parser_error;
        }

        if (parser->http_version != NULL) {
            parser->error = parser->http_version(
                buf + parser->state->mark + parser->state->http_version,
                parser->state->http_version_len
            );

            if (parser->error)
                fgoto *http_parser_error;
        }

        parser->state->mark = -1;
    }

    action status_line {
        if (parser->http_version != NULL) {
            parser->error = parser->http_version(
                buf + parser->state->mark + parser->state->http_version,
                parser->state->http_version_len
            );

            if (parser->error)
                fgoto *http_parser_error;
        }

        if (parser->status_code != NULL) {
            parser->error = parser->status_code(
                buf2status_code(
                    buf + parser->state->mark + parser->state->status_code,
                    parser->state->status_code_len
                )
            );

            if (parser->error)
                fgoto *http_parser_error;
        }

        if (parser->reason_phrase != NULL
                && parser->state->reason_phrase != -1
                && parser->state->reason_phrase_len != -1) {
            parser->error = parser->reason_phrase(
                buf + parser->state->mark + parser->state->reason_phrase,
                parser->state->reason_phrase_len
            );

            if (parser->error)
                fgoto *http_parser_error;
        }

        parser->state->mark = -1;
    }

    action header_field {
        if (parser->http_header != NULL) {
            if (find_obs_fold(
                    buf + parser->state->mark + parser->state->field_value,
                    parser->state->field_value_len) >= 0) {

                field_value = malloc(parser->state->field_value_len);
                if (field_value == NULL) {
                    parser->error = ENOMEM;
                    fgoto *http_parser_error;
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
                fgoto *http_parser_error;
        }

        parser->state->field_name = -1;
        parser->state->field_name_len = -1;
        parser->state->field_value = -1;
        parser->state->field_value_len = -1;
        parser->state->mark = -1;
    }

    action invalid_http_version {
        parser->error = EBADVERSION;
    }

    action done {
        fbreak;
    }

    action eof_received {
        parser->error = EEOF;
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

    method = token >method %method_len ;
    request_target = ( any -- CRLF )+ >uri %uri_len ;
    http_major_version = "1" >lerr(invalid_http_version) ;
    http_version = ( "HTTP" "/" http_major_version "." digit ) >http_version %http_version_len ;
    status_code = digit{3} >status_code %status_code_len ;
    reason_phrase = ( HTAB | SP | VCHAR | obs_text )* >reason_phrase %reason_phrase_len ;

    request_line = ( CRLF* method SP request_target SP http_version CRLF ) >mark %request_line ;
    status_line = ( http_version SP status_code ( SP reason_phrase )? CRLF ) >mark %status_line ;

    field_name = token >field_name %field_name_len ;
    field_vchar = ( VCHAR | obs_text ) ;
    field_content = field_vchar ( ( SP | HTAB )+ field_vchar )? ;
    field_value = ( field_content | obs_fold )* >field_value %field_value_len ;
    header_field = ( field_name ":" OWS field_value OWS CRLF ) >mark %header_field ;

    http_message = ( request_line | status_line ) header_field* CRLF ;

main := http_message @done @eof(eof_received);

}%%


%% write data;


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
    %% access parser->state->;
    %% write init;

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

    %% access parser->state->;
    %% write exec;

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
