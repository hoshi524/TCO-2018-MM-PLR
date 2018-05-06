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

int H, W, X, X_, C;
int regions[MAX_S][MAX_S];
int regionsColor[MAX_N][MAX_C];
int oldColors[MAX_S][MAX_S];
int result[MAX_R];
bool connect[MAX_N][MAX_N];
int edge[MAX_N][1 << 6];
int edge_[MAX_N][1 << 6];
int colorBit[MAX_N];
int nmap[MAX_N];
int rmap[MAX_N];
int nlist[MAX_N];
mt19937 engine(get_random());

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
  shuffle(nlist, nlist + X_, engine);
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
    C = 0;
    for (int i = 0; i < H; ++i) {
      for (int j = 0; j < W; ++j) {
        regions[i][j] = regions_[i * W + j];
        oldColors[i][j] = oldColors_[i * W + j];
        if (X < regions[i][j] + 1) X = regions[i][j] + 1;
        if (C < oldColors[i][j] + 1) C = oldColors[i][j] + 1;
      }
    }
    memset(regionsColor, 0, sizeof(regionsColor));
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
        for (int k = 0; k < MAX_C; ++k) {
          if (oldColors[i][j] == k) continue;
          regionsColor[regions[i][j]][k]++;
        }
      }
    }
    for (int i = 0; i < X; ++i) {
      sort(edge[i] + 1, edge[i] + edge[i][0] + 1);
    }
    int cs = MAX_C;
    int cur[MAX_R];
    int ans[MAX_R];
    int value = INT_MAX;
    do {
      [&]() {
        int nc = cs - (get_random() & 1);
        mapping(nc);
        for (int i = 0; i < X_; ++i) colorBit[i] = (1 << nc) - 1;
        for (int i = 0; i < X_; ++i) {
          int n, nv = 0xff;
          for (int j = i; j < X_; ++j) {
            int t = bitset<MAX_C>(colorBit[nlist[j]]).count();
            if (t == 0) return;
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
        {
          memset(cur, -1, sizeof(cur));
          for (int i = 0; i < X_; ++i) {
            cur[rmap[i]] = result[i];
          }
          X_ = 0;
          for (int i = 0; i < X; ++i) {
            if (cur[i] == -1) nlist[X_++] = i;
          }
          shuffle(nlist, nlist + X_, engine);
          for (int i = 0; i < X_; ++i) {
            int ti = -1, bc = 0xff, b = -1;
            for (int j = i; j < X_; ++j) {
              int r = nlist[j];
              int bit = (1 << nc) - 1;
              for (int j = 1; j <= edge[r][0]; ++j) {
                int c = cur[edge[r][j]];
                if (c != -1) bit &= ~(1 << c);
              }
              int t = bitset<MAX_C>(bit).count();
              if (t == 0) return;
              if (bc > t) {
                bc = t;
                ti = j;
                b = bit;
              }
            }
            swap(nlist[i], nlist[ti]);
            for (int c = 0; c < nc; ++c) {
              if (b & (1 << c)) {
                cur[nlist[i]] = c;
                break;
              }
            }
          }
        }
        {
          static int color[MAX_C];
          static int ans[MAX_C];
          static int map[MAX_C][MAX_C];
          int value, v = 0;
          memset(map, 0, sizeof(map));
          for (int i = 0; i < X; ++i) {
            for (int j = 0; j < nc; ++j) {
              map[cur[i]][j] += regionsColor[i][j];
            }
          }
          for (int i = 0; i < nc; ++i) {
            color[i] = ans[i] = i;
            v += map[i][color[i]];
          }
          value = v;
          for (int t = 0; t < 1000; ++t) {
            int a = get_random() % nc;
            int b = get_random() % nc;
            if (a == b) continue;
            if (color[a] >= C && color[b] >= C) continue;
            int p = map[a][color[a]] + map[b][color[b]];
            int n = map[a][color[b]] + map[b][color[a]];
            if (p >= n or (get_random() & 1)) {
              swap(color[a], color[b]);
              v += n - p;
              if (value > v) {
                value = v;
                memcpy(ans, color, sizeof(ans));
              }
            }
          }
          for (int i = 0; i < X; ++i) {
            cur[i] = ans[cur[i]];
          }
        }
        {
          while (true) {
            int tr, tc, tv = 0;
            for (int i = 0; i < X; ++i) {
              int bit = (1 << nc) - 1;
              for (int j = 1; j <= edge[i][0]; ++j) {
                bit &= ~(1 << cur[edge[i][j]]);
              }
              if (bit == 0) continue;
              for (int j = 0; j < nc; ++j) {
                if (bit & (1 << j)) {
                  if (regionsColor[i][cur[i]] > regionsColor[i][j]) {
                    int t = regionsColor[i][cur[i]] - regionsColor[i][j];
                    if (tv < t) {
                      tv = t;
                      tr = i;
                      tc = j;
                    }
                  }
                }
              }
            }
            if (tv == 0) break;
            cur[tr] = tc;
          }
        }
        {
          int v = nc * 5000;
          for (int i = 0; i < X; ++i) {
            v += regionsColor[i][cur[i]];
          }
          if (value > v) {
            value = v;
            memcpy(ans, cur, sizeof(ans));
          }
        }
        cs = nc;
      }();
    } while (timer.getElapsed() < TIME_LIMIT);

    vector<int> ret(X);
    for (int i = 0; i < X; ++i) ret[i] = ans[i];
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