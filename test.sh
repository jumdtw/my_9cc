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

try 0 "int main(){return 0;}"
try 42 "int main(){return 42;}"
try 21 "int main(){return 5+20-4;}"
try 27 "int main(){return 3*3*3;}"
try 41 "int main(){return 12 + 34 -5;}"
try 47 "int main(){return 5 + 6 * 7;}"
try 15 "int main(){return 5 * ( 9 - 6);}"
try 4 "int main(){return (3+5)/2;}"
try 1 "int main(){return -4+5;}"
try 1 "int main(){return 1<2;}"
try 0 "int main(){return 8+8+8<(1+5+6);}"
try 1 "int main(){return 2>1;}"
try 0 "int main(){return (1+5+6)>8+8+8;}"
try 1 "int main(){return 1<=1;}"
try 1 "int main(){return 1>=1;}"
try 0 "int main(){return 5<=(-5+4);}"
try 0 "int main(){return (-5+4)>=5;}"
try 1 "int main(){return 1==1;}"
try 1 "int main(){return 1!=0;}"
try 72 "int main(){int b;return b=8*9;}"
try 42 "int main(){int a;return a = 6*7;}"
try 9 "int main(){int b;b = 18;return b-9;}"
try 9 "int main(){int jum;int dtw;jum = 18;dtw = 9;return jum - dtw;}"
try 9 "int main(){if(1) return 9;}"
try 9 "int main(){if(0) return 1; else return 9;}"
try 9 "int main(){if(2>1) return 9;}"
try 10 "int main(){int b;int p;b = 10;p = 9;while(p)p=p-1;return b;}"
try 9 "int main(){{int b;b = 18;return b-9;}}"
try 1 "int main(){
int b;
int p;
b = 10;
p = 9;
while(p){
    p=p-1;
    b=b-1;
}
return b;
}"

echo OK