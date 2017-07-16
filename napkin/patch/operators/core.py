from patch.operator import *


def bool_and(a:bool=False, b:bool=False)->bool: return a and b
And = operatorFromFunction(bool_and)

def bool_or(a:bool=False, b:bool=False)->bool: return a or b
Or = operatorFromFunction(bool_or)

def bool_not(b:bool=False)->bool: return not b
Not = operatorFromFunction(bool_not)



def eval(code:str='')->object: return eval(code)
Eval = operatorFromFunction(eval)