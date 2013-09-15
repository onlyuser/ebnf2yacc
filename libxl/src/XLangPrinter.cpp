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

#include "visitor/XLangPrinter.h" // visitor::LispPrinter
#include "XLangString.h" // xl::escape
#include <iostream> // std::cout

//#define INCLUDE_NODE_UID

namespace xl { namespace visitor {

void LispPrinter::visit(const node::SymbolNodeIFace* _node)
{
    static size_t depth;
    std::cout << '(' << _node->name() << std::endl;
    depth++;
    bool more;
    do
    {
        std::cout << std::string(depth*4, ' ');
        more = visit_next_child(_node);
        std::cout << std::endl;
    } while(more);
    depth--;
    std::cout << std::string(depth*4, ' ') << ')';
    if(_node->is_root())
        std::cout << std::endl;
}

void LispPrinter::visit_null()
{
    std::cout << std::string(depth*4, ' ') << "(NULL)" << std::endl;
}

void XMLPrinter::visit(const node::TermNodeIFace<node::NodeIdentIFace::INT>* _node)
{
    std::cout << std::string(depth*4, ' ') << "<term ";
    #ifdef INCLUDE_NODE_UID
        std::cout << "id=" << _node->uid() << " ";
    #endif
    std::cout << "type=\"" << _node->name() << "\" value=";
    VisitorDFS::visit(_node);
    std::cout << "/>" << std::endl;
}

void XMLPrinter::visit(const node::TermNodeIFace<node::NodeIdentIFace::FLOAT>* _node)
{
    std::cout << std::string(depth*4, ' ') << "<term ";
    #ifdef INCLUDE_NODE_UID
        std::cout << "id=" << _node->uid() << " ";
    #endif
    std::cout << "type=\"" << _node->name() << "\" value=";
    VisitorDFS::visit(_node);
    std::cout << "/>" << std::endl;
}

void XMLPrinter::visit(const node::TermNodeIFace<node::NodeIdentIFace::STRING>* _node)
{
    std::cout << std::string(depth*4, ' ') << "<term ";
    #ifdef INCLUDE_NODE_UID
        std::cout << "id=" << _node->uid() << " ";
    #endif
    std::cout << "type=\"" << _node->name() << "\" value=";
    std::cout << '\"' << xl::escape_xml(*_node->value()) << '\"';
    std::cout << "/>" << std::endl;
}

void XMLPrinter::visit(const node::TermNodeIFace<node::NodeIdentIFace::CHAR>* _node)
{
    std::cout << std::string(depth*4, ' ') << "<term ";
    #ifdef INCLUDE_NODE_UID
        std::cout << "id=" << _node->uid() << " ";
    #endif
    std::cout << "type=\"" << _node->name() << "\" value=";
    VisitorDFS::visit(_node);
    std::cout << "/>" << std::endl;
}

void XMLPrinter::visit(const node::TermNodeIFace<node::NodeIdentIFace::IDENT>* _node)
{
    std::cout << std::string(depth*4, ' ') << "<term ";
    #ifdef INCLUDE_NODE_UID
        std::cout << "id=" << _node->uid() << " ";
    #endif
    std::cout << "type=\"" << _node->name() << "\" value=";
    VisitorDFS::visit(_node);
    std::cout << "/>" << std::endl;
}

void XMLPrinter::visit_null()
{
    std::cout << std::string(depth*4, ' ') << "<NULL/>" << std::endl;
}

void XMLPrinter::visit(const node::SymbolNodeIFace* _node)
{
    std::cout << std::string(depth*4, ' ') << "<symbol ";
    #ifdef INCLUDE_NODE_UID
        std::cout << "id=" << _node->uid() << " ";
    #endif
    std::cout << "type=\"" << _node->name() << "\">" << std::endl;
    depth++;
    VisitorDFS::visit(_node);
    depth--;
    std::cout << std::string(depth*4, ' ') << "</symbol>" << std::endl;
}

void DotPrinter::visit(const node::TermNodeIFace<node::NodeIdentIFace::INT>* _node)
{
    std::cout << "\t" << _node->uid() << " [" << std::endl <<
            "\t\tlabel=\"" << _node->value() << "\"," << std::endl <<
            "\t\tshape=\"box\"" << std::endl <<
            "\t];" << std::endl;
    std::cout << '\t' << _node->parent()->uid() << "->" << _node->uid() << ";" << std::endl;
}

void DotPrinter::visit(const node::TermNodeIFace<node::NodeIdentIFace::FLOAT>* _node)
{
    std::cout << "\t" << _node->uid() << " [" << std::endl <<
            "\t\tlabel=\"" << _node->value() << "\"," << std::endl <<
            "\t\tshape=\"box\"" << std::endl <<
            "\t];" << std::endl;
    std::cout << '\t' << _node->parent()->uid() << "->" << _node->uid() << ";" << std::endl;
}

void DotPrinter::visit(const node::TermNodeIFace<node::NodeIdentIFace::STRING>* _node)
{
    std::cout << "\t" << _node->uid() << " [" << std::endl <<
            "\t\tlabel=\"" << xl::escape(*_node->value()) << "\"," << std::endl <<
            "\t\tshape=\"box\"" << std::endl <<
            "\t];" << std::endl;
    std::cout << '\t' << _node->parent()->uid() << "->" << _node->uid() << ";" << std::endl;
}

void DotPrinter::visit(const node::TermNodeIFace<node::NodeIdentIFace::CHAR>* _node)
{
    std::cout << "\t" << _node->uid() << " [" << std::endl <<
            "\t\tlabel=\"" << xl::escape(_node->value()) << "\"," << std::endl <<
            "\t\tshape=\"box\"" << std::endl <<
            "\t];" << std::endl;
    std::cout << '\t' << _node->parent()->uid() << "->" << _node->uid() << ";" << std::endl;
}

void DotPrinter::visit(const node::TermNodeIFace<node::NodeIdentIFace::IDENT>* _node)
{
    std::cout << "\t" << _node->uid() << " [" << std::endl <<
            "\t\tlabel=\"" << *_node->value() << "\"," << std::endl <<
            "\t\tshape=\"box\"" << std::endl <<
            "\t];" << std::endl;
    std::cout << '\t' << _node->parent()->uid() << "->" << _node->uid() << ";" << std::endl;
}

void DotPrinter::visit_null()
{
    std::cout << "/* NULL */";
}

void DotPrinter::visit(const node::SymbolNodeIFace* _node)
{
    if(m_print_digraph_block && _node->is_root())
        print_header(m_horizontal);
    std::cout << "\t" << _node->uid() << " [" << std::endl <<
            "\t\tlabel=\"" << _node->name() << "\"," << std::endl <<
            "\t\tshape=\"ellipse\"" << std::endl <<
            "\t];" << std::endl;
    VisitorDFS::visit(_node);
    if(!_node->is_root())
        std::cout << '\t' << _node->parent()->uid() << "->" << _node->uid() << ";" << std::endl;
    if(m_print_digraph_block && _node->is_root())
        print_footer();
}

void DotPrinter::print_header(bool horizontal)
{
    std::cout << "digraph g {" << std::endl;
    if(horizontal)
        std::cout << "\tgraph [rankdir = \"LR\"];" << std::endl;
}

void DotPrinter::print_footer()
{
    std::cout << "}" << std::endl;
}

} }
