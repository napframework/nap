import json
with open('module.json') as f:
    x = json.load(f)
    print(x)
    print(type(x['Type']))
    print(type(x['Type']) == bytes)

