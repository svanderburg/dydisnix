Dynamic Disnix
==============
Dynamic Disnix is a prototype extension framework for Disnix supporting
*dynamic* (re)deployment of service-oriented systems.

The basic Disnix toolset implements models and mechanisms to execute the
deployment of a service oriented system into a network of machines. The
deployment models of the basic toolset are *static* -- one must explicitly
write down the available services and their characteristics, the available
target machines in the network and their properties, and a distribution mapping
services to machines.

In practice, a network of machines may be quite dynamic -- a machine may crash
and disappear from the network, we may add machines to provide extra system
resources, or we may change their characteristics, e.g. a RAM upgrade.

These events may require reconfiguration of the services defined in the services
model, the target machines in the infrastructure model and the mapping of
services to machines in the distribution model, which is quite costly to do by
hand, in particular in large networks of machines.

The extension framework provides a number of components to cope with the dynamism
of the infrastructure:

* A *discovery service* is responsible for discovering the machines in the
  network and their characteristics and generating an infrastructure model
* The *infrastucture augmenter* adds privacy sensitive information to the
  discovered infrastructure model that cannot be discovered
* The *distribution generator* computes a mapping of services to machines based
  on their technical requirements and non-functional requirements

Prerequisites
=============
In order to build Dynamic Disnix from source code, the following packages are
required:

* [Disnix](http://nixos.org/disnix)
* [libxml2](http://xmlsoft.org)
* [libxslt](http://xmlsoft.org)
* [glib](https://developer.gnome.org/glib)

Installation
============
Dynamic Disnix is a typical autotools based package which can be compiled and
installed by running the following commands in a shell session:

    $ ./configure
    $ make
    $ make install

For more information about using the autotools setup or for customizing the
configuration, take a look at the `./INSTALL` file.

Usage
=====
This package contains a number of command-line utilities. The most important
use cases are the following:

Augmenting a discovered infrastructure model
--------------------------------------------
The following command takes an infrastructure model and augmentation model and
produces a new infrastructure model with the properties augmented:

    $ dydisnix-augment-infra -i infrastructure-discovered -a augment.nix

Generating a distribution model
-------------------------------
We can generate a distribution model from a services model, infrastructure model
and quality of service (QoS) model in which the last model describes how to map
services to machines based on their technical and non-functional properties:

    $ dydisnix-gendist -s services.nix -i infrastructure.nix -q qos.nix

Dynamically deploying a system
------------------------------
The following command deploys a service-oriented system in which the
infrastructure is dynamically discovered, and the distribution dynamically
generated:

    $ dydisnix-env -s services.nix -a augment.nix -q qos.nix

Self adaptive deployment of a system
------------------------------------
We can also run a basic feedback loop regularly checking for changes and
redeploying the machine if any change has been detected:

    $ dydisnix-self-adapt -s services.nix -a augment.nix -q qos.nix

Port assigner
-------------
Some services require unique TCP port assignments. The following tool can be
used to generate them and to reuse previous port assignments to prevent
unnecessary redeployments:

    $ dydisnix-port-assign -s services.nix -i infrastructure.nix -d distribution.nix -p ports.nix > ports2.nix

License
=======
Disnix is free software; you can redistribute it and/or modify it under the terms
of the [GNU Lesser General Public License](http://www.gnu.org/licenses/lgpl.html)
as published by the [Free Software Foundation](http://www.fsf.org) either version
2.1 of the License, or (at your option) any later version. Disnix is distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.
