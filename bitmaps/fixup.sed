/_$/N
s/#define .*_\(.*\) \(.*\)/#define NAME_\1 \2/
s/static char .*/static unsigned char NAME_bits[] = {/
