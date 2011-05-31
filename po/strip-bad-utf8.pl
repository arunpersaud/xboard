#! /usr/bin/perl
#
# Strip off utf8 encoding that was spuriously applied to a string of
# 8-bit bytes.  Note: This may be totally bogus.  If you get any "Wide
# character in print" messages, then the input was not a string of
# 8-bit bytes that had utf8 encoding applied to it -- the message
# means that utf8 decoding produced some characters > 0xff.

binmode STDIN, ':encoding(UTF-8)';
binmode STDOUT, ':raw';

while (<>) {
    print $_;
}
