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

#include "mvc/XLangMVCModel.h" // mvc::MVCModel
#include "XLangTreeContext.h" // TreeContext
#include "node/XLangNode.h" // node::NodeIdentIFace
#include "XLangString.h" // xl::unescape_xml
#include "XLangType.h" // uint32_t
#include <stdarg.h> // va_list
#include <string.h> // memset
#include <string> // std::string
#include <vector> // std::vector

#ifdef EXTERN_INCLUDE_PATH
    #define TIXML_USE_TICPP
#endif
#ifdef TIXML_USE_TICPP
    #include "ticpp/ticpp.h"
    #include <map> // std::map

    // prototype
    extern uint32_t name_to_id(std::string name);
#endif

namespace xl { namespace mvc {

node::SymbolNode* MVCModel::make_symbol(TreeContext* tc, uint32_t lexer_id, YYLTYPE loc, size_t size, ...)
{
    va_list ap;
    va_start(ap, size);
    node::SymbolNode* node = new (PNEW(tc->alloc(), node::, NodeIdentIFace))
            node::SymbolNode(lexer_id, loc, size, ap);
    va_end(ap);
    return node;
}

template<>
node::NodeIdentIFace* MVCModel::make_term<
        node::TermInternalType<node::NodeIdentIFace::STRING>::type
        >(TreeContext* tc, uint32_t lexer_id, YYLTYPE loc,
                node::TermInternalType<node::NodeIdentIFace::STRING>::type value)
{
    return new (PNEW(tc->alloc(), node::, NodeIdentIFace))
            node::TermNode<node::NodeIdentIFace::STRING>(lexer_id, loc, value); // supports non-trivial dtor
}

template<>
node::NodeIdentIFace* MVCModel::make_term<
        node::TermInternalType<node::NodeIdentIFace::IDENT>::type
        >(TreeContext* tc, uint32_t lexer_id, YYLTYPE loc,
                node::TermInternalType<node::NodeIdentIFace::IDENT>::type value)
{
    return new (PNEW(tc->alloc(), node::, NodeIdentIFace))
            node::TermNode<node::NodeIdentIFace::IDENT>(lexer_id, loc, value); // supports non-trivial dtor
}

#ifdef TIXML_USE_TICPP
static node::NodeIdentIFace* _make_term_from_typename(
        TreeContext* tc, std::string _typename, uint32_t lexer_id, std::string value)
{
    static YYLTYPE dummy_loc;
    memset(&dummy_loc, 0, sizeof(dummy_loc));
    if(_typename == "int")
        return mvc::MVCModel::make_term(tc, lexer_id, dummy_loc,
                static_cast<node::TermInternalType<node::NodeIdentIFace::INT>::type>(
                        atoi(value.c_str())
                        ));
    if(_typename == "float")
        return mvc::MVCModel::make_term(tc, lexer_id, dummy_loc,
                static_cast<node::TermInternalType<node::NodeIdentIFace::FLOAT>::type>(
                        atof(value.c_str())
                        ));
    if(_typename == "string")
        return mvc::MVCModel::make_term(tc, lexer_id, dummy_loc,
                static_cast<node::TermInternalType<node::NodeIdentIFace::STRING>::type>(
                        tc->alloc_string(xl::unescape_xml(value))
                        ));
    if(_typename == "char")
        return mvc::MVCModel::make_term(tc, lexer_id, dummy_loc,
                static_cast<node::TermInternalType<node::NodeIdentIFace::CHAR>::type>(
                        xl::unescape_xml(value)[0]
                        ));
    if(_typename == "ident")
        return mvc::MVCModel::make_term(tc, lexer_id, dummy_loc,
                static_cast<node::TermInternalType<node::NodeIdentIFace::IDENT>::type>(
                        tc->alloc_unique_string(value)
                        ));
    return NULL;
}

static node::NodeIdentIFace* _make_ast_from_ticpp(TreeContext* tc, ticpp::Node* ticpp_node)
{
    static YYLTYPE dummy_loc;
    memset(&dummy_loc, 0, sizeof(dummy_loc));
    if(dynamic_cast<ticpp::Document*>(ticpp_node))
    {
        uint32_t lexer_id = 0;
        node::SymbolNode* root_symbol = mvc::MVCModel::make_symbol(tc, lexer_id, dummy_loc, 0);
        if(!ticpp_node->NoChildren())
        {
            ticpp::Iterator<ticpp::Node> p;
            for(p = p.begin(ticpp_node); p != p.end(); p++)
                root_symbol->push_back(_make_ast_from_ticpp(tc, p.Get()));
            if(root_symbol->size() == 1)
            {
                node::NodeIdentIFace* root_node = (*root_symbol)[0];
                root_node->detach();
                tc->alloc()._free(root_symbol);
                return root_node;
            }
        }
        return root_symbol;
    }
    if(dynamic_cast<ticpp::Declaration*>(ticpp_node))
        return NULL;
    std::string node_typename, node_value;
    uint32_t lexer_id;
    ticpp::Element* elem = dynamic_cast<ticpp::Element*>(ticpp_node);
    if(elem)
    {
        std::map<std::string, std::string> attr_map; // in case you need it
        ticpp::Iterator<ticpp::Attribute> q;
        for(q = q.begin(elem); q != q.end(); q++)
        {
            std::string attr_name, attr_value;
            q->GetName(&attr_name);
            q->GetValue(&attr_value);
            attr_map[attr_name] = attr_value;
        }
        node_typename = attr_map["type"];
        node_value = attr_map["value"];
        lexer_id = name_to_id(node_typename);
    }
    node::NodeIdentIFace* term_node = _make_term_from_typename(tc, node_typename, lexer_id, node_value);
    if(term_node)
        return term_node;
    else
    {
        node::SymbolNode* symbol_node = mvc::MVCModel::make_symbol(tc, lexer_id, dummy_loc, 0);
        ticpp::Iterator<ticpp::Node> r;
        for(r = r.begin(ticpp_node); r != r.end(); r++)
            symbol_node->push_back(_make_ast_from_ticpp(tc, r.Get()));
        return symbol_node;
    }
}
#endif

node::NodeIdentIFace* MVCModel::make_ast(TreeContext* tc, std::string filename)
{
#ifdef TIXML_USE_TICPP
    ticpp::Document doc(filename.c_str());
    doc.LoadFile();
    return _make_ast_from_ticpp(tc, &doc);
#else
    return NULL;
#endif
}

} }
