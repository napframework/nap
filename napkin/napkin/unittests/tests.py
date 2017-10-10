from unittest.case import TestCase

from napkin.models import nap


def animalTree():
    """
    animal
        mammal
            cat
                lion
            dog
        reptile
    """
    animal = nap.Object('animal')
    mammal = nap.Object('mammal')
    animal.addChild(mammal)
    cat = nap.Object('cat')
    mammal.addChild(cat)
    lion = nap.Object('lion')
    cat.addChild(lion)
    dog = nap.Object('dog')
    mammal.addChild(dog)
    reptile = nap.Object('reptile')
    animal.addChild(reptile)

    return animal


def testPatch() -> nap.Patch:
    from napkin.models.modules import control, string
    patch = nap.Patch()

    start = control.StartOperator()
    prnt = string.PrintOperator()
    concat = string.Concat()

    patch.addOperator(start)
    patch.addOperator(prnt)
    patch.addOperator(concat)

    start.start.connect(prnt.print)

    prnt.str.setValue('Foo')
    concat.getChild('a').setValue('Hello, ')
    concat.getChild('b').setValue('World!')
    prnt.str.connect(concat.getChild('result'))

    return patch


class TestOperator(nap.Operator):
    def __init__(self):
        super(TestOperator, self).__init__()
        self.dataInlet = self.addDataInlet('myInlet', float, 0)
        self.dataOutlet = self.addDataOutlet('myOutlet', int, self._getter)
        self.addTriggerInlet('myInTrigger', self._myTrigger())

    def _getter(self):
        return None

    def _myTrigger(self):
        pass


class MainTest(TestCase):
    def test_createObjectTree(self):
        obj = nap.Object()
        self.assertTrue(obj)

    def test_defaultName(self):
        o = nap.Object()
        expectedName = nap.Object.__name__
        self.assertEqual(o.name(), expectedName,
                         'Object name shoud be "%s", instead got "%s' % (
                             expectedName, o.name()))

    def test_sameNameNotParented(self):
        a = nap.Object()
        b = nap.Object()
        self.assertEqual(a.name(), b.name(),
                         'Objects should have same name: "%s" and "%s"' % (
                             a.name(), b.name()))

    def test_initializeName(self):
        name = 'MyName'
        a = nap.Object(name)
        self.assertEqual(a.name(), name)

    def test_parent(self):
        parent = nap.Object()
        child = nap.Object()
        parent.addChild(child)
        self.assertIn(child, parent.children())

    def test_uniqueNameParented(self):
        parent = nap.Object()
        child1 = nap.Object()
        parent.addChild(child1)
        child2 = nap.Object()
        parent.addChild(child2)
        self.assertNotEqual(child1.name(), child2.name(),
                            'Objects "%s" should have a different name' % child1.name())
        child1.setName(child2.name())
        self.assertNotEqual(child1.name(), child2.name(),
                            'Objects "%s" should have a different name' % child1.name())

    def test_setObjectName(self):
        name = 'Hello'
        obj = nap.Object()
        obj.setName(name)
        self.assertEqual(name, obj.name())

    def test_setParentKeepName(self):
        childName = 'child'
        parent = nap.Object()
        child = nap.Object(childName)
        parent.addChild(child)

        self.assertEqual(child.name(), childName,
                         'Child was renamed from "%s" to "%s" upon parenting' % (
                             childName, child.name()))

        self.assertIsNotNone(parent.getChild(childName),
                             'Expected child "%s"' % childName)
        self.assertIsNone(parent.getChild('NonExistent'), 'Unexpected child')

    def test_getChildByPath(self):
        tree = animalTree()

        reptile = tree.getChild('reptile')
        self.assertIsNotNone(reptile)

        cat = tree.getChild('mammal/cat')
        self.assertIsNotNone(cat)

        lion = tree.getChild('mammal/cat/lion')
        self.assertIsNotNone(lion)

        self.assertEqual(cat.getChild('/mammal'), tree.getChild('mammal'))

        # root
        self.assertEqual(tree, lion.root())

        # relative
        self.assertEqual(reptile,
                         lion.getChild('../../../reptile'))

        # relative
        self.assertEqual(cat,
                         reptile.getChild('../mammal/cat'))

    def test_path(self):
        expectedRootPath = []
        root = nap.Object('root')
        self.assertEqual(expectedRootPath, root.path(),
                         'Expected "%s", got "%s"' % (
                             expectedRootPath, root.path()))
        self.assertEqual(root.pathStr(), '/')

        expectedChildPath = ['child']
        child = nap.Object('child')
        root.addChild(child)
        self.assertEqual(expectedChildPath, child.path(),
                         'Expected "%s", got "%s"' % (
                             expectedChildPath, child.path()))
        self.assertEqual(child.pathStr(), '/child')

        expectedGrandChildPath = ['child', 'grandChild']
        grandChild = nap.Object('grandChild')
        child.addChild(grandChild)
        self.assertEqual(expectedGrandChildPath, grandChild.path(),
                         'Expected "%s", got "%s"' % (
                             expectedGrandChildPath, grandChild.path()))
        self.assertEqual(grandChild.pathStr(), '/child/grandChild')

    def test_createOperators(self):
        patch = nap.Patch()
        for opType in nap.operatorTypes():
            patch.addOperator(opType)


class OperatorTests(TestCase):
    def test_operator(self):
        patch = nap.Patch()
        op = patch.addOperator(TestOperator)
        self.assertTrue(isinstance(op, nap.Operator))
        inlets = list(op.inlets())
        self.assertTrue(inlets)
        self.assertEqual(inlets[0].name(), 'myInlet')
        self.assertEqual(inlets[1].name(), 'myInTrigger')

    def test_fileops(self):
        fileoptype = nap.operatorType('SplitExt')
        op = fileoptype()


class SerializerTests(TestCase):
    def setUp(self):
        self.maxDiff = None

    def test_classname(self):
        self.assertEqual(nap.Object.typeName(), 'nap.Object')
        self.assertEqual(TestOperator.typeName(), 'tests.TestOperator')

    def test_serializer(self):
        obj = animalTree()
        serialized = nap.dumps(obj)
        self.assertIsNotNone(serialized)
        obj2 = nap.loads(serialized)
        self.assertIsNotNone(obj2)
        serialized2 = nap.dumps(obj2)
        self.assertEqual(serialized, serialized2)

    def test_serializeAllOperators(self):
        patch = nap.Patch()
        for op in nap.operatorTypes():
            patch.addOperator(op)

        serialized = nap.dumps(patch)
        self.assertIsNotNone(serialized)
        obj2 = nap.loads(serialized)
        self.assertIsNotNone(obj2)
        serialized2 = nap.dumps(obj2)
        self.assertEqual(serialized, serialized2)

    def test_runPatch(self):
        patch = testPatch()
        self.assertTrue(isinstance(patch.getChild('Concat'), nap.Operator))
        self.assertTrue(isinstance(patch.getChild('Concat/a'), nap.DataInlet))
        self.assertTrue(isinstance(patch.getChild('Concat/b'), nap.DataInlet))
        self.assertTrue(isinstance(patch.getChild('Concat/result'), nap.DataOutlet))
        self.assertEqual(patch.getChild('Concat/a').value(), 'Hello, ')
        self.assertEqual(patch.getChild('Concat/b').value(), 'World!')
        self.assertEqual(patch.getChild('Concat/result')(), 'Hello, World!')

    def test_serializePatch(self):
        patch = testPatch()
        serialized = nap.dumps(patch)
        print(serialized)
    #     self.assertIsNotNone(serialized)
    #     obj2 = nap.loads(serialized)
    #     self.assertIsNotNone(obj2)
    #     serialized2 = nap.dumps(obj2)
    #     self.assertEqual(serialized, serialized2)
