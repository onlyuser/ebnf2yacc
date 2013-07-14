// ebnf2yacc
// -- A kleene closure preprocessor for yacc
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

#ifndef EBNF_PRINTER_H_
#define EBNF_PRINTER_H_

#include "visitor/XLangVisitorDFS.h" // visitor::VisitorDFS
#include "SetTreeChangesIFace.h" // SetTreeChangesIFace
#include <map> // std::map

namespace xl { namespace node { class SymbolNodeIFace; } }
namespace xl { class TreeContext; }
class TreeChanges;

struct EBNFContext
{
    void reset();

    xl::node::NodeIdentIFace*                        m_definitions_node;
    xl::node::NodeIdentIFace*                        m_proto_block_node;
    xl::node::NodeIdentIFace*                        m_union_block_node;
    std::map<std::string, xl::node::NodeIdentIFace*> m_def_symbol_name_to_node;
    std::map<std::string, std::string>               m_union_typename_to_type;
    std::map<std::string, std::string>               m_def_symbol_name_to_union_typename;
};

struct KleeneContext
{
    KleeneContext(
            TreeChanges*              tree_changes,
            xl::node::NodeIdentIFace* kleene_node,
            EBNFContext*              ebnf_context,
            xl::TreeContext*          tc);

    uint32_t                  m_kleene_op;
    xl::node::NodeIdentIFace* m_outermost_paren_node;
    xl::node::NodeIdentIFace* m_innermost_paren_node;
    xl::node::NodeIdentIFace* m_rule_node;
    xl::node::NodeIdentIFace* m_rule_def_symbol_node;
    std::string               m_rule_name_recursive;
    std::string               m_rule_name_term;
};

class EBNFPrinter : public xl::visitor::VisitorDFS, public SetTreeChangesIFace
{
public:
    EBNFPrinter(xl::TreeContext* tc)
        : m_tc(tc), m_tree_changes(NULL)
    {}
    void visit(const xl::node::SymbolNodeIFace* __node);
    void setTreeChanges(TreeChanges* tree_changes)
    {
        m_tree_changes = tree_changes;
    }

private:
    xl::TreeContext* m_tc;
    TreeChanges* m_tree_changes;
};

#endif
