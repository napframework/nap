import napclient


def printTypes():
    core = napclient.Core()
    print('Modules')
    for mod in core.modules():
        print(mod)
        print('\tData Types:')
        for t in core.dataTypes(mod):
            print('\t\t%s' % t)
        print('\tComponent Types:')
        for t in core.componentTypes(mod):
            print('\t\t%s' % t)
        print('\tOperator Types:')
        for t in core.operatorTypes(mod):
            print('\t\t%s' % t)

if __name__ == '__main__':
    # printTypes()
    core = napclient.Core()
    core.objectTree()
