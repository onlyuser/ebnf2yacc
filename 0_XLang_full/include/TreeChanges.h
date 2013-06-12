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

#ifndef TREE_CHANGES_H_
#define TREE_CHANGES_H_

#include <string> // std::string
#include <list> // std::list
#include <map> // std::map

namespace xl { namespace node { class NodeIdentIFace; } }
namespace xl { class TreeContext; }

class TreeChange
{
public:
    typedef enum
    {
        NODE_INSERT_AFTER,
        NODE_PUSH_BACK,
        NODE_REPLACE,
        NODE_DELETE,
        STRING_APPEND,
        STRING_INSERT_FRONT
    } type_t;

    TreeChange(type_t _type, xl::node::NodeIdentIFace* reference_node)
        : m_type(_type), m_reference_node(reference_node)
    {}
    virtual ~TreeChange()
    {}
    virtual void apply() = 0;

protected:
    type_t m_type;
    xl::node::NodeIdentIFace* m_reference_node;
};

template<TreeChange::type_t>
class TreeChangeImpl;

template<>
class TreeChangeImpl<TreeChange::NODE_INSERT_AFTER> : public TreeChange
{
public:
    TreeChangeImpl(
            TreeChange::type_t _type, xl::node::NodeIdentIFace* reference_node,
            xl::node::NodeIdentIFace* new_node)
        : TreeChange(_type, reference_node), m_new_node(new_node)
    {}
    void apply();

private:
    xl::node::NodeIdentIFace* m_new_node;
};

template<>
class TreeChangeImpl<TreeChange::NODE_PUSH_BACK> : public TreeChange
{
public:
    TreeChangeImpl(
            TreeChange::type_t _type, xl::node::NodeIdentIFace* reference_node,
            xl::node::NodeIdentIFace* new_node)
        : TreeChange(_type, reference_node), m_new_node(new_node)
    {}
    void apply();

private:
    xl::node::NodeIdentIFace* m_new_node;
};

template<>
class TreeChangeImpl<TreeChange::NODE_REPLACE> : public TreeChange
{
public:
    TreeChangeImpl(
            TreeChange::type_t _type, xl::node::NodeIdentIFace* reference_node,
            xl::node::NodeIdentIFace* new_node)
        : TreeChange(_type, reference_node), m_new_node(new_node)
    {}
    void apply();

private:
    xl::node::NodeIdentIFace* m_new_node;
};

template<>
class TreeChangeImpl<TreeChange::NODE_DELETE> : public TreeChange
{
public:
    TreeChangeImpl(
            TreeChange::type_t _type, xl::node::NodeIdentIFace* reference_node,
            int index)
        : TreeChange(_type, reference_node), m_index(index)
    {}
    void apply();

private:
    int m_index;
};

template<>
class TreeChangeImpl<TreeChange::STRING_APPEND> : public TreeChange
{
public:
    TreeChangeImpl(
            TreeChange::type_t _type, xl::node::NodeIdentIFace* reference_node,
            std::string new_string)
        : TreeChange(_type, reference_node), m_new_string(new_string)
    {}
    void apply();

private:
    std::string m_new_string;
};

template<>
class TreeChangeImpl<TreeChange::STRING_INSERT_FRONT> : public TreeChange
{
public:
    TreeChangeImpl(
            TreeChange::type_t _type, xl::node::NodeIdentIFace* reference_node,
            std::string new_string)
        : TreeChange(_type, reference_node), m_new_string(new_string)
    {}
    void apply();

private:
    std::string m_new_string;
};

class TreeChanges
{
public:
    TreeChanges()
    {}
    ~TreeChanges();
    void reset();
    void add_change(
            TreeChange::type_t        _type,
            xl::node::NodeIdentIFace* reference_node,
            xl::node::NodeIdentIFace* new_node);
    void add_change(
            TreeChange::type_t        _type,
            xl::node::NodeIdentIFace* reference_node,
            int                       index);
    void add_change(
            TreeChange::type_t        _type,
            xl::node::NodeIdentIFace* reference_node,
            std::string               new_string);
    bool apply();

private:
    std::list<TreeChange*> m_tree_changes;
};

#endif
