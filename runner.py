from ctypes import *

hello_world = CDLL("build/hello_world")

hello_world.init()
hello_world.run("print('hello, world!');")
hello_world.run("var i = 1;")
hello_world.run("print('i = ' + i);")
hello_world.run("i += 1;")
hello_world.run("print('hello'.length);")
hello_world.run("print(Math.PI);")
hello_world.run("'use strict'; var foo = function() { let x = 5; print(`x = ${x}`); };")
hello_world.run("foo();")

hello_world.run.restype = POINTER(c_char)
result = hello_world.run("JSON.stringify({ x:5, y: 10 });");
print cast(result, c_char_p).value
# hello_world.run("console.log('hello from the console.');")

hello_world.run('throw "something bad happened";')
hello_world.run('print("everything is okay now.")')

hello_world.cleanup()

