# GBN Simulator

This repository implements and runs a simulation of the Go-Back-N protocol over UPD in ns3.
It prints statistics about retransmissions and goodput.

## Usage
Place the contents of this repository in the `scratch/` directory in a ns3 project.

##### Build
run `./ns3 build` 

##### Run
run `./ns3 run scratch/gbn-simulator.cc`

##### Configuration
In addition to the two configuration files, the simulator supports two command line arguments both of which support floats in \[0,1\].
* DataPacketLossProbability
* AckPacketLossProbability

The configuration files, `config.h` and `gbn/config.h` control simulation and GBN protocol parameters respectively.
