#ifndef SKIP_COMMENT_H
#define SKIP_COMMENT_H

#include <istream>

namespace toolbox {

/** Eats all whitespace and comment lines.
    Comment lines are lines with the comment character in the
    first non-whitespace position.  All whitespace and comment 
    lines are gobbled. After calling the function, the first
    character in the stream will be non-white, non-comment.

    @param is input stream
    @param comment_char comment character
*/

std::istream& skip_comment(std::istream& is, char comment_char = '%');

}

#endif
