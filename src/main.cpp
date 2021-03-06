#include <bits/stdc++.h>
#include <sys/time.h>
using namespace std;

constexpr double ticks_per_sec = 2800000000;
constexpr double ticks_per_sec_inv = 1.0 / ticks_per_sec;
inline double rdtsc() {
  uint32_t lo, hi;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return (((uint64_t)hi << 32) | lo) * ticks_per_sec_inv;
}

inline unsigned get_random() {
  static unsigned y = 2463534242;
  return y ^= (y ^= (y ^= y << 13) >> 17) << 5;
}

constexpr float TIME_LIMIT = 9;
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
uint8_t colorCount[MAX_N][MAX_C];
uint8_t colorBit[MAX_N];
uint16_t regions[MAX_S][MAX_S];
uint16_t edge[MAX_N][MAX_E];
uint16_t nlist[MAX_N];
mt19937 engine(get_random());

namespace state {
void set(int r, int c) {
  color[r] = c;
  for (int i = 1; i <= edge[r][0]; ++i) {
    int s = edge[r][i];
    colorCount[s][c]++;
    colorBit[s] &= ~(1 << c);
  }
}
void reset(int r) {
  int c = color[r];
  for (int i = 1; i <= edge[r][0]; ++i) {
    int s = edge[r][i];
    if (--colorCount[s][c] == 0) colorBit[s] &= ~(1 << c);
  }
  color[r] = -1;
}
}  // namespace state

class MapRecoloring {
 public:
  vector<int> recolor(int H_, const vector<int>& regions_,
                      const vector<int>& oldColors_) {
    double start = rdtsc();
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
    constexpr int COLOR = 100000;
    int value = INT_MAX;
    for (int i = 0; i < X; ++i) nlist[i] = i;
    for (int z = 0; z < 100 or value == INT_MAX; ++z) {
      [&]() {
        int nc = value == INT_MAX ? 7 : 6;
        memset(colorBit, (1 << nc) - 1, sizeof(colorBit));
        memset(colorCount, 0, sizeof(colorCount));
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
    {
      int cv = value;
      int nc = value / COLOR;
      {
        memset(colorBit, (1 << nc) - 1, sizeof(colorBit));
        memset(colorCount, 0, sizeof(colorCount));
        for (int i = 0; i < X; ++i) {
          state::set(i, ans[i]);
        }
      }
      constexpr int LOG_SIZE = 1 << 10;
      int log_[LOG_SIZE];
      while (true) {
        float time = TIME_LIMIT - (rdtsc() - start);
        if (time < 0) break;
        for (int i = 0; i < LOG_SIZE; ++i) {
          log_[i] = -30 * log((i + 0.5) / LOG_SIZE) * time / TIME_LIMIT;
        }
        for (int i = 0; i < 0xff; ++i) {
          static int queue[2][2];
          int qs = 0;
          int v = 0;
          auto add = [&](int r, int c) {
            v += regionsColor[r][c] - regionsColor[r][color[r]];
            queue[qs][0] = r;
            queue[qs][1] = c;
            ++qs;
          };
          if (get_random() & 1) {
            int r = get_random() % X;
            int c = get_random() % nc;
            if (c == color[r] or colorCount[r][c] > 0) continue;
            add(r, c);
          } else {
            int r0 = get_random() % X;
            int r1 = edge[r0][1 + get_random() % edge[r0][0]];
            int c0 = color[r0];
            int c1 = color[r1];
            if (colorCount[r0][c1] > 1 or colorCount[r1][c0] > 1) continue;
            add(r0, c1);
            add(r1, c0);
          }
          if (v <= log_[get_random() & (LOG_SIZE - 1)]) {
            cv += v;
            for (int i = 0; i < qs; ++i) {
              int r = queue[i][0];
              int c = queue[i][1];
              state::reset(r);
              state::set(r, c);
            }
            if (value > cv) {
              value = cv;
              memcpy(ans, color, sizeof(ans));
            }
          }
        }
      }
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