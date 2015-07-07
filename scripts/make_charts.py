from matplotlib import pyplot as plt
from collections import defaultdict
from math import log
from scipy.stats import linregress
import numpy as np

def chunks(l, n):
    n = max(1, n)
    ltest = (l[i:i + n] for i in range(0, len(l), n))
    return [lv for lv in ltest if len(lv) == n]

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
        slope, inter, r, p, s = linregress(np.log2(sizes), np.log2(outl))
        print slope, outn
        cline, = plt.plot(np.sqrt(sizes), outl, label=outn)
        lines.append(cline)
    plt.show()

def doread():
    with open("../testout.txt") as inf:
        read_copy(inf)
