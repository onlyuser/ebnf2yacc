// XLang
// -- A parser framework for language modeling
// Copyright (C) 2011 onlyuser <mailto:onlyuser@gmail.com>
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

#ifndef XLANG_VISIT_STATE_IFACE_H_
#define XLANG_VISIT_STATE_IFACE_H_

namespace xl { namespace visitor {

class VisitStateIFace
{
public:
    typedef void* state_t;

    virtual ~VisitStateIFace()
    {}
    virtual state_t &visit_state() = 0;
};

} }

#endif
