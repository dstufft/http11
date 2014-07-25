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

#ifndef _http11_h
#define _http11_h

typedef enum {false, true} bool;

typedef struct _HTTPParserState *HTTPParserState;

typedef int (*element_cb)(const char *buf, size_t length);

typedef struct HTTPParser {
  /* Public State */
  bool finished;
  int error;

  /* Callback Methods */
  element_cb request_method;
  element_cb request_uri;

  /* Internal state */
  HTTPParserState state;

} HTTPParser;

HTTPParser *HTTPParser_create();
void HTTPParser_init(HTTPParser *parser);
size_t HTTPParser_execute(HTTPParser *parser, const char *data, size_t len, size_t off);
void HTTPParser_destroy(HTTPParser *parser);

#endif
