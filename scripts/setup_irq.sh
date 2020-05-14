#!/bin/bash

cpu_low_irqs=2
cpu_high_irqs=3

low_irqs=`grep "ens3f0" /proc/interrupts | awk '{print substr($1, 1, length($1)-1)}' | head -n 20`
high_irqs=`grep "ens3f0" /proc/interrupts | awk '{print substr($1, 1, length($1)-1)}' | tail -n 20`

for irq in $low_irqs;
do
        echo $cpu_low_irqs > /proc/irq/$irq/smp_affinity_list
done

for irq in $high_irqs;
do
        echo $cpu_high_irqs > /proc/irq/$irq/smp_affinity_list
done