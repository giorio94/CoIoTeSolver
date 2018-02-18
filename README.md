# CoIoTe Solver

CoIoTe Solver is an application designed and developed as an assignment from the
''Optimization Methods and Algorithms'' (a.y. 2016/2017) course taught at the
Politecnico di Torino (Italy). The team was composed by:

+ Marco Iorio
+ Paolo Iannino
+ Aldo Lacuku
+ Kian Kamyab Masouleh
+ Mehrnoosh Rigi Kanhari

The following description gives a brief insight about the topic and the
purpose of this assignment; more information can be found inside the
[Assignment Introduction](AssignmentIntroduction.pdf) PDF file, while the
[Assignment Presentation](AssignmentPresentation.pdf) describes at a high level
our implementation choices and strategies.

*Disclaimer: the authors were not related to and did not work with TIM/Telecom
Italia during the development of the assignment. It has been carried on simply
as a university project required by the course and the company only gave the
context for the work, with no further relationships.*

## CoIoTe

It is a new service being developed by TIM/Telecom Italia which is capable of
connecting Internet of Things (IoT) objects to Internet, without requiring the
deployment of a new ad-hoc wireless network. The service is an alternative to
other mainstream solutions for connecting smart objects to the Internet.

In a Smart City, IoT objects collect and exchange data in order to offer new
services to the community. They can be remotely configured without the need to
send technicians on site. For doing that each object should have access to the
Internet. For example some typically IoT objects that could be part of a Smart
City are:

+ Dumpsters that have to send to the *cloud* their level of waste filling.
+ Smart street lights able to measure the level of filth that is obfuscating
  their light and request intervention for cleaning.
+ Traffic lights which have to be reconfigured based on the traffic conditions.
+ Manholes that may send an alarm when they get hacked (for stealing metal) or
  are clogged by leaves.
+ Bus stops able to show the times of arrival of public modes and detect their
  transit.
+ Bike sharing columns that detect the presence or absence of available bicycles.

**How could IoT Objects connect to the Internet?**

Well, smart objects could use the mobile network to connect to the Internet but
it would be very expensive for the companies to deploy smart objects within the
city. Another problem is that mobile connection is very battery hungry and
batteries should last for months or years without requiring human intervention.
A second possibility could be a new ad-hoc wireless network, but it is also
very expensive to be deployed.
Here comes the solution offered by the CoioTe service: the smart objects around
the city do not need to be all the time connected to the mobile network but just
when they have to send data. For doing that they can use:

+ Domestic Wi-Fi networks that trespass domestic walls and *invade* the street.
+ Hotspot Wi-Fi networks created on-the-fly by the smartphones of TIM users
  passing by Internet of Things objects (e.g. dumpsters) needing to send data to
  Internet.

## CoIoTe Assignment

The goal is to optimize a system capable of assigning missions to mobile
customers: users have to be sent near to, e.g., dumpsters within the city, in
order to give them Internet connectivity for the time needed to send the data
over the network. Such dumpsters, for example, need connectivity so that they
can send their waste filling level to the waste company main server in order to
better schedule the emptying of dumpsters. The mobile users will not participate
without some kind of reward; the idea is to have a trade-off between:

+ The level of connectivity service guaranteed by TIM to the dumpster company.
+ The cost of engaging mobile users so that they change their habitual route and
  let TIM control the hotspot Wi-Fi network of their smartphones for some small
  amount of time.

## Requirements

+ C++11 toolchain
+ Git
+ Input files (not included in this repository)

## How To - Linux\*

+ Get a copy of this repository;
+ Create a symbolic link called `input` in the root directory of the project
  pointing to the location containing the input files describing the instances;
+ Open the shell and navigate to the directory `scripts_linux`;
+ Execute `make` to compile all the necessary files and build the executable;
+ Execute one of the solve scripts provided to solve a part of or all the
  instances provided. At the end a comparison against the optimal solutions
  contained in the `compare` folder will be automatically provided in the
  aforementioned folder.

*\* Tested using Debian Stretch with Linux kernel version 4.8 and the most
up-to-date tools normally available in any Linux system.*

## How To - Windows\*

+ Get a copy of this repository;
+ Be sure that your `PATH` environment variable contains the mingw binaries
  directory;
+ Open the shell and navigate to the directory `scripts_windows`;
+ Execute the `compile` script to compile all the necessary files and build the
  executable file;
+ Execute the `solve_all` script, specifying as parameter the directory
  containing the instances, to solve all of them.

*\* Tested using Windows 10 and an up-to-date version of [mingw-w64](
https://sourceforge.net/projects/mingw-w64) with posix thread support.*

## License

This project is licensed under the [GNU General Public License version 3](
https://www.gnu.org/licenses/gpl-3.0.en.html).
