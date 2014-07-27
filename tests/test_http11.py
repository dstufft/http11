# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import ast
import glob
import os.path

import pytest

import http11


def _load_cases():
    cases = glob.iglob(
        os.path.join(
            os.path.dirname(os.path.abspath(__file__)),
            "cases",
            "*.py",
        )
    )

    for casefile in cases:
        with open(casefile) as fp:
            for case in ast.literal_eval(fp.read()):
                yield case["message"], case["expected"]


@pytest.mark.parametrize(("message", "expected"), _load_cases())
def test_parsing(parser, data, message, expected):
    if not isinstance(message, list):
        message = [message]

    for chunk in message:
        http11.lib.HTTPParser_execute(parser, chunk, len(chunk), 0)

    assert data == expected
