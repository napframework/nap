import inspect

from patch.operators import *
from patch.operators.math import *
from patch.operator import *
from utils.qtutils import randomColor


def testPlugEvaluation():
    op1 = AddOperator()
    op1.termA.defaultValue = 2
    op1.termB.defaultValue = 3
    op2 = AddOperator()
    op2.termA.connect(op1.sum)
    result = op1.sum()
    assert(op1.sum() == 5)


def testPrintMeta(cls):
    print(serializeAllOperatorsMeta())

def testListOperators():
    for op in allOperators():
        print(op)
        print(op.displayName)




def add(a:float=0, b:float=0) -> (float, float):
    return a + b

def testOperatorFromFunction():
    op = operatorFromFunction(add)

    print(json.dumps(op().dict(), indent=2))

if __name__ == '__main__':
    # testPlugEvaluation()
    # testPrintMeta(AddOperator)
    # testListOperators()
    # testOperatorFromFunction()

    for i in range(100):
        print('<font color="{0}">{0}</font><br/>'.format(randomColor(i).name()))
