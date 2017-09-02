import nap

NAP_MODULE_NAME = 'Core'

DATA_TYPES = [
    float,
    int,
    str,
]

class PatchComponent(nap.Component):
    def __init__(self):
        super(PatchComponent, self).__init__()