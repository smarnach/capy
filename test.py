#!/usr/bin/env python2.7

import samplesim

class Config:
    def f(x):
        return x*x
    name = "sqr"
    verbose = True
    x0 = 0.0
    x1 = 1.0

sim = samplesim.MySimulation(vars(Config))
sim.do_time_step(0.1)
Config.verbose = False
sim.do_time_step(0.05)
