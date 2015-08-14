from matplotlib import pyplot as plt
from collections import defaultdict
from math import log
from scipy.stats import linregress
import numpy as np
from itertools import chain, imap
concat = chain.from_iterable

def mapcat(infn, inl):
    return concat(imap(infn, inl))

def chunks(l, n):
    n = max(1, n)
    ltest = (l[i:i + n] for i in range(0, len(l), n))
    return [lv for lv in ltest if len(lv) == n]

def grp_b(inl):
    rdict = defaultdict(lambda: [])
    for (k, v) in inl:
        rdict[k].append(v)
    return rdict

def trans_dicts(mainlist):
    def mapfn(val):
        (k, v) = val
        rval = grp_b(concat(v))
        newval = [(nk, np.mean(nv)) for (nk, nv) in rval.iteritems()]
        return (k, newval)
    return map(mapfn, mainlist)


def proc_chunks(inc):
    lines = int(inc[0].split()[1])
    def getc(inl):
        impl, implt = inl.split()[:2]
        return (impl, float(implt))
    return (lines, map(getc, inc[1:]))

def read_copy(inf):
    infl = list(inf)
    lchunks = chunks(infl, 6)
    segs = map(proc_chunks, lchunks)
    segs = trans_dicts(grp_b(segs).iteritems())
    print segs[:1]
    vals = [x[1] for x in segs]
    sizes = np.array([x[0] for x in segs])
    sizes = (sizes)
    sarg = np.argsort(sizes)
    sizes = sizes[sarg]
    typedct = defaultdict(lambda: [])
    for vlist in vals:
        for v in vlist:
            typedct[v[0]].append(v[1])

    lines = []
    for (outn, outl) in typedct.items():
        outl = (np.asarray(outl)[sarg])
        cline, = plt.plot(sizes, outl, label=outn)
        lines.append(cline)
        plt.xlabel('Number of trees being accessed')
        plt.ylabel('mean ns per tree lookup')
        plt.ylim(0, 150)
        plt.title('Access time per lookup vs number of trees being accessed\nfor various allocation schemes', multialignment='center')
        plt.legend(lines, typedct.keys(), loc='upper left')
    plt.show()

def doread():
    with open("../testout.txt") as inf:
        read_copy(inf)

if __name__ == "__main__":
    doread()
