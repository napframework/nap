import nap
if __name__ == '__main__':
    print("Members of nap (%s)" % nap)
    for k in sorted(nap.__dict__.keys()):
        print(('\t%s ' % k).ljust(30, '.'), nap.__dict__[k])