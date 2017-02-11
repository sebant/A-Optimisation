#!/bin/bash
    # 3472620 nodes cat 
    # 201235 ways cat
    # 23895681 nodes spa
    # 1417363 ways spa
if [ $# -ne 1 ]; then
echo we\'re done, you didn\'t give any input file moron! :-/
else
    if [ $1 == catalunya.csv ]; then 
        nodes=3472620
        ways=201235
    elif [ $1 == spain.csv ]; then
        nodes=23895681
        ways=1417363
    else
        nodes=`grep "^node" $1|wc -l`
        ways=`grep "^way" $1|wc -l`
    fi

    echo  $nodes nodes
    echo $ways ways
    time ./readmap $1 $nodes $ways
fi
