MeshSim
=======

Overview
--------

The main purpose of the MeshSim Software detailed below is to provide a
baseline ns-3 based simulator that allows simulation of a mesh network
in a fairly faithful manner. It provides a simulation environment to
allow the study of existing multi-hop networks and to analyze their
characteristics. 

The mesh networks being targeted for simulation are multi-hop linear and
mesh networks that have wireless connections from UEs (User Equipment,
also referred to as STAs) to APs (Access Points); the APs are wirelessly
connected among each other, i.e., each AP has a connection to a few of
the other APs.  At least one of the APs has a link to the backhaul,
which is high bandwidth and may be wired or not.  

Our goal is to provide a solid simulator foundation for further work. To
achieve that, the MeshSim software is composed of a set of three
components that can work in concert, but can also be used individually.
Those components are the configuration generator, the ns-3 simulator
proper, and the post-processing tools. The components are written in a
way that they can be reused in other setups. For example, some of the
post-processing tools can be applied to real-world rather than simulated
data.  Another advantage of such a modular approach is that components
can be swapped out easily, without touching any internals.  For example,
the generation of routing information is separate from the simulator,
and therefore the routing information is easy to swap out.

In our modular design, the configuration generator is a toolbox of
template files and scripts that generates a high level description of
the simulation to be run, containing simutation information such as the
number of UEs and APs there are, their positioning in the space, the
connectivity graph, network parameters, link connection quality, the
applications running on the UEs including the timing, etc. All
configuration template files are human readable and easily editable for
ease of configuration.

The simulator itself acts on such a specified, fairly low level
configuration.  It is based on ns-3 and its only dedicated operation is
to run a simulation according to the specification and produce output
files, which are pcap or trace files, along with some augmented output
about the timing information about the simulation run itself.

A separate post-processing step is used to sift through simulator output
and produce useful data.  They take as inputs a set of pcap or trace
ouptut files along with some of the configuration data to compute
metrics of interest. Such metrics include for example data throughput
achieved for downloads, the consistency of the throughput, delay etc.
Post-processing tools also have functionality to collect results from
different runs and tabulate them and create graphs.

Components
----------

Following are the main components that the MeshSim software package is
composed of: 

### ns-3 package with ICSI changes

The MeshSim software is based off the ns-3 open source project. ns-3 is
a discrete-event network simulator for Internet systems, targeted
primarily for research and educational use. As such it serves our
purpose of developing a MeshSim simulator well. 

More information on the ns-3 project can be found on the [NS3
website](https://www.nsnam.org/)

ns-3, however, is a work in progress and presented some bugs that needed
fixing. We are currently working off of ns-3 version 3.30. While we push
discovered bugs back to the ns-3 developers, we have an ICSI development
branch that tracks an ns-3 branch (presently tracking ns-3 version 3.30)
and add our fixes locally to the development branch to prevent any
showstoppers at our end. 

Following is a documentation of the ns-3 bugs that have been discovered
thus far: We’ve seen wifi connection drops even in simple networks.  We
reported that bug was reported together with a testcase to the ns-3
developers.  Several fixes were quickly put forward by the NS-3
developer team, and tested by us. Finally, a hotfix release 3.30.1, was
published on September 16, 2019 to address the issue. References:

* Problem report:
  <https://mailman.isi.edu/pipermail/ns-developers/2019-August/014871.html>
  and <https://gitlab.com/nsnam/ns-3-dev/issues/79>

* Fix and testing discussions:
  <https://gitlab.com/nsnam/ns-3-dev/merge_requests/96>

* 3.30.1 release announcement:
  <https://mailman.isi.edu/pipermail/ns-developers/2019-September/014890.html>

Pcap files generated directly from wireless devices are sometimes
incomplete, or at least they strongly appear so from wireshark.  For
example, wireshark will sometimes find ACKs sent out in TCP
conversations for which it didn’t see the corresponding segment.  For a
real packet capture, this would suggest that the capture is incomplete,
but that shouldn’t be possible in the simulator.  We work around this
limitation by connecting all our wireless endpoints to with a
point-to-point link to another second node; so we can capture traces
from that second node.  We have not yet submitted a bug report for this;
but plan on doing so going forward.  NS-3’s mesh module only works with
802.11g and older.  We adapted some code from the generic wifi code to
the mesh module, which now partially supports 802.11n.  This work is
incomplete; going forward we hope to merge our efforts with those from
UIUC to avoid duplicate work.  We’ve sporadically encountered a failed
assertion: "Merging one lost and another not lost. Impossible" triggered
in tcp-tx-buffer.cc, line 597 (ns-3.30). We believe we have tracked down
the problem, and fixed it in our tree.  We plan on reporting this issue,
together with a suggested fix, to the ns-3 developers, time permitting.
Another issue, that has been reported to the ns-3 developers, is that
TCP flow control [is not
working](https://gitlab.com/nsnam/ns-3-dev/issues/125).  Another
problem, already known by the ns-3 developers is that UDP sockets have
no backpressure; i.e., UDP packets are already dropped on the node
attempting to send them, if they're being sent at a rate exceeding the
capacity of the first link.

### Configuration generator

Configuration generator is a set of tools to generate a simulation
configuration such as physical placement of the APs and UEs according to
deterministic or randomized methods, routing algorithm, as well as
translating other higher level specifications into a detailed simulation
configuration.

The generator scripts use different template input files to specify the
configuration for the simulation. In particular, templates are provided
to configure the following:

* AP/mesh nodes position and mobility model (`mesh_mobility.txt.in`)

* STA/UE positions and mobility model (`sta_mobility.txt.in`)

* Routing algorithm to use e.g. OLSR, AODV, static (`routing.txt.in`)

* A script and template to generate static routes if static routing is
  chosen (`routing_template.txt.php`)

* Wireless standard, rate control algorithm and loss-delay model for
  AP-STA network (`apsta_wifi.txt`)

* Wireless standard, rate control algorithm for mesh network
  (`mesh_wifi.txt`)

* Various applications and their attributes to be simulated on the STAs,
  including the times when the applications start and stop on each STA
  are configurable. (`apps.txt`)

* Command line arguments for the `mesh_sim` executable including the
  random number generator seed, number of mesh nodes, number of
  STAs/UEs, tcp segment size etc. (`cmdline_args.txt.in`)

Some example configuration files can be found in the following directory
of MeshSim: `MeshSim/examples/conf`.

It is typical to run multiple similar simulations that sweep over a few
varying parameters. Such tests are needed to see how the system scales,
to analyze sensitivity, or to find good operating spots. For that
purpose we provide scripts that automatically generate the varying
configurations, run the simulator, and collect the output in a way that
is useful for post-processing. In a similar vein, sometimes it is useful
to repeat identical simulations with different random seeds, so as to
get a handle on the variability of the behavior. We provide scripts to
sweep over various seed values. 

In order to allow repeatability of tests easily, when a configuration
generator is run, it archives all the input configuration templates,
generated configuration files, `Makefile` and the `mesh_sim` executable
in the output directory. This allows someone to easily reproduce the
exact run in a predictable manner. 

### MeshSim

The MeshSim simulator software forms the core simulator software that
implements a mesh network. It is developed in c++. It uses ns-3
libraries to provide the lower level simulation functionality. It also
takes as input the configuration input files generated by the
configuration generator. 

The MeshSim mesh network consists of the backhaul, mesh nodes and
UEs/STAs. The assignment of IP addresses to the various network
interfaces is indicative of the simulated functionality. 

* Network interfaces of backhaul or talking to backhaul: 10.1.1.X

* Mesh IPs: 10.1.2.X for intra-mesh-node communication, 10.1.3.X for
  AP `<->` STA communication.

* STA IPs: 10.1.3.(100+X) is the network interface talking to the mesh
  APs. 10.1.4.(100+X) is the network interface talking to the wired STA
  (wSTA) corresponding to each STA.

* wSTA IPs:  each STA 10.1.4.(100+X) is connected to the wSTA of IP
  address 10.1.4.X

A typical chainsim toppology is like this:

	backhaul
	  |
	mesh[1]------mesh[2]------ ... ------mesh[n]------STA

### Post processing scripts

The output of the simulator is provided in 2 different forms---pcap
files and trace files. Trace files are generated by default, and pcap
files are turned on using command line option `--enablePcap`. 

Post-processing steps are needed to extract useful statistics from that
data.

For trace files, we provide a tool/script called createresultsdb in the
scripts sub folder. This tool can be run from within the out folder of a
simulation and it creates an sql database from all the parameters and
trace files from a particular simulation sweep. This sql database can
then be queried using normal sql commands and tools to get an
understanding of the simulation results.

For pcap output, we provide a pcap evaluation tool written in C++ to
extract relevant metrics from the pcap files. In particular, we allow
the following to be extracted from the pcaps:

* The average TCP throughput
  (file download) a user sees in one specific simulation run. 

* The UDP echo packet delay variation per second as seen by an STA

* The TCP throughput seen by an STA as a function of time

* The UDP throughput seen by an STA as a function of time

Another set of tools/scripts aggregates and organizes statistics from
simulation runs.  The tabulate script sorts the average TCP throughput
seen by a user as a function of a varying parameter (for example, number
of mesh hops).  Awk and gnuplot scripts to plot the UDP packet delay
data against the TCP throughput as a function of time. 

Building and installing
-----------------------

### Software requirements

The MeshSim software requires the following tools/libraries to be
installed before you can build it successfully. Version numbers are
provided to indicate our build environment. 

* GCC version 8.3
* Python version 3
* PHP version 7.1.19
* Boost libraries version 1.67 (libboost-dev)
* Cmake version 3.13
* Git version 2.20.1
* Libpcap version 1.8.1 (libpcap-dev)
* Numpy for Python 3
* sqlite3

Furthermore and SQL browser such as sqlitebrowser may be handy to browse
the database of results

### Building and installing ns-3

Download ns3 from the ICSI git repo and cd into ns-3-dev

   git clone TBD
   cd ns-3-dev

Switch to the icsi-devel branch

   git checkout icsi-devel

Use waf to configure and build the ns-3 software, ensuring that you do
not install it for ease of further development. 

   ./waf --disable-python configure
   ./waf

### Building and installing MeshSim

Download MeshSim and cd into MeshSim directory.  MeshSim uses cmake to
build the software. Create the build directory, cd into it, and invoke
cmake using the ns-3 build tree path (ns-3 build directory) where you
downloaded and built ns-3 

	mkdir build
	cd build
	cmake \
	  -DNS3_USE_BUILD_TREE=ON \
	  -DNS3_BUILD_TREE=<ns3-build-tree path> \
	  ..
	cmake --build .

This should build the `mesh_sim` executable in the sim subdir directory of the build directory. 
 
MeshSim dynamically loads the ns3 libraries during execution. Add the
ns-3 build tree library path to `LD_LIBRARY_PATH` environment variable. 

	export LD_LIBRARY_PATH=<ns3 build tree path>/lib

NOTE : The configuration generator software is located within the
MeshSim scripts directory and as such gets downloaded/installed when you
download the MeshSim git repo. 

### Building and installing `pcap_eval`

Download `pcap_eval` from the ICSI git repo and cd into `pcap_eval` directory. 

	git clone TBD
	cd pcap_eval

`pcap_eval` uses cmake to build the software. Create the build
directory, cd into it, and invoke cmake:

	mkdir build
	cd build
	cmake ..
	cmake --build .

This should build the `pcap_eval` tool in the build directory. 

Workflow
--------

A typical workflow of running simulations using the MeshSim software
consists of first generating an input configuration for the simulation,
running the actual simulation using the `mesh_sim` executable and then
evaluating and studying the results using createresultsdb or the
`pcap_eval` tool or other graphing scripts. 

### Generating the simulation configuration

The stagesim script is used to generate configuration files more easily
for the MeshSim software. The script takes as input a json file with
parameters to sweep over and a set of configuration template files and
generates configuration files that can be used with `mesh_sim`
executable directly. In addition to generating the configuration files,
it also creates a Makefile to run all the simulations easily with a
single command and an output directory folder where the results of the
simulations will be automatically stored. The output directory folder
structure includes a subdirectory for each of the individual run’s
output files and the configuration files generated that were used to run
that particular simulation.

Example configuration templates and json enumerator files can be found
in the `scripts/chainsim/sim_configs` sub folder of MeshSim. There is
also a README in there to describe the various simulation scenarios
configured using the templates and enumerators in each of the sub
directories present there.

### Running the simulation

Upon doing a build in the MeshSim build directory (as described above),
a `mesh_sim` executable is created in the sim subdirectory of the build
folder. This is the simulator and can be used to run simulations with
various different configurations.  In order to play/experiment with the
`mesh_sim` executable certain steps need to be followed. 
 
Change into directory where the `mesh_sim` executable was built:

	cd MeshSim/build/sim/

MeshSim dynamically loads the ns3 libraries during execution. Add the
ns-3 build tree library path to `LD_LIBRARY_PATH` environment variable. 

	export LD_LIBRARY_PATH=<ns3 build tree path>/lib

Look at the help/usage information for `mesh_sim`:

	./mesh_sim -h

In order to run a simulation, `mesh_sim` requires a `conf/` subdirectory
where the simulation configuration files are stored as input and an out
subdirectory where the generated pcap files are output during the
simulation. Some example configuration files are provided within the
MeshSim software under MeshSim/examples directory. Copy one of these
over to use as a starter set of configuration files. 

	cp -r ../../examples/conf .
	mkdir out
	./mesh_sim 

This now runs the `mesh_sim` with the configuration stored in the conf
subdirectory. Trace files from the simulation are put in the out
directory. A pcap trace is generated for each network interface in the
simulation if the enablePcap option is used. The pcap traces are labeled
with the IP-address and a human readable prefix to give an intuition of
the entity/node producing the trace. The IP-addresses follow the same
numbering scheme as detailed in the MeshSim network topology drawn
above. 


Walkthrough:  Linear network simulations
-----------------------------------------

The MeshSim git tree contains linear network work files.  A simple
example is as follows:

	cd scripts/chainsim/sim_configs/multiple
	../../../stagesim -d out -e enum.json 
	cd out
	cp ../../../../../build/sim/mesh_sim .

MeshSim dynamically loads the ns3 libraries during execution. Add the
ns-3 build tree library path to `LD_LIBRARY_PATH` environment variable.

	export LD_LIBRARY_PATH=<ns3 build tree path>/lib

	make -j`nproc`

This runs the various simulation scenarios enumerated in the enum.json
and conf.in and places the results in a "run" sub folder within the out
directory.

Results in trace files can be post processed using `createsqldb` or the
`pcap_eval` tool. To create the sql database do the following from within
the out folder:

	../../../../createresultsdb

This creates an SQL database `results.db` in the out folder.  This
database can then be analyzed using `sqlitebrowser` or some such tool:

	sqlitebrowser results.db

Pcap files can be analyzed using the `pcap_eval` tool. Copy the
`pcap_eval` tool to out folder. To get udp echo delay measurements you
would do something like this:

	./pcap_eval -m "udpdelay" -s "10.1.4.1" -d "10.1.1.1" \
	            -n 12 run/*/wiredsta-10.1.4.1.pcap.gz

Use `./pcap_eval -h` to look at other modes to run `pcap_eval`.

NOTE: Look at the `graphit.sh` script in
`MeshSim/scripts/chainsim/pp_scripts` directory for other usage for
`pcap_eval`. This script is obsolete but provides usage guidance for
`pcap_eval`.

More on the stagesim script
---------------------------

What does this do? First, the stagesim script will create a directory
containing simulations described by the template configurations `conf.in`
with a set of parameters described in `enum.json`.

### JSON enumerators

For simulation, we often want the enumerate the possible values in a
grid, i.e. look at the cartesian product possible values. For example,
we might look at chain simulations with sizes 2, 3, 4, 5 and test
different routing algorithms for each case. Furthermore, we might want
to test each configuration with 5 different seeds. Thus we want all the
possible configurations of size, seed, and routing method in a test,
that is the cartesian product of the individual sets of possible
parameter values.  Sometimes though the set is slightly more
complicated; usually not much so.  For example, it might be the case
that we want to run tests with varying number of wifi PHY transmit
streams and a varying number of antennas, but only combinations where
there are at least as many antennas as streams.  In such cases, the
explicit listing of parameter values together with the `cartesian_ext`
enumerator, as described below, comes in handy.

* `cartesian` enumerator.  Is described in JSON as an object of the form

	{
	    ‘type’:  ‘cartesian’,
	    { 'var_a': [ list of var_a values ],
	      'var_b': [ list of var_b values ],
	      …
	    }
	}

  The `cartesian` enumerator then produces all sets of parameter
  assigments with all the possible combinations; e.g. if `var_a` can
  have values 1, 2, 3, and `var_b` can have values 'a', 'b', there will
  be six pairs.


* `listed` enumerator: Simply returns each item in the explicitly
  provided list, one after another.  So the input is a list of objects
  with parameters, and one of those sets of parameters are the one that
  are generated.  JSON object:

	{ "type": "listed", "list": [ { ... }, ... ] }

* `cartesian_ext` enumerator: Takes two enumerators and returns all the
  possible unions of pairs from first and second. For example one
  enumerator could enumerate all the dicts with values for variables a
  and b, and another one could list all the dicts with values for
  variables c and d.  Then every combination of values for a and b is
  joined with every combination of values for c and d. JSON syntax:

	{ "type": "cartesian_ext", "enum1": { ... }, "enum2": { ... } }

  Here, the objects given as values for the keys enum1 and enum2 are,
  recursively any valid JSON desciption of an enumerator.

The `chainsim/sim_configs` directory contains a number of sub folders
with such JSON enumerators, which can serve as useful examples.

### Template configuration folders

The other argument that stagesim takes, is the path to a template
configuration folder, by default called `conf.in`.  A template
configuation folder, together with a set of parameter values, is used to
generate a configuration for `mesh_sim`.

In the simplest case, for files not ending in .in or .php in the
configuration folder, they are simply copied over when a configuration
is generated from a template.  Files in the template folder ending in
.in cause a corresponding file in the configuration folder to be
generated, with the .in suffix stripped.  That file has the same content
as the template, but with occurrences of `{param_name}` substituted by
`param_value`. Here, `param_name` is the name of a parameter, and
`param_value` is the value it was given to generate the configuration.

It turns out that sometimes more flexibility is needed than can be
provided with simple `.in` files.  Un such cases we use the PHP
language:  Configuration template files ending in `.php` will be
processed by a PHP interpreter, which can interpret parameter names and
values in any way it sees fit.  For example, suppose we were to have a
parameter called numberOfApps, designating the number of traffic
generating applications to put on a node.  To achieve that, we’d want
the template to create an `apps.txt` file with one line per application,
containing a description of a concrete application.  This can be
realized in php with a simple for loop, and cannot be done with basic
parameter substitution.

### Running simulations

Attentive readers will have noticed that in the above stagesim example,
the `mesh_sim` binary was copied into the mysim folder explicitly.  This
is intentional:  When testing, and implementing changes on `mesh_sim`,
it is crucial to be very attentive to which copy of `mesh_sim` is
exactly being used.  The need to explicitly copy the binary reduces the
chance of errors.  It also reduces the chance of another error:  Suppose
we’re hacking on `mesh_sim` while running simulations in the background.
In such a case, we want to avoid replacing the `mesh_sim` binary of the
simulation while it’s running.  The copied version would not be affected
by us running `make` to build a new `mesh_sim`, which is what we want.
Third, the mysim folder can be used as an archive of simulations; it is
useful to have access to the exact binary that created the results for
future debugging.
