import sys
r = int(sys.argv[1])
import random
for _ in range(2):
    print(r,r)
    for _ in range(r):
        for _ in range(r):
            print(random.randint(1,100))
            # print(1)
