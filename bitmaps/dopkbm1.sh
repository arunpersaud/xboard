size=$1
from=$2
char=$3
piece=$4
kind=$5

# pk2bm has a bug in the -W -H feature: sometimes it gives a
# garbled bitmap.
#pk2bm -W$size -H$size -o $char $from |  tr -d '\000' | \
#    sed -f fixup.sed | sed -e s/NAME/${piece}${size}${kind}/ \
#    > ${piece}${size}${kind}.bm

# It doesn't help to add the -b flag; the bug remains the same.
# This way also has the drawback of depending on a quickie homebrew
# program (pktype2bm) that isn't very robust.
#pk2bm -W$size -H$size -b -o $char $from | egrep '\.|\*' |
#    pktype2bm ${piece}${size}${kind} $size $size \
#    > ${piece}${size}${kind}.bm

# This way, you have to manually edit each bitmap (with the "bitmap"
# program, say) to add margins as needed.  It has the advantage that
# you have control over placement, so you don't have to center exactly
# if that's the wrong thing to do.
pk2bm -o $char $from |  tr -d '\000' | \
    sed -f fixup.sed | sed -e s/NAME/${piece}${size}${kind}/ \
    > ${piece}${size}${kind}.bm

