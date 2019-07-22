
## Generates the checksum for 3rd-party
#!/bin/bash


if [ -f  checksum.tmp ]; then
    rm checksum.tmp
fi

touch checksum.tmp

checksum_file() {
    echo `openssl md5 $1 | awk '{print $2}'`
}

if [ -f /tmp/checksum.$$ ]; then
    rm /tmp/checksum.$$
fi

find 3rd-party/ build/googletest-src/ -name *.cpp  -o -name *.h > /tmp/checksum.$$

while read line;
do
    checksum_file $line >> checksum.tmp
done < "/tmp/checksum.$$"

rm /tmp/checksum.$$

sort checksum.tmp -o checksum.tmp

