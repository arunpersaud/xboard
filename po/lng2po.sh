cp winboard/$1 language.lng
recode latin-1 language.lng
ed language.lng < metascript
rm language.lng
cp xboard.pot language.po
ed language.po < script
rm script

