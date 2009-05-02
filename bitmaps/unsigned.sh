#! /bin/csh

foreach file ($*)
  mv $file ${file}.old$$ && \
  sed -e 's/static char/static unsigned char/' < $file.old$$ > $file && \
  rm ${file}.old$$
end

