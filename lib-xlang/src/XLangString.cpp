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

#include "XLangString.h" // Allocator
#include <string> // std::string
#include <sstream> // std::stringstream
#include <string.h> // strcpy
#include <regex.h> // regex_t
#include <stdarg.h> // va_list

namespace xl {

std::string replace(std::string s, const std::string& find_string, const std::string& replace_string)
{
    for(size_t p = 0; (p = s.find(find_string, p)) != std::string::npos; p += replace_string.length())
         s.replace(p, find_string.length(), replace_string);
    return s;
}

std::string escape_xml(std::string s)
{
    std::string xml = s;
    xml = replace(xml, "&",  "&amp;"); // must replace first
    xml = replace(xml, "\"", "&quot;");
    xml = replace(xml, "\'", "&apos;");
    xml = replace(xml, "<",  "&lt;");
    xml = replace(xml, ">",  "&gt;");
    return escape(xml);
}

std::string unescape_xml(std::string s)
{
    std::string xml = s;
    xml = replace(xml, "&quot;", "\"");
    xml = replace(xml, "&apos;", "\'");
    xml = replace(xml, "&lt;",   "<");
    xml = replace(xml, "&gt;",   ">");
    xml = replace(xml, "&amp;",  "&"); // must replace last
    return unescape(xml);
}

std::string escape(std::string s)
{
    std::stringstream ss;
    for(size_t i = 0; i<s.length(); i++)
        ss << escape(s[i]);
    return ss.str();
}

std::string unescape(std::string s)
{
    char* buf = new char[s.length()+1]; // can't use allocator for arrays
    strcpy(buf, s.c_str());
    char* w = buf;
    bool unescape_next_char = false;
    for(const char* r = buf; *r; r++)
    {
        if(!unescape_next_char && *r == '\\')
        {
            unescape_next_char = true;
            continue;
        }
        else if(unescape_next_char)
        {
            *w++ = unescape(*r);
            unescape_next_char = false;
            continue;
        }
        *w++ = *r;
    }
    *w = '\0';
    std::string s2(buf);
    delete []buf;
    return s2;
}

std::string escape(char c)
{
    switch(c)
    {
        case '\r': return "\\r";
        case '\n': return "\\n";
        case '\t': return "\\t";
        case '\"': return "\\\"";
        case '\'': return "\\\'";
        case '\\': return "\\\\";
    }
    char buf[] = " \0";
    buf[0] = c;
    std::string s(buf);
    return s;
}

char unescape(char c)
{
    switch(c)
    {
        case 'r': return '\r';
        case 'n': return '\n';
        case 't': return '\t';
    }
    return c;
}

bool match_regex(std::string s, std::string pattern, int nmatch, ...)
{
    bool result = true;
    regex_t preg;
    if(regcomp(&preg, pattern.c_str(), REG_ICASE|REG_EXTENDED))
        return false;
    regmatch_t* pmatch = new regmatch_t[nmatch];
    if(pmatch)
    {
        int status = regexec(&preg, s.c_str(), nmatch, pmatch, 0);
        regfree(&preg);
        if(!status)
        {
            va_list ap;
            va_start(ap, nmatch);
            for(int i = 0; i<nmatch; i++)
            {
                std::string* ptr = va_arg(ap, std::string*);
                if(ptr)
                    *ptr = s.substr(pmatch[i].rm_so, pmatch[i].rm_eo-pmatch[i].rm_so);
            }
            va_end(ap);
        }
        else
            result = false;
        delete[] pmatch;
    }
    else
        result = false;
    return result;
}

}
