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

typedef void (*element_cb)(const char *buf, size_t len);

typedef struct http_parser {
  /* Public State */
  bool finished;
  int error;

  /* Callback Methods */
  element_cb request_method;

  /* Internal State */
  int cs;
  int mark;

} http_parser;

void http_parser_init(http_parser *parser);
size_t http_parser_execute(http_parser *parser, const char *data, size_t len, size_t off);

#endif
