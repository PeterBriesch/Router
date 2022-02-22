import requests
import time
import random


if __name__ == '__main__':

    target = "21e80"
    source = "bb3620bb5a90b6801671724d0fd03f3cc0bfa0063b41a369aba56edded063dc1"

    while(1):
        data = random.randrange(0, 999999, 1)
        params = {'target': target, 'source': source, 'data': data}
        r = requests.post('http://localhost:2180/api/v2/mine', params=params)
        time.sleep(5)


