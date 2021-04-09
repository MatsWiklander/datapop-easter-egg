#!/usr/bin/env python
# -*- coding: utf-8 -*-

from math import pi
from math import sin


def main():
    gen_sintab(221, 0, 255)
    gen_sintab(197, 50, 208)
    pass


def gen_sintab(length, min, max):
    sintab = []

    for i in range(0, length):
        fac = sin(i * 2 * pi / length) * (max - min) / \
            2 + min + (max - min) / 2
        sintab.append(str(round(fac)))

    print('{' + ', '.join(sintab) + '}')


if __name__ == "__main__":
    main()
