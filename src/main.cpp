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
constexpr int MAX_R = MAX_S * MAX_S / 8;
constexpr int MAX_N = 1 << 12;
constexpr int MAX_C = 1 << 3;

int H, W, X;
int regions[MAX_S][MAX_S];
int regionsColor[MAX_N][MAX_C];
int oldColors[MAX_S][MAX_S];
int result[MAX_R];
int edge[MAX_N][1 << 6];
int colorBit[MAX_N];
int nlist[MAX_N];
int colorOrder[MAX_N][MAX_C];
mt19937 engine(get_random());

namespace state {
void set(int r, int c) {
  result[r] = c;
  for (int j = 1; j <= edge[r][0]; ++j) colorBit[edge[r][j]] &= ~(1 << c);
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
    static int ans[MAX_R];
    int value = INT_MAX;
    for (int i = 0; i < X; ++i) {
      nlist[i] = i;
    }
    while (timer.getElapsed() < TIME_LIMIT) {
      [&]() {
        int nc = 6 + (get_random() & 1);
        for (int i = 0; i < X; ++i) {
          colorBit[i] = (1 << nc) - 1;
        }
        shuffle(nlist, nlist + X, engine);
        for (int i = 0; i < X; ++i) {
          int n, nv = 0xff;
          for (int j = i; j < X; ++j) {
            int t = bitset<MAX_C>(colorBit[nlist[j]]).count();
            if (t == 0) return;
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
        {
          int v = nc * 5000;
          for (int i = 0; i < X; ++i) {
            v += regionsColor[i][result[i]];
          }
          if (value > v) {
            value = v;
            memcpy(ans, result, sizeof(ans));
          }
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