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
#include <iostream> // std::cerr
#include <string> // std::string
#include <sstream> // std::stringstream
#include <string.h> // strcpy
#include <vector> // std::vector
#include <regex.h> // regex_t
#include <stdarg.h> // va_list
#include <stdio.h> // FILE

namespace xl {

bool read_file(std::string filename, std::string &s)
{
    FILE* file = fopen(filename.c_str(), "rb");
    if(!file)
    {
        std::cerr << "cannot open file" << std::endl;
        return false;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    if(!length)
    {
        std::cerr << "file empty" << std::endl;
        fclose(file);
        return false;
    }
    char* buffer = new char[length+1];
    if(!buffer)
    {
        std::cerr << "not enough memory" << std::endl;
        fclose(file);
        return false;
    }
    buffer[length] = '\0';
    fread(buffer, 1, length, file);
    fclose(file);
    s = buffer;
    delete[] buffer;
    return true;
}

std::string replace(std::string &s, std::string find_string, std::string replace_string)
{
    std::string _s(s);
    for(size_t p = 0; (p = _s.find(find_string, p)) != std::string::npos; p += replace_string.length())
         _s.replace(p, find_string.length(), replace_string);
    return _s;
}

std::vector<std::string> tokenize(const std::string &s, const char* delim)
{
    std::vector<std::string> results;
    size_t prev = 0;
    size_t next = 0;
    while((next = s.find_first_of(delim, prev)) != std::string::npos)
    {
        if(next-prev != 0)
            results.push_back(s.substr(prev, next - prev));
        prev = next+1;
    }
    if(prev < s.size())
        results.push_back(s.substr(prev));
    return results;
}

std::string escape_xml(std::string &s)
{
    std::string _s(s);
    _s = replace(_s, "&",  "&amp;"); // must replace first
    _s = replace(_s, "\"", "&quot;");
    _s = replace(_s, "\'", "&apos;");
    _s = replace(_s, "<",  "&lt;");
    _s = replace(_s, ">",  "&gt;");
    return escape(_s);
}

std::string unescape_xml(std::string &s)
{
    std::string _s(s);
    _s = replace(_s, "&quot;", "\"");
    _s = replace(_s, "&apos;", "\'");
    _s = replace(_s, "&lt;",   "<");
    _s = replace(_s, "&gt;",   ">");
    _s = replace(_s, "&amp;",  "&"); // must replace last
    return unescape(_s);
}

std::string escape(std::string &s)
{
    std::stringstream ss;
    for(size_t i = 0; i<s.length(); i++)
        ss << escape(s[i]);
    return ss.str();
}

std::string unescape(std::string &s)
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

bool match_regex(std::string &s, std::string pattern, int nmatch, ...)
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
