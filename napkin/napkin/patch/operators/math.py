from napkin.patch.operator import *


def add(a: float = 0, b: float = 0) -> float: return a + b
Add = operatorFromFunction(add)

def sub(a: float = 1, b: float = 1) -> float: return a - b
Sub = operatorFromFunction(sub)

def mult(a: float = 1, b: float = 1) -> float: return a * b
Mult = operatorFromFunction(mult)

def div(a: float = 1, b: float = 1) -> float: return a / b
Div = operatorFromFunction(div)

def mod(a: float = 1, b: float = 1) -> float: return a % b
Mod = operatorFromFunction(mod)

def pow(n: float = 1, e: float = 1) -> float: return n ** e
Pow = operatorFromFunction(pow)