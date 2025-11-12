#!/bin/bash

export test="or.tst"
export chips="test/And16.hdl test/Or.hdl test/And.hdl test/Not.hdl"

cat test/$test | ./hdl $chips
