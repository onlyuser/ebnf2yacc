ebnf2yacc
=========

Copyright (C) 2011-2013 Jerry Chen <mailto:onlyuser@gmail.com>

About:
------

ebnf2yacc is a kleene closure preprocessor for yacc.

Requirements:
-------------

Unix tools and 3rd party components (accessible from $PATH):

    gcc (with -std=c++0x support), flex, bison, valgrind, cppcheck, doxygen, graphviz, ticpp

**Environment variables:**

* $EXTERN_INCLUDE_PATH -- where "ticpp/ticpp.h" resides
* $EXTERN_LIB_PATH     -- where "libticppd.a" resides

Make targets:
-------------

    all, test, pure, dot, lint, doc, xml, import, clean.

<table>
    <tr><th> target </th><th> action                                                </th></tr>
    <tr><td> all    </td><td> make binaries                                         </td></tr>
    <tr><td> test   </td><td> all + run tests                                       </td></tr>
    <tr><td> pure   </td><td> test + use valgrind to check for memory leaks         </td></tr>
    <tr><td> dot    </td><td> test + generate .png graph for tests                  </td></tr>
    <tr><td> lint   </td><td> use cppcheck to perform static analysis on .cpp files </td></tr>
    <tr><td> doc    </td><td> use doxygen to generate documentation                 </td></tr>
    <tr><td> xml    </td><td> test + generate .xml for tests                        </td></tr>
    <tr><td> import </td><td> test + use ticpp to serialize-to/deserialize-from xml </td></tr>
    <tr><td> clean  </td><td> remove all intermediate files                         </td></tr>
</table>

References:
-----------

<dl>
    <dt>"Kleene star"</dt>
    <dd>http://en.wikipedia.org/wiki/Kleene_star</dd>
</dl>

Keywords:
---------

    Lex, Yacc, Flex, Bison, Parsing, EBNF, Kleene Closure, Kleene Star
