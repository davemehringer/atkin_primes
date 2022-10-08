import bisect
from functools import reduce
from operator import mul
import math
import dask, dask.distributed
from multiprocessing import Pool
from time import perf_counter, sleep

__orig_primes = [2, 3, 5, 7, 11, 13, 17, 19, 23]

__primes = __orig_primes[:]


# __loop_counts = []

def reset():
    """Useful for debugging"""
    global __orig_primes
    global __primes
    __primes = __orig_primes[:]

def __do_n1(lower, upper, x, sieve, myid=None):
    _4x2 = 4*x*x
    end = upper - _4x2
    if end <= 0:
        return
    t2 = lower - _4x2
    y = int(math.sqrt(t2)) if t2 >= 9 else 1
    if y % 2 == 0:
        y += 1
    y2 = y*y
    while y2 < t2:
        y += 2
        y2 = y*y
    if x % 3 > 0:
        while y2 <= end:
            sieve[y2 - t2] ^= True
            y += 2
            y2 = y*y
    else:
        if y % 3 == 0:
            y += 2
            y2 = y*y
        while y2 <= end:
            sieve[y2 - t2] ^= True
            y += 4 if y % 3 == 1 else 2
            y2 = y*y


def __do_n2(lower, upper, x, sieve):
    _3x2 = 3*x*x
    y = 1 if x % 2 == 0 else 2
    y2 = y*y
    while y2 < upper:
        n2 = _3x2 + y2
        if n2 >= lower and n2 <= upper and n2 % 12 == 7:
            sieve[n2 - lower] ^= True
        y += 2
        y2 = y*y


def __do_n3(lower, upper, x, sieve):
    _3x2 = 3*x*x
    y = 1 if x % 2 == 0 else 2
    y2 = y*y
    while y2 < upper:
        if x > y:
            n3 = _3x2 - y2
            if n3 >= lower and n3 <= upper and n3 % 12 == 11:
                sieve[n3 - lower] ^= True
        y += 2
        y2 = y*y


def __do_n2_n3(lower, upper, x, sieve, myid=None, tm=None):
    _3x2 = 3*x*x
    t2 = lower - _3x2
    end = upper - _3x2
    if end > 0 and x % 2 == 1:
        y = int(math.sqrt(t2)) if t2 >= 9 else 2
        if y % 2 == 1:
            y += 1
        if y*y < t2:
            y += 2
        if y % 6 == 0:
            y += 2
        y2 = y*y
        while y2 <= end:
            sieve[y2 - t2] ^= True
            y += 2 if y % 3 == 2 else 4
            y2 = y*y
    low_limit = _3x2 - upper
    y = int(math.sqrt(low_limit)) if low_limit >= 9 else 1
    if (x % 2 + y % 2) % 2 == 0:
        y += 1
    if y*y < low_limit:
        y += 2
    if y % 3 == 0:
        y += 2
    if y < x:
        y2 = y*y
        up_limit = -t2
        while y < x and y2 <= up_limit:
            sieve[-t2 - y2] ^= True
            y += 4 if y % 3 == 1 else 2
            y2 = y*y


def __do_n1_n2_n3(lower, upper, x, sieve):
    x2 = x*x
    _4x2 = 4*x2
    w = 1
    w2 = 1
    while w2 <= upper:
        n1 = _4x2 + w2
        if n1 >= lower and n1 <= upper and n1 % 12 in [1, 5]:
            sieve[n1 - lower] ^= True
        w += 2
        w2 = w*w
    _3x2 = 3*x2
    z = 1 if x % 2 == 0 else 2
    z2 = z*z
    while z2 < upper:
        n2 = _3x2 + z2
        if n2 >= lower and n2 <= upper and n2 % 12 == 7:
            sieve[n2 - lower] ^= True
        if x > z:
            n3 = _3x2 - z2
            if n3 >= lower and n3 <= upper and n3 % 12 == 11:
                sieve[n3 - lower] ^= True

        z += 2
        z2 = z*z


def __do_r(lower, upper, sieve, myid=None):
    # global __loop_counts
    r = 5
    r2 = r*r
    while r2 <= upper:
        start = r2 * (int(lower/r2) + 1)
        if start % 2 == 0:
            start = start - r2 if start - r2 > 0 else start + r2
        while start < lower:
            start += 2*r2
        for i in range(start, upper+1, 2*r2):
            # __loop_counts[myid] += 1
            sieve[i - lower] = False
        r += 2
        r2 = r*r


def __sieve_of_atkin_loops(lower, upper, myid=None):
    # tm = 4 * [0.0]
    # global __loop_counts
    sieve = (upper - lower + 1) * [False]
    x = 1
    x2 = x*x
    # t1 = 0
    while x2 <= upper:
        # __loop_counts[myid] += 1
        # start = perf_counter()
        __do_n1(lower, upper, x, sieve, myid)
        # tm[0] += perf_counter() - start
        # start = perf_counter()
        __do_n2_n3(lower, upper, x, sieve, myid)
        # tm[1] += perf_counter() - start
        x += 1
        x2 = x*x
    # start = perf_counter()
    __do_r(lower, upper, sieve, myid)
    # tm[2] += perf_counter() - start
    # start = perf_counter()
    p = []
    for i, b in enumerate(sieve):
        if b:
            p.append(lower + i)
    # tm[3] += perf_counter() - start
    # print(myid, sum(tm), tm)
    # print(myid, tm)
    return p

def __call_pairs(s0, e0, s1, e1, myid=None):
    p0 = __sieve_of_atkin_loops(s0, e0, myid)
    p1 = __sieve_of_atkin_loops(s1, e1, myid)
    return [p0, p1]


def __do_pools(lower, upper, ncores):
    # global __loop_counts
    # __loop_counts = ncores * [0]
    pool = Pool(processes=ncores)
    n = upper - lower
    # inc = int(n/ncores/2)
    inc = int(n/ncores)
    results = []
    # s = lower
    # start = perf_counter()
    for i in range(ncores):
        s0 = lower if i == 0 else int(i*inc) + lower
        e0 = upper if i == ncores - 1 else int((i+1)*inc) + lower - 1
        # e0 = int((upper + lower)/2) if i == ncores - 1 else int((i+1)*inc) + lower - 1
        # s1 = e0 + 1 if i == ncores - 1 else upper - e0 + lower
        # e1 = upper - s0 + lower
        # print(i, s0, e0, s1, e1)
        # parms = (s0, e0, s1, e1, i)
        # k = pool.apply_async(__call_pairs, args=parms)
        parms = (s0, e0, i)
        k = pool.apply_async(__sieve_of_atkin_loops, args=parms)

        results.append(k)
    pool.close()
    pool.join()
    # print(perf_counter() - start)
    p0 = []
    # p1 = []
    for r in results:
        p0.extend(r.get())
        # x = r.get()
        # p0.extend(x[0])
        # p1.append(x[1])
    # for i in range(len(p1) - 1, -1, -1):
        # p0.extend(p1[i])
    return p0


def __do_dask(lower, upper, ncores):
    client = dask.distributed.Client(threads_per_worker=1)
    dask_parallel = dask.delayed(__sieve_of_atkin_loops)
    ss = []
    n = upper - lower
    inc = int(n/ncores)
    for i in range(ncores):
        s = lower if i == 0 else int(i*inc) + lower
        e = upper if i == ncores - 1 else int((i+1)*inc) + lower - 1
        k = dask_parallel(s, e)
        ss.append(k)
        z = dask.delayed(ss)
    z = z.compute()
    client.close()
    p = []
    for r in z:
        p.extend(r)
    return p


def sieve_of_atkin(lower, upper, ncores=12, update=True):
    global __primes
    if lower < 2:
        raise ValueError('lower cannot be less than 2')
    if upper < __primes[-1]:
        low_idx = bisect.bisect_left(__primes, lower)
        high_idx = bisect.bisect_right(__primes, upper)
        return __primes[low_idx: high_idx]
    orig_lower = lower
    if lower < __primes[-1] or update:
        lower = __primes[-1] + 2
    if ncores == 1:
        p = __sieve_of_atkin_loops(lower, upper)
    else:
        p = __do_pools(lower, upper, ncores)
        start = perf_counter()
        # p = __do_dask(lower, upper, ncores)
    if update:
        __primes.extend(p)
        low_idx = bisect.bisect_left(__primes, orig_lower)
        high_idx = bisect.bisect_right(__primes, upper)
        return __primes[low_idx: high_idx]
    else:
        if orig_lower < __primes[-1]:
            p = __primes + p
        low_idx = bisect.bisect_left(p, orig_lower)
        high_idx = bisect.bisect_right(p, upper)
        return p[low_idx: high_idx]


def __fill_primes(n):
    global __primes
    for j in range(__primes[-1]+2, n+1, 2):
        prime = True
        for p in __primes:
            if j % p == 0:
                prime = False
                break
            if p*p > j:
                break
        if prime:
            __primes.append(j)

def brute_force(n):
    global __primes
    __fill_primes(n)
    return __primes


def is_prime(i, all_primes=False):
    global __primes
    if i > 2 and i % 2 == 0:
        return False
    if __primes[-1] > i:
        return False
    if i in __primes:
        return True
    n = (i + 1) if all_primes else int(math.sqrt(i) + 1)
    # __fill_primes(n)
    # if not all_primes:
    #    max_prime = primes[-1]
    sieve_of_atkin(__primes[-1] + 2, n)
    # if all_primes:
    return i in __primes
    """
    index = bisect.bisect(primes, max_prime*max_prime)
    for j in range(index, len(primes):
        if i % primes[j] == 0:
            return False
    return True
    """

def get_primes(i=None):
    global __primes
    if i is None:
        return __primes
    #__fill_primes(i)
    sieve_of_atkin(2, i)
    index = bisect.bisect(__primes, i)
    return __primes[:index]


def factors(i):
    myfact = []
    if is_prime(i):
        return []
    for p in __primes:
        if i % p == 0:
            if 3*p > i:
                break
            myfact.append(p)
            x = i//p
            if __primes[-1] < x:
                # fill in primes up to x
                is_prime(x, True)
            r = factors(x)
            if len(r) == 0 and x*x == i:
                myfact.append(x)
            if len(r) > 0:
                myfact.extend(r)
            if len(myfact) > 0 and reduce(mul, myfact) == i:
                break
    return myfact

