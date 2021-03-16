# easter bug hunt 2021

the comments are correct.

the code is buggy.

happy hunting!

## build instructions:

    $ g++ -o ebh ebh.cc

## to encode a message

    $ ./ebh -e "what is this?"
    stringofrandomsymbols

## to decode a message

    $ ./ebh -d stringofrandomsymbols
    what is this?
