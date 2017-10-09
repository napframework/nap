from napkin.models import nap

@nap.Operator
def add(a: float = 0, b: float = 0) -> float: return a + b

@nap.Operator
def sub(a: float = 1, b: float = 1) -> float: return a - b

@nap.Operator
def mult(a: float = 1, b: float = 1) -> float: return a * b

@nap.Operator
def div(a: float = 1, b: float = 1) -> float: return a / b

@nap.Operator
def mod(a: float = 1, b: float = 1) -> float: return a % b

@nap.Operator
def pow(n: float = 1, e: float = 1) -> float: return n ** e
