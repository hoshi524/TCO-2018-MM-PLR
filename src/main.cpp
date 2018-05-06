#include <bits/stdc++.h>
#include <sys/time.h>
using namespace std;

class Timer {
 public:
  void restart();
  double getElapsed();

  Timer();

 private:
  static double rdtsc_per_sec_inv;

  double getTimeOfDay();
  unsigned long long int getCycle();

  double start_time;
  unsigned long long int start_clock;
};
double Timer::rdtsc_per_sec_inv = -1;

inline double Timer::getElapsed() {
  if (rdtsc_per_sec_inv != -1)
    return (double)(getCycle() - start_clock) * rdtsc_per_sec_inv;

  const double RDTSC_MEASUREMENT_INTERVAL = 0.1;
  double res = getTimeOfDay() - start_time;
  if (res <= RDTSC_MEASUREMENT_INTERVAL) return res;

  rdtsc_per_sec_inv = 1.0 / (getCycle() - start_clock);
  rdtsc_per_sec_inv *= getTimeOfDay() - start_time;
  return getElapsed();
}

inline void Timer::restart() {
  start_time = getTimeOfDay();
  start_clock = getCycle();
}

Timer::Timer() { restart(); }

inline double Timer::getTimeOfDay() {
  timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec + tv.tv_usec * 0.000001;
}

inline unsigned long long int Timer::getCycle() {
  unsigned int low, high;
  __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
  return ((unsigned long long int)low) | ((unsigned long long int)high << 32);
}

Timer timer;

inline unsigned get_random() {
  static unsigned y = 2463534242;
  return y ^= (y ^= (y ^= y << 13) >> 17) << 5;
}

constexpr float TIME_LIMIT = 1.5;
constexpr int MAX_S = 1 << 8;
constexpr int MAX_R = MAX_S * MAX_S / 8;
constexpr int MAX_N = 1 << 12;
constexpr int MAX_C = 1 << 3;

int H, W, X, X_;
int regions[MAX_S][MAX_S];
int oldColors[MAX_S][MAX_S];
int result[MAX_R];
bool connect[MAX_N][MAX_N];
int edge[MAX_N][1 << 6];
int edge_[MAX_N][1 << 6];
int colorBit[MAX_N];
int nmap[MAX_N];
int rmap[MAX_N];
int nlist[MAX_N];

void mapping(int c) {
  static bool use[MAX_N];
  memset(use, true, sizeof(use));
  bool ok = true;
  while (ok) {
    ok = false;
    X_ = 0;
    for (int i = 0; i < X; ++i) {
      if (use[i]) {
        nmap[i] = X_;
        rmap[X_] = i;
        ++X_;
      } else {
        nmap[i] = -1;
      }
    }
    for (int i = 0; i < X_; ++i) {
      int mi = rmap[i];
      edge_[i][0] = 0;
      for (int j = 1; j <= edge[mi][0]; ++j) {
        int mj = edge[mi][j];
        if (use[mj]) {
          edge_[i][++edge_[i][0]] = nmap[mj];
        }
      }
      if (edge_[i][0] < c) {
        use[mi] = false;
        ok = true;
      }
    }
  }
  for (int i = 0; i < X_; ++i) {
    nlist[i] = i;
  }
}

namespace state {
void set(int i, int c) {
  result[i] = c;
  for (int j = 1; j <= edge_[i][0]; ++j) colorBit[edge_[i][j]] &= ~(1 << c);
}
}  // namespace state

class MapRecoloring {
 public:
  vector<int> recolor(int H_, vector<int> regions_, vector<int> oldColors_) {
    H = H_;
    W = regions_.size() / H;
    X = 0;
    for (int i = 0; i < H; ++i) {
      for (int j = 0; j < W; ++j) {
        regions[i][j] = regions_[i * W + j];
        oldColors[i][j] = oldColors_[i * W + j];
        if (X < regions[i][j] + 1) X = regions[i][j] + 1;
      }
    }
    memset(connect, false, sizeof(connect));
    memset(edge, 0, sizeof(edge));
    for (int i = 0; i < H; ++i) {
      for (int j = 0; j < W; ++j) {
        auto add = [&](int a, int b) {
          if (a == b or connect[a][b]) return;
          connect[a][b] = true;
          connect[b][a] = true;
          edge[a][++edge[a][0]] = b;
          edge[b][++edge[b][0]] = a;
        };
        if (i > 0) add(regions[i][j], regions[i - 1][j]);
        if (j > 0) add(regions[i][j], regions[i][j - 1]);
      }
    }
    for (int i = 0; i < X; ++i) {
      sort(edge[i] + 1, edge[i] + edge[i][0] + 1);
    }
    int cs = MAX_C + 1;
    mapping(cs - 1);
    int cur[MAX_R];
    mt19937 engine(get_random());
    do {
      shuffle(nlist, nlist + X_, engine);
      int nc = cs - 1;
      for (int i = 0; i < X_; ++i) colorBit[i] = (1 << nc) - 1;
      for (int i = 0; i < X_; ++i) {
        int n, nv = 0xff;
        for (int j = i; j < X_; ++j) {
          int t = bitset<MAX_C>(colorBit[nlist[j]]).count();
          if (t == 0) goto OUTER;
          if (nv > t) {
            nv = t;
            n = j;
          }
        }
        swap(nlist[n], nlist[i]);
        for (int c = 0; c < nc; ++c) {
          if (colorBit[nlist[i]] & (1 << c)) {
            state::set(nlist[i], c);
            break;
          }
        }
      }
      memset(cur, -1, sizeof(cur));
      for (int i = 0; i < X_; ++i) {
        cur[rmap[i]] = result[i];
      }
      cs = nc;
      mapping(cs - 1);
    OUTER:;
    } while (timer.getElapsed() < TIME_LIMIT);
    {
      int e = 0;
      for (int i = 0; i < X; ++i) {
        if (cur[i] == -1) ++e;
      }
      while (e--) {
        int ti = -1, bc = 0xff, b = -1;
        for (int i = 0; i < X; ++i) {
          if (cur[i] == -1) {
            int bit = (1 << MAX_C) - 1;
            for (int j = 1; j <= edge[i][0]; ++j) {
              int c = cur[edge[i][j]];
              if (c != -1) bit &= ~(1 << c);
            }
            int tbc = bitset<MAX_C>(bit).count();
            if (bc > tbc) {
              bc = tbc;
              ti = i;
              b = bit;
            }
          }
        }
        for (int c = 0; c < MAX_C; ++c) {
          if (b & (1 << c)) {
            cur[ti] = c;
            break;
          }
        }
      }
    }
    vector<int> ret(X);
    for (int i = 0; i < X; ++i) ret[i] = cur[i];
    return ret;
  }
};

// -------8<------- end of solution submitted to the website -------8<-------
template <class T>
void getVector(vector<T>& v) {
  for (int i = 0; i < v.size(); ++i) cin >> v[i];
}

int main() {
  MapRecoloring mr;
  int H, S, R;
  cin >> H >> S;
  vector<int> regions(S);
  getVector(regions);
  cin >> R;
  vector<int> oldColors(R);
  getVector(oldColors);

  vector<int> ret = mr.recolor(H, regions, oldColors);
  cout << ret.size() << endl;
  for (int i = 0; i < (int)ret.size(); ++i) cout << ret[i] << endl;
  cout.flush();
}
