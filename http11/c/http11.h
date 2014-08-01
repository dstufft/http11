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
#include <errno.h>

#ifndef _http11_h
#define _http11_h


#define EEOF        -2 /* An EOF was received prior to the message being
                          finished. */
#define EINVALIDMSG -3 /* The message was invalid an unable to be parsed */
#define EBADVERSION -4 /* The HTTP version was invalid */


typedef enum {false, true} bool;

typedef struct _HTTPParserState *HTTPParserState;

typedef enum {REQUEST, RESPONSE} http_msg_type;

typedef int (*element_cb)(const char *buf, size_t length);

typedef int (*status_code_cb)(const unsigned short status_code);

typedef int (*header_cb)(const char *name,
                         size_t namelen,
                         const char *value,
                         size_t valuelen);

typedef struct HTTPParser {
    /* Public State */
    bool finished;
    int error;

    http_msg_type type;

    /* Callback Methods */
    element_cb request_method;
    element_cb request_uri;
    element_cb http_version;
    element_cb reason_phrase;

    status_code_cb status_code;

    header_cb http_header;

    /* Internal state */
    HTTPParserState state;

} HTTPParser;

HTTPParser *HTTPParser_create();
void HTTPParser_init(HTTPParser *parser);
size_t HTTPParser_execute(HTTPParser *parser,
                          const char *buf,
                          size_t offset,
                          size_t length);
void HTTPParser_destroy(HTTPParser *parser);

#endif
