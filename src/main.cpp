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

constexpr float TIME_LIMIT = 2;
constexpr int MAX_S = 1 << 8;
constexpr int MAX_N = 1 << 12;
constexpr int MAX_C = 1 << 3;
constexpr int MAX_E = 1 << 6;

int H, W, X;
uint8_t regionsColor[MAX_N][MAX_C];
uint8_t oldColors[MAX_S][MAX_S];
uint8_t ans[MAX_N];
uint8_t color[MAX_N];
uint8_t colorOrder[MAX_N][MAX_C];
uint8_t colorBit[MAX_N];
uint16_t regions[MAX_S][MAX_S];
uint16_t edge[MAX_N][MAX_E];
uint16_t nlist[MAX_N];
mt19937 engine(get_random());

namespace state {
void set(int r, int c) {
  color[r] = c;
  colorBit[r] = 0;
  static uint16_t queue[MAX_N];
  int qs = 0;
  auto remove = [&](int r, int c) {
    uint8_t& x = colorBit[r];
    if (x & (1 << c)) {
      x ^= 1 << c;
      if (bitset<MAX_C>(x).count() == 2) queue[qs++] = r;
    }
  };
  for (int i = 1; i <= edge[r][0]; ++i) remove(edge[r][i], c);
  for (int i = 0; i < qs; ++i) {
    int r0 = queue[i];
    int b = colorBit[r0];
    auto removes = [&](vector<int>& v) {
      std::set<int> set;
      int f = v[0];
      for (int r : v) {
        std::set<int> next;
        for (int i = 1; i <= edge[r][0]; ++i) {
          int s = edge[r][i];
          if (colorBit[s] == 0) continue;
          if (f != r and set.find(s) == set.end()) continue;
          next.insert(s);
        }
        set = move(next);
      }
      for (int r : set) {
        for (int c = 0; c < MAX_C; ++c) {
          if (b & (1 << c)) remove(r, c);
        }
      }
    };
    int t = bitset<MAX_C>(b).count();
    if (t == 2) {
      for (int j = 1; j <= edge[r0][0]; ++j) {
        int r1 = edge[r0][j];
        if (b == colorBit[r1]) {
          vector<int> v{r0, r1};
          removes(v);
        }
      }
    }
  }
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
    memset(regionsColor, 0, sizeof(regionsColor));
    memset(edge, 0, sizeof(edge));
    {
      static bool connect[MAX_N][MAX_N];
      memset(connect, false, sizeof(connect));
      for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
          auto add = [&](int a, int b) {
            if (a == b or connect[a][b]) return;
            connect[a][b] = true;
            connect[b][a] = true;
            edge[a][++edge[a][0]] = b;
            edge[b][++edge[b][0]] = a;
            // assert(edge[a][0] < MAX_E);
            // assert(edge[b][0] < MAX_E);
          };
          if (i > 0) add(regions[i][j], regions[i - 1][j]);
          if (j > 0) add(regions[i][j], regions[i][j - 1]);
          for (int k = 0; k < MAX_C; ++k) {
            if (oldColors[i][j] == k) continue;
            regionsColor[regions[i][j]][k]++;
          }
        }
      }
    }
    for (int i = 0; i < X; ++i) {
      sort(edge[i] + 1, edge[i] + edge[i][0] + 1);
      for (int j = 0; j < MAX_C; ++j) colorOrder[i][j] = j;
      sort(colorOrder[i], colorOrder[i] + MAX_C, [&](int a, int b) {
        if (regionsColor[i][a] == regionsColor[i][b]) return a > b;
        return regionsColor[i][a] < regionsColor[i][b];
      });
    }
    constexpr int COLOR = 100000;
    int value = INT_MAX;
    for (int i = 0; i < X; ++i) nlist[i] = i;
    while (timer.getElapsed() < TIME_LIMIT or value == INT_MAX) {
      [&]() {
        int nc = 6 + (value < 7 * COLOR ? 0 : (get_random() & 1));
        memset(colorBit, (1 << nc) - 1, sizeof(colorBit));
        shuffle(nlist, nlist + X, engine);
        for (int i = 0; i < X; ++i) {
          int n, nv = 0xff;
          for (int j = i; j < X; ++j) {
            int b = colorBit[nlist[j]];
            if (b == 0) return;
            int t = bitset<MAX_C>(b).count();
            if (nv > t) {
              nv = t;
              n = j;
            }
          }
          swap(nlist[n], nlist[i]);
          int r = nlist[i];
          for (int j = 0; j < MAX_C; ++j) {
            int c = colorOrder[r][j];
            if (colorBit[r] & (1 << c)) {
              state::set(r, c);
              break;
            }
          }
        }
        int v = nc * COLOR;
        for (int i = 0; i < X; ++i) {
          v += regionsColor[i][color[i]];
        }
        if (value > v) {
          value = v;
          memcpy(ans, color, sizeof(ans));
        }
      }();
    }
    vector<int> ret(X);
    for (int i = 0; i < X; ++i) ret[i] = ans[i];
    return ret;
  }
};

// -------8<------- end of solution submitted to the website -------8<-------
template <class T>
void getVector(vector<T>& v) {
  int s = v.size();
  for (int i = 0; i < s; ++i) cin >> v[i];
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