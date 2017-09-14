/*
 * reader.h:
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

#ifndef _READER_H_
#define _READER_H_

enum token_type
{
    ENTOK_NONE,
    ENTOK_AX,
    ENTOK_ID,
    ENTOK_PATTERN,
    ENTOK_REPLACEMENT,
    ENTOK_EOBUF,
    ENTOK_EOF,
    ENTOK_ERR
};

struct token_s
{
    enum token_type type;
    char *value;
    size_t length;
};

char *reader_init (void);
void reader_reset_buffer (int max); 
struct token_s *reader_get_next_token (void);
void reader_release (void);

#define READ_BUFFER_SIZE 4096

#endif /* READER_H_ */
