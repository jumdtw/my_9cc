#! /bin/bash
try(){
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$expected expected, but got $actual"
        exit 1
    fi
}

try 0 "return 0;"
try 42 "return 42;"
try 21 "return 5+20-4;"
try 41 "return 12 + 34 -5;"
try 47 "return 5 + 6 * 7;"
try 15 "return 5 * ( 9 - 6);"
try 4 "return (3+5)/2;"
try 1 "return -4+5;"
try 1 "return 1<2;"
try 0 "return 8+8+8<(1+5+6);"
try 1 "return 2>1;"
try 0 "return (1+5+6)>8+8+8;"
try 1 "return 1<=1;"
try 1 "return 1>=1;"
try 0 "return 5<=(-5+4);"
try 0 "return (-5+4)>=5;"
try 1 "return 1==1;"
try 1 "return 1!=0;"
try 72 "return b=8*9;"
try 42 "return a = 6*7;"
try 9 "b = 18;return b-9;"
try 9 "jum = 18;dtw = 9;return jum - dtw;"
echo OK