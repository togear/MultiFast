/*
 * example3.cpp: It shows how to use ahocorasick library with a C++ wrapper
 * 
 * This file is part of multifast.
 *
    Copyright 2010-2015 Kamiar Kanani <kamiar.kanani@gmail.com>

    multifast is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    multifast is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with multifast.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <string>
#include "AhoCorasickPlus.h"

std::string sample_patterns[] = {
    "city",
    "clutter",
    "ever",
    "experience",
    "neo",
    "one",
    "simplicity",
    "utter",
    "whatever",
};
#define PATTERN_COUNT (sizeof(sample_patterns)/sizeof(std::string))

std::string chunk1 = "experience the ease and simplicity of multifast";
std::string chunk2 = "whatever you are be a good one";
std::string chunk3 = "out of clutter, find simplicity";


int main (int argc, char **argv)
{
    AhoCorasickPlus trie;

    for (unsigned int i = 0; i < PATTERN_COUNT; i++)
    {
        AhoCorasickPlus::EnumReturnStatus status;
        AhoCorasickPlus::PatternId patId = i;
        status = trie.addPattern(sample_patterns[i], patId);
        if (status != AhoCorasickPlus::RETURNSTATUS_SUCCESS)
        {
            std::cout << "Failed to add: " << sample_patterns[i] << std::endl;
        }
    }
    trie.finalize();
    
    AhoCorasickPlus::Match aMatch;
    
    std::cout << "Searching '" << chunk1 << "'" << std::endl;
    trie.search(chunk1, false);
    while (trie.findNext(aMatch))
    {
        std::cout 
                << "@" << aMatch.position 
                << "\t#" << aMatch.id 
                << "\t" << sample_patterns[aMatch.id] 
                << std::endl;
    }
    
    std::cout << "Searching '" << chunk2 << "'" << std::endl;
    trie.search(chunk2, false);
    while (trie.findNext(aMatch))
    {
        std::cout 
                << "@" << aMatch.position 
                << "\t#" << aMatch.id 
                << "\t" << sample_patterns[aMatch.id] 
                << std::endl;
    }
    
    std::cout << "Searching '" << chunk3 << "'" << std::endl;
    trie.search(chunk3, true); // try it with keep flag disabled
    while (trie.findNext(aMatch))
    {
        std::cout 
                << "@" << aMatch.position 
                << "\t#" << aMatch.id 
                << "\t" << sample_patterns[aMatch.id] 
                << std::endl;
    }
    
    return 0;
}
