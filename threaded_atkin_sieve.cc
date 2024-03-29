#include <algorithm>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

using uint = unsigned int;
// using ulong = unsigned long;
using size_x = long;
using ContType = std::vector<size_x>;

using namespace std;


size_x sqrt_size_x(size_x n) {
    /*
    if (n <= 0) {
        throw runtime_error("got " + to_string(n));
    }
    */  
    int log2 = 0;
    auto init = n;
    while (n>>=1) {
        ++log2;
    }
    int initGuessLog2 = log2 >> 1;
    size_x guess = 1;
    guess <<= initGuessLog2;
    size_x prevGuess = -999;
    int times = 0;
    while(times <= 1) {
        prevGuess = guess;
        guess = (prevGuess + init/prevGuess) >> 1;
        if (guess == prevGuess) {
            return guess;
        }
        if (abs(prevGuess - guess) == 1) {
            ++times;
        }
    }
    return min(guess, prevGuess);
}

inline size_x init_y_for_n1(size_x lower, size_x _4x2, size_x& t2) {
    t2 = lower - _4x2;
    auto y = t2 >= 9 ? sqrt_size_x(t2) : 1;
    if (y % 2 == 0) {
        y += 1;
	}
    auto y2 = y*y;
    while (y2 < t2) {
        y += 2;
        y2 = y*y;
	}
    return y;
}



void do_n1(
    size_x lower, size_x upper, size_x x, vector<bool>& sieve,
    size_x _4x2, size_x end, int myid=-1
) {
    /*
    auto _4x2 = 4*x*x;
    auto end = upper - _4x2;
    if (end <= 0) {
       return;
    }
    */
    size_x t2 = lower - _4x2;
    auto y = t2 >= 9 ? sqrt_size_x(t2) : 1;
    if (y % 2 == 0) {
        y += 1;
	}
    auto y2 = y*y;
    while (y2 < t2) {
        y += 2;
        y2 = y*y;
	}
    size_x j2 = 0;
    if (x % 3 > 0) {
        while (y2 <= end) {
            j2 = y2 - t2;
            sieve[j2] = ! sieve[j2];
            y += 2;
            y2 = y*y;
		}
	}
    else {
        if (y % 3 == 0) {
            y += 2;
            y2 = y*y;
		}
        while (y2 <= end) {
            j2 = y2 - t2;
            sieve[j2] = ! sieve[j2];
            y += y % 3 == 1 ? 4 : 2;
            y2 = y*y;
        }
    }
}


void do_n1_x_mod_3_is_0(
    vector<bool>& sieve, size_x y, size_x end, size_x t2, int myid=-1
) {
    auto y2 = y*y;
    size_x j2 = 0;
    if (y % 3 == 0) {
        y += 2;
        y2 = y*y;
	}
    while (y2 <= end) {
        j2 = y2 - t2;
        sieve[j2] = ! sieve[j2];
        y += y % 3 == 1 ? 4 : 2;
        y2 = y*y;
    }
}


void do_n1_x_mod_3_is_not_0(
    vector<bool>& sieve, size_x y, size_x end, size_x t2, int myid=-1
) {
    auto y2 = y*y;
    auto j2 = 0;
    while (y2 <= end) {
        j2 = y2 - t2;
        sieve[j2] = ! sieve[j2];
        y += 2;
        y2 = y*y;
	}
}



void do_n2_n3(size_x lower, size_x upper, size_x x, vector<bool>& sieve, int myid=-1) {
    size_x _3x2 = 3*x*x;
    size_x t2 = lower - _3x2;
    auto end = upper - _3x2;
    size_x j2 = 0;
    if (end > 0 && x % 2 == 1) {
        size_x y = t2 >= 9 ? sqrt_size_x(t2) : 2;
        if (y % 2 == 1) {
            y += 1;
		}
        if (y*y < t2) {
            y += 2;
		}
        if (y % 6 == 0) {
            y += 2;
		}
        auto y2 = y*y;
        while (y2 <= end) {
            j2 = y2 - t2;
            sieve[j2] = ! sieve[j2];
            y = y % 3 == 2 ? y + 2 : y + 4;
            y2 = y*y;
        }
	}
    size_x low_limit = _3x2 - upper;
    size_x y = low_limit > 9 ? sqrt_size_x(low_limit) : 1;
    if (y < x) {
        if ( (x % 2 + y % 2) % 2 == 0) {
            y += 1;
		}
        if (y*y < low_limit) {
            y += 2;
		}
        if (y % 3 == 0) {
            y += 2;
		}
        if (y < x) {
            auto y2 = y*y;
            auto up_limit = -t2;
            while (y < x && y2 <= up_limit) {
                j2 = -(t2 + y2);
                sieve[j2] = ! sieve[j2];
                y += y % 3 == 1 ? 4 : 2;
			    y2 = y*y;
		    }
        }
	}
}


void do_r(size_x lower, size_x upper, vector<bool>& sieve, int myid=-1) {
    size_x r = 5;
    auto r2 = r*r;
    while (r2 <= upper) {
        auto start = r2 * (size_x(lower/r2) + 1);
        if (start % 2 == 0) {
            start = start - r2 > 0 ? start - r2 : start + r2;
        }
        while (start < lower) {
             start += 2*r2;
        }
        for (auto i=start; i<=upper; i+=2*r2) {
            sieve[i - lower] = false;
		}
        r += 2;
        r2 = r*r;
	}
}


ContType trueIndices(size_x lower, const vector<bool>& sieve) {
    ContType p;
	auto it = sieve.begin();
	while (
        (
            it = find_if(
                it, sieve.end(), [&] (bool const &e) {
		            return e;
	            }
            )
        ) != sieve.end()
    ) {
		p.push_back(distance(sieve.begin(), it) + lower);
		++it;
	}
    return p;
}


template <class T> ostream& operator<<(ostream& os, const vector<T>& elements) {
    os << "[";
    for (uint i=0; i<elements.size(); ++i) {
        os << elements[i];
        if (i < elements.size() - 1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

template <class T> ostream& operator<<(ostream& os, const vector<vector<T>>& v) {
    os << "[";
    for (uint j=0; j<v.size(); ++j) {
        for (uint i=0; i<v[j].size(); ++i) {
            os << v[j][i];
            if (i < v[j].size() - 1 || j < v.size() - 1) {
                os << ", ";
            }
        }
    }
    os << "]";
    return os;
}




ContType sieve_of_atkin_loops(size_x lower, size_x upper, int myid=-1) {
    vector<bool> sieve(upper - lower + 1, false);
    size_x x = 1;
    size_x x2 = x*x;
    size_x _4x2 = 4*x2;
    size_x end = upper - _4x2;
    size_x t2 = 0;
    size_x j2 = 0;
    size_x y = 1;
    size_x y2 = 1;
    for (size_x i=1; i<4; ++i) {
        if (i == 3) {
            for (size_x k=1; k<3; k++) {
                x = 3;
                _4x2 = 4*x*x;
                while (_4x2 < upper) {
                    end = upper - _4x2;
                    y = init_y_for_n1(lower, _4x2, t2);
                    while ((y % 3) != k) {
                        y += 2;
	                }
                    y2 = y*y;
                    while (y2 <= end) {
                        j2 = y2 - t2;
                        sieve[j2] = ! sieve[j2];
                        y += 6;
                        y2 = y*y;
                    }
                    x += 3;
                    _4x2 = 4*x*x;
                }
            }
        }
        else {
            x = i;
             _4x2 = 4*x*x;
            while (_4x2 < upper) {
                end = upper - _4x2;
                y = init_y_for_n1(lower, _4x2, t2);
                y2 = y*y;
                j2 = 0;
                while (y2 <= end) {
                    j2 = y2 - t2;
                    sieve[j2] = ! sieve[j2];
                    y += 2;
                    y2 = y*y;
	            }
                x += 3;
                _4x2 = 4*x*x;
            }
        }
    }
    size_x _3x2 = 0;
    for (size_x k=2; k<6; k+=2) {
        x = 1;
        _3x2 = 3*x*x;
        while (_3x2 < upper) {
            t2 = lower - _3x2;
            end = upper - _3x2;
            y = t2 >= 9 ? sqrt_size_x(t2) : k;
            if (y % 2 == 1) {
                y += 1;
		    }
            if (y*y < t2) {
                y += 2;
		    }
            while ((y % 6) != k) {
                y += 2;
            }
            y2 = y*y;
            while (y2 <= end) {
                j2 = y2 - t2;
                sieve[j2] = ! sieve[j2];
                y += 6;
                y2 = y*y;
            }
            x += 2;
            _3x2 = 3*x*x;
        }
    }
    size_x low_limit = 0;
    for (size_x k=1; k<3; ++k) {
        x = 1;
        x2 = 1;
        while (x2 <= upper) {
            _3x2 = 3*x2;
            t2 = _3x2 - lower;
            low_limit = _3x2 - upper;
            y = low_limit > 9 ? sqrt_size_x(low_limit) : 1;
            if (y < x) {
                if ( (x % 2 + y % 2) % 2 == 0) {
                    y += 1;
		        }
                if (y*y < low_limit) {
                    y += 2;
		        }
                while (y % 3 != k) {
                    y += 2;
                }
                y2 = y*y;
                while (y < x && y2 <= t2) {
                    j2 = t2 - y2;
                    sieve[j2] = ! sieve[j2];
                    y += 6;
			        y2 = y*y;
		        }
	        }
            x += 1;
            x2 = x*x;
        }
    }
    do_r(lower, upper, sieve, myid);
    return trueIndices(lower, sieve);
}


vector<ContType> do_pools(size_x lower, size_x upper, uint ncores) {
    auto n = upper - lower;
    if (n < ncores) {
        ncores = n;
    }
    auto inc = size_x(n/ncores);
    thread t[ncores];
    future<ContType> f[ncores];
    vector<ContType> result;
    size_x s0 = 0;
    size_x e0 = 0;
    for (unsigned int i=0; i<ncores; ++i) {
        s0 = i == 0 ? lower : size_x(i*inc) + lower;
        e0 = i == ncores - 1 ? upper : size_x((i+1)*inc) + lower - 1;
        packaged_task<ContType(size_x, size_x, int)> task{sieve_of_atkin_loops};
        f[i] = task.get_future();
        t[i] = thread{std::move(task), s0, e0, i};
    }
    for (unsigned int i=0; i<ncores; ++i) {
        t[i].join();
        result.push_back(f[i].get());
        // result.insert(result.end(), v.begin(), v.end());
    }
    return result;
}


ContType primes {2, 3, 5, 7, 11, 13, 17, 19, 23};

vector<ContType> sieve_of_atkin(size_x lower, size_x upper, uint ncores=12, bool update=true) {
    if (lower < 2) {
        throw runtime_error("lower cannot be less than 2");
    }
    auto last = *(primes.end() - 1);
    if (upper < last) {
        auto low_idx = lower_bound(primes.begin(), primes.end(), lower);
        auto high_idx = upper_bound(primes.begin(), primes.end(), upper);
        return vector<ContType> { ContType(low_idx, high_idx) };
    }
    auto orig_lower = lower;
    ContType r;
    if (lower < last /* or update */) {
        auto low_idx = lower_bound(primes.begin(), primes.end(), orig_lower);
        r = ContType(low_idx, primes.end()); 
        lower = last + 2;
    }
    vector<ContType> p;
    if (ncores == 1) {
        p.push_back(sieve_of_atkin_loops(lower, upper));
    }
    else {
        p = do_pools(lower, upper, ncores);
    }
    if (r.size() != 0) {
        p.insert(p.begin(), r);
    }
    return p;
    /*
    if (update) {
        primes.insert(primes.end(), p.begin(), p.end());
        auto low_idx = lower_bound(primes.begin(), primes.end(), orig_lower);
        auto high_idx = upper_bound(primes.begin(), primes.end(), upper);
        return ContType(low_idx, high_idx);
    }
    else {
        auto *q = &p;
        auto r = shared_ptr<ContType>(new ContType());
        if (orig_lower < last) {
            *r = primes;
            r->insert(r->end(), p.begin(), p.end());
            q = r.get();
        }
        auto low_idx = lower_bound(q->begin(), q->end(), lower);
        auto high_idx = upper_bound(q->begin(), q->end(), upper);
        return ContType(low_idx, high_idx);
    }
    */
}



int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        throw runtime_error("Incorrect number of parameters. Application takes two or three");
    }
    size_x lower = stol(argv[1]);
    size_x upper = stol(argv[2]);
    uint ncores = 12;
    if (argc == 4) {
        ncores = stoi(argv[3]);
    }
    auto p = sieve_of_atkin(lower, upper, ncores);
    size_x size = 0;
    for (const auto &r: p) {
        size += r.size();
    }
    cout << size << endl;
    if (size <= 100) {
        cout << p << endl;
    }
    /*
    cout << "begin sqrt" << endl;
    for (size_x i = 1000000000; i<2000000000; ++i) {
        auto k = int(sqrt(i));
    }
    cout << "end sqrt" << endl;
    cout << "begin sqrt_size_x" << endl;
    for (size_x i = 1000000000; i<2000000000; ++i) {
        auto sq = sqrt_size_x(i);
    }
    cout << "end sqrt_size_x" << endl;
    */
    return 0;
}



