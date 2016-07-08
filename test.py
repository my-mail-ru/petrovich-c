#!/usr/bin/env python3

import unittest
import subprocess
import os.path

BINARY_PATH = os.path.join(os.path.dirname(__file__),
                           'build', 'debug', 'petr_test')

MALE = 'male'
FEMALE = 'female'

FIRST = 'first'
MIDDLE = 'middle'
LAST = 'last'

def run_test(kind, gender, name):
    with subprocess.Popen([BINARY_PATH, kind, gender, name],
                          stdout=subprocess.PIPE, close_fds=True) as process:
        return process.stdout.read().decode('utf-8').strip().split('\n')

class TestPetrovich(unittest.TestCase):
    def test_male_first(self):
        res = run_test(FIRST, MALE, 'Николай')
        self.assertEqual(res[0], 'Николай')
        self.assertEqual(res[1], 'Николая')
        self.assertEqual(res[2], 'Николаю')
        self.assertEqual(res[3], 'Николая')
        self.assertEqual(res[4], 'Николаем')
        self.assertEqual(res[5], 'Николае')

    def test_male_middle(self):
        res = run_test(MIDDLE, MALE, 'Петрович')
        self.assertEqual(res[0], 'Петрович')
        self.assertEqual(res[1], 'Петровича')
        self.assertEqual(res[2], 'Петровичу')
        self.assertEqual(res[3], 'Петровича')
        self.assertEqual(res[4], 'Петровичем')
        self.assertEqual(res[5], 'Петровиче')

    def test_male_last(self):
        res = run_test(LAST, MALE, 'Воронин')
        self.assertEqual(res[0], 'Воронин')
        self.assertEqual(res[1], 'Воронина')
        self.assertEqual(res[2], 'Воронину')
        self.assertEqual(res[3], 'Воронина')
        self.assertEqual(res[4], 'Ворониным')
        self.assertEqual(res[5], 'Воронине')

    def test_female_first(self):
        res = run_test(FIRST, MALE, 'Татьяна')
        self.assertEqual(res[0], 'Татьяна')
        self.assertEqual(res[1], 'Татьяны')
        self.assertEqual(res[2], 'Татьяне')
        self.assertEqual(res[3], 'Татьяну')
        self.assertEqual(res[4], 'Татьяной')
        self.assertEqual(res[5], 'Татьяне')

    def test_female_middle(self):
        res = run_test(MIDDLE, FEMALE, 'Алексеевна')
        self.assertEqual(res[0], 'Алексеевна')
        self.assertEqual(res[1], 'Алексеевны')
        self.assertEqual(res[2], 'Алексеевне')
        self.assertEqual(res[3], 'Алексеевну')
        self.assertEqual(res[4], 'Алексеевной')
        self.assertEqual(res[5], 'Алексеевне')

    def test_female_last(self):
        res = run_test(LAST, FEMALE, 'Воронина')
        self.assertEqual(res[0], 'Воронина')
        self.assertEqual(res[1], 'Ворониной')
        self.assertEqual(res[2], 'Ворониной')
        self.assertEqual(res[3], 'Воронину')
        self.assertEqual(res[4], 'Ворониной')
        self.assertEqual(res[5], 'Ворониной')

    def test_caps(self):
        res = run_test(FIRST, FEMALE, 'ОЛЬГА')
        self.assertEqual(res[0], 'ОЛЬГА')
        # FIXME: mixed case is ugly
        self.assertEqual(res[1], 'ОЛЬГи')
        self.assertEqual(res[2], 'ОЛЬГе')
        self.assertEqual(res[3], 'ОЛЬГу')
        self.assertEqual(res[4], 'ОЛЬГой')
        self.assertEqual(res[5], 'ОЛЬГе')

    def test_latin(self):
        res = run_test(FIRST, FEMALE, 'latin')
        self.assertEqual(res[0], 'latin')
        self.assertEqual(res[1], 'latin')
        self.assertEqual(res[2], 'latin')
        self.assertEqual(res[3], 'latin')
        self.assertEqual(res[4], 'latin')
        self.assertEqual(res[5], 'latin')

if __name__ == '__main__':
    unittest.main()
