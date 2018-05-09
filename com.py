import subprocess
import json
import threading
import queue
import json

subprocess.call('g++ --std=c++0x -W -Wall -O2 -s -pipe -mmmx -msse -msse2 -msse3 -o out/main.out src/main.cpp', shell=True)
subprocess.call('javac -d out src/MapRecoloringVis.java', shell=True)

scorefile = "best-score.json"
case = 100
scores = [0]

try:
    with open(scorefile, "r") as f:
        scores = json.loads(f.read())
except FileNotFoundError:
    scores = [999999] * case

if case > len(scores):
    scores = [999999] * case

def solve(seed):
    return int(subprocess.check_output('java -cp out MapRecoloringVis -exec out/main.out -seed {}'.format(seed), shell=True))

class State:
    count = 0
    rate = 0
    lock = threading.Lock()

    def add(self, seed, score):
        T = 100000
        score_a = int(score / T)
        score_b = score % T
        best__a = int(scores[seed] / T)
        best__b = scores[seed] % T
        if best__a > score_a:
            best__a = score_a
        if best__b > score_b:
            best__b = score_b
        nom = (best__a / score_a) * (best__b / score_b)
        scores[seed] = best__a * T + best__b
        with self.lock:
            self.count = self.count + 1
            self.rate = (self.rate * (self.count - 1) + nom) / self.count
            print('{}\t{}\t\t{}'.format(seed, score, self.rate))

state = State()
q = queue.Queue()

def worker():
    while True:
        seed = q.get()
        if seed is None:
            break
        score = solve(seed)
        state.add(seed, score)
        q.task_done()

num_worker_threads = 4
threads = []
for i in range(num_worker_threads):
    t = threading.Thread(target=worker)
    t.start()
    threads.append(t)

for seed in range(1, case):
    q.put(seed)

q.join()

for i in range(num_worker_threads):
    q.put(None)
for t in threads:
    t.join()

with open(scorefile, "w") as f:
    f.write(json.dumps(scores))