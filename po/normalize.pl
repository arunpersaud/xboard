#! /usr/bin/perl
# Normalize unicode text into form C (canonical decomposition)

use Unicode::Normalize;
binmode STDIN, ':encoding(UTF-8)';
binmode STDOUT, ':encoding(UTF-8)';

while (<>) {
    print NFC($_);
}
