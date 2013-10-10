// XLang
// -- A parser framework for language modeling
// Copyright (C) 2011 Jerry Chen <mailto:onlyuser@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef XLANG_PRINTER_H_
#define XLANG_PRINTER_H_

#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "visitor/XLangVisitorDFS.h" // visitor::VisitorDFS

namespace xl { namespace visitor {

class LispPrinter : public VisitorDFS
{
public:
    LispPrinter() : m_depth(0)
    {}
    void visit(const node::SymbolNodeIFace*                             _node);
    void visit(const node::TermNodeIFace<node::NodeIdentIFace::INT>*    _node);
    void visit(const node::TermNodeIFace<node::NodeIdentIFace::FLOAT>*  _node);
    void visit(const node::TermNodeIFace<node::NodeIdentIFace::STRING>* _node);
    void visit(const node::TermNodeIFace<node::NodeIdentIFace::CHAR>*   _node);
    void visit(const node::TermNodeIFace<node::NodeIdentIFace::IDENT>*  _node);
    void visit_null();

private:
    size_t m_depth;
};

struct XMLPrinter : public VisitorDFS
{
public:
    XMLPrinter() : m_depth(0)
    {}
    void visit(const node::SymbolNodeIFace*                             _node);
    void visit(const node::TermNodeIFace<node::NodeIdentIFace::INT>*    _node);
    void visit(const node::TermNodeIFace<node::NodeIdentIFace::FLOAT>*  _node);
    void visit(const node::TermNodeIFace<node::NodeIdentIFace::STRING>* _node);
    void visit(const node::TermNodeIFace<node::NodeIdentIFace::CHAR>*   _node);
    void visit(const node::TermNodeIFace<node::NodeIdentIFace::IDENT>*  _node);
    void visit_null();

private:
    size_t m_depth;
};

struct DotPrinter : public VisitorDFS
{
public:
    DotPrinter(bool horizontal = false, bool print_digraph_block = true)
        : m_horizontal(horizontal), m_print_digraph_block(print_digraph_block)
    {}
    void visit(const node::SymbolNodeIFace*                             _node);
    void visit(const node::TermNodeIFace<node::NodeIdentIFace::INT>*    _node);
    void visit(const node::TermNodeIFace<node::NodeIdentIFace::FLOAT>*  _node);
    void visit(const node::TermNodeIFace<node::NodeIdentIFace::STRING>* _node);
    void visit(const node::TermNodeIFace<node::NodeIdentIFace::CHAR>*   _node);
    void visit(const node::TermNodeIFace<node::NodeIdentIFace::IDENT>*  _node);
    void visit_null();
    static void print_header(bool horizontal);
    static void print_footer();

private:
    bool m_horizontal;
    bool m_print_digraph_block;
};

} }

#endif
