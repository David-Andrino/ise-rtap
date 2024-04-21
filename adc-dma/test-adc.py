import sys
from matplotlib import pyplot as plt

resolution = 12
Vref = 3.3
fs = 48000
buffer = []
line = "init"

while line.strip():
    line = sys.stdin.readline()
    if line.startswith('First: ') or line.startswith('Second: '):
        line = line.split(': ')[1]
    buffer += [int(x, 16) for x in line.split()]

buffer = [x * Vref / (2**resolution) for x in buffer]
X = [i/fs for i in range(len(buffer))]

plt.figure()
plt.title('Test de ADC')
plt.plot(X, buffer)
plt.xlabel('Tiempo (s)')
plt.ylabel('Tensión (V)')
plt.tight_layout()
plt.show()

