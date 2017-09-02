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
    mammal.setParent(animal)
    cat = nap.Object('cat')
    cat.setParent(mammal)
    lion = nap.Object('lion')
    lion.setParent(cat)
    dog = nap.Object('dog')
    dog.setParent(mammal)
    reptile = nap.Object('reptile')
    reptile.setParent(animal)

    return animal


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
        child.setParent(parent)
        self.assertIn(child, parent.children())

    def test_uniqueNameParented(self):
        parent = nap.Object()
        child1 = nap.Object()
        child1.setParent(parent)
        child2 = nap.Object()
        child2.setParent(parent)
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
        child.setParent(parent)

        self.assertEqual(child.name(), childName,
                         'Child was renamed from "%s" to "%s" upon parenting' % (
                             childName, child.name()))

        self.assertIsNotNone(parent.getChild(childName),
                             'Expected child "%s"' % childName)
        self.assertIsNone(parent.getChild('NonExistent'), 'Unexpected child')

    def test_getChildByPath(self):
        tree = animalTree()

        lion = tree.getChild('mammal/cat/lion')
        self.assertIsNotNone(lion)

        reptile = tree.getChild('reptile')
        self.assertIsNotNone(reptile)

        cat = tree.getChild('mammal/cat')
        self.assertIsNotNone(cat)

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
        child.setParent(root)
        self.assertEqual(expectedChildPath, child.path(),
                         'Expected "%s", got "%s"' % (
                             expectedChildPath, child.path()))
        self.assertEqual(child.pathStr(), '/child')

        expectedGrandChildPath = ['child', 'grandChild']
        grandChild = nap.Object('grandChild')
        grandChild.setParent(child)
        self.assertEqual(expectedGrandChildPath, grandChild.path(),
                         'Expected "%s", got "%s"' % (
                             expectedGrandChildPath, grandChild.path()))
        self.assertEqual(grandChild.pathStr(), '/child/grandChild')
