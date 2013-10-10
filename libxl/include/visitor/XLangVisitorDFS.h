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

#ifndef XLANG_VISITOR_DFS_H_
#define XLANG_VISITOR_DFS_H_

#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "visitor/XLangVisitorIFace.h" // visitor::VisitorIFace
#include <sstream> // std::stringstream
#include <stack>

namespace xl { namespace visitor {

struct VisitorDFS : public VisitorIFace<const node::NodeIdentIFace>
{
    VisitorDFS() : m_allow_visit_null(true), m_skip_singleton(false)
    {}
    virtual ~VisitorDFS()
    {}
    virtual void visit(const node::SymbolNodeIFace*                             _node);
    virtual void visit(const node::TermNodeIFace<node::NodeIdentIFace::INT>*    _node);
    virtual void visit(const node::TermNodeIFace<node::NodeIdentIFace::FLOAT>*  _node);
    virtual void visit(const node::TermNodeIFace<node::NodeIdentIFace::STRING>* _node);
    virtual void visit(const node::TermNodeIFace<node::NodeIdentIFace::CHAR>*   _node);
    virtual void visit(const node::TermNodeIFace<node::NodeIdentIFace::IDENT>*  _node);
    virtual void visit_null();
    void dispatch_visit(const node::NodeIdentIFace* unknown);
    void set_allow_visit_null(bool allow_visit_null)
    {
        m_allow_visit_null = allow_visit_null;
    }
    void set_skip_singleton(bool skip_singleton)
    {
        m_skip_singleton = skip_singleton;
    }

protected:
    node::NodeIdentIFace* get_next_child(const node::SymbolNodeIFace* _node);
    bool visit_next_child(const node::SymbolNodeIFace* _node,
            node::NodeIdentIFace** ref_node = NULL);
    void abort_visitation(const node::SymbolNodeIFace* _node);

private:
    std::stack<int> m_visit_state;
    bool            m_allow_visit_null;
    bool            m_skip_singleton;

    int get_next_child_index(const node::SymbolNodeIFace* _node);
};

} }

#endif
