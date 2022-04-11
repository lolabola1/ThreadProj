
def make_mask(i):
    acc = 0
    for j in range(i):
        acc = acc | (1 << j)
    return acc

def check_f_ordering(fname):
    fd = open(fname)
    ln = fd.readlines()[0]
    fd.close()
    ln = [int(a) for a in ln.strip().split(' ')]
    max_thread = max(ln)
    nthreads = max_thread+1
    tries = int(len(ln)/(max_thread+1)/2)
    mymask = make_mask(nthreads)
    for i in range(tries):
        sset = ln[i*nthreads*2:(i+1)*nthreads*2]
        mask = 0
        for j in sset[:nthreads]:
            mask = mask | (1 << j)
        if (mask != mymask):
            return False
        for j in sset[nthreads:]:
            mask = mask ^ (1 << j)
        if (mask != 0):
            return False
    return True

def check_lap():
    fd = open("laplace_out.txt")
    lns = fd.readlines()
    fd.close()
    fd = open("solutions/correct.txt")
    lns2 = fd.readlines()
    fd.close()
    return lns == lns2

def check_tests():
    try:
        sbt = check_f_ordering('sbt_out.txt')
    except Exception:
        sbt = False

    try:
        brt = check_f_ordering('brt_out.txt')
    except Exception:
        brt = False

    try:
        app = check_lap()
    except:
        app = False

    return sbt, brt, app

