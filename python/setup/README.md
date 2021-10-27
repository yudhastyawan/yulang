# Yu Language - yulang

A toy project for creating a simple programming language using Bison and Flex in C++.
This is the wrapper to Python.

## interactive console
```python
$ python
>>> import yulang
>>> yulang.console()
Yu Language 0.0.1 (unstable, Oct 23 2021, 21:31:19)
interactive:
y> var a = 3;
y> print(a);
print: 3
y> square a: {return a ^ 2;}
y> print(square(a : 10));
print: 100
y> 
```

## parsing Python string
```python
$ python
>>> import yulang
>>> yulang.parse("var a = 10; print(a);")
print: 10
```

## examples
### arithmetic

source codes:
```bash
print(10 + 30);
print(10 - 30);
print(10. / 30.);
print(10 * 30);
print(30 % 9);
print((5. + 5.) / 3.);
```

output:
```python
$ python
>>> import yulang
>>> yulang.read("path/to/arithmetics.yu") 
print: 40
print: -20
print: 0.333333
print: 300
print: 3
print: 3.33333
```

### string

source codes:
```bash
var a = "hello world";
print(a);
```

```python
$ python
>>> import yulang
>>> yulang.read("path/to/strings.yu")  
print: hello world
```

### functions

source codes: (`//` is used for comments)
```bash
// create a square function
square p, l:
{
    var L = p * l;
    return L;
}

// using the square function in a volume function
volume t:
{
    var V = square(p : 5, l : 6) * t;
    return V;
}

// assigning volume return in myVol variable
var myVol = volume(4);
print(myVol);
```

output:
```python
$ python
>>> import yulang
>>> yulang.read("path/to/functions.yu")  
print: 120
```

### variables

source codes: (`var` is used for generating variables)
```bash
// defining variables
var a, b, c, d;
a = 10; // integer
b = .5; // float
c = 3.; // float
d = 3.14; // float

print(a);
print(b);
print(c);
print(d);
```

output:
```python
$ python
>>> import yulang
>>> yulang.read("path/to/variables.yu") 
print: 10
print: 0.5
print: 3
print: 3.14
```

### import files

source codes in `includes.yu`
```bash
multiply a, b:
{
    return a * b;
}
```

source codes in `multi_files.yu`
```bash
// relative to current directory of terminal (or shell)
// you can import multiple relative paths and they do not create errors

import: "tests/stable/multi_files/includes.yu"
import: "stable/multi_files/includes.yu"
import: "multi_files/includes.yu"

var c = multiply(a : 10, b : 20);
print(c);
```

output:
```python
$ python
>>> import yulang
>>> yulang.read("path/to/multi_files.yu")
print: 200
```