Dynamic Disnix
==============
Dynamic Disnix is a (very experimental!) prototype extension framework for
Disnix supporting *dynamic* (re)deployment of service-oriented systems.

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
  discovered infrastructure model that cannot be auto-discovered
* The *distribution generator* computes a mapping of services to machines based
  on their technical requirements and non-functional requirements
* The *port assigner* automatically assigns unique TCP ports to services that
  require them

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

The first parameter refers to a Disnix infrastructure model that can be written
by hand or generated through `disnix-capture-infra` or the
[Dynamic Disnix Avahi client](http://github.com/svanderburg/dydisnix-avahi).

An augmentation model could look as follows:

```nix
{infrastructure, lib}:

lib.mapAttrs (targetName: target:
  target // (if target ? containers && target.containers ? mysql-database then {
    containers = target.containers // {
      mysql-database = target.containers.mysql-database //
        { mysqlUsername = "root";
          mysqlPassword = "secret";
        };
    };
  } else {})
) infrastructure
```

An augmentation model is a function in which `infrastructure` refers to the
original infrastructure model and `lib` to the set of library functions in
Nixpkgs. In the body, a policy should be written that augments the
infrastructure model with additional data.

In the above example, we search for all machines that provide a MySQL DBMS as a
container service, and we manually configure a password.

Generating a distribution model
-------------------------------
We can generate a distribution model from a services model, infrastructure model
and quality of service (QoS) model in which the last model describes how to map
services to machines based on their technical and non-functional properties:

    $ dydisnix-gendist -s services.nix -i infrastructure.nix -q qos.nix

A QoS model is a Nix expression declaring a function that has the following
header:

```nix
{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:
```

The parameters have the following properties:

* The `services` parameter refers to a Disnix services model
* The `infrastructure` parameter refers to a Disnix infrastructure model
* The `initialDistribution` refers to a cartesian product mapping each service
  in the services model to each machine in the infrastructure model
* The `previousDistribution` refers to the distribution model of the previous
  deployment, or `null` in case of an initial deployment
* The `filters` parameter exposes a set of utility functions to dynamically
  compose mappings
* The `lib` parameter exposes the utility functions from Nixpkgs

A simple QoS model would be the following:

```nix
{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

filters.divide {
  strategy = "greedy";

  inherit services infrastructure;
  distribution = initialDistribution;
  
  serviceProperty = "requireMem";
  targetProperty = "mem";
}
```

The above QoS model uses a greedy division method that takes the `mem` propery
of each machine, representing the total amount of RAM that it provides, and
divides services over them each having their own memory requirements (indicated
by `requireMem`).

We can also combine filter functions:

```nix
{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

filters.divide {
  strategy = "greedy";
  inherit services infrastructure;
  
  distribution = filters.mapAttrOnList {
    inherit services infrastructure;
    distribution = initialDistribution;
    serviceProperty = "type";
    targetPropertyList = "supportedTypes";
  };
  
  serviceProperty = "requireMem";
  targetProperty = "mem";
}
```

In the above example, we first filter each service (that has a specific `type`)
in such a way that that they are only mapped to machines that support them
(through a list property named `supportedTypes` indicating which types of
service the machine can run). Then we use the same division method as in the
previous example to divide the services over the candidates.

The notation used in the above example is a bit inconvenient as for each
transformation step on the candidate distribution, we nest one indentation level
deeper. A better practice would be to write a QoS model as follows:

```nix
{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

let
  # Filter the candidate distribution by types.
  # This prevents services of a type to get distributed to a machine that is
  # incapable of activating it.
  
  distribution1 = filters.mapAttrOnList {
    inherit services infrastructure;
    distribution = initialDistribution;
    serviceProperty = "type";
    targetPropertyList = "supportedTypes";
  };
  
  # Divide the services over the candidates based on their memory constraints
  # using the greedy division method.
  
  distribution2 = filters.divide {
    strategy = "greedy";
    inherit services infrastructure;
    distribution = distribution1;
    serviceProperty = "requireMem";
    targetProperty = "mem";
  };
in
distribution2
```

In the revised example shown above, we compose a let-block in which each
attribute composes a new candidate distribution, using the previous candidate
distribution as an input. Each transformation step can be done on the same
indentation level.

When implementing QoS policies, it is a good practice to divide them in to
two phases -- the *candidate selection* phase determines which services a
specific target can host, the *division phase* divides the services over the
candidates according to some strategy,

The Dynamic Disnix toolset provides a collection of algorithms described in the
academic literature. For more information on filter functions, consult the API
documentation of the `$PREFIX/share/dydisnix/filters.nix` module. Currently, the
following algorithms are provided:

* A collection of service to target machine mapping functions that filters
  candidate mappings on relationships, e.g. one to many, many to one, one to
  one.
* A round robin division method.
* A function that orders candidate target machines on priority
* A one dimensional division method, using a `greedy`, `lowest-bidder` or
  `highest-bidder` strategy
* An approximation alogrithm for the subset sum problem.
* An approximation algorithm for the multiway cut problem.
* An approximation algorithm for the graph coloring problem.

Port assigner
-------------
Some services require unique TCP port assignments. We can automate this process
by augmenting services in the Disnix services model with two properties:

```nix
{distribution, system, pkgs}:

let
  portsConfiguration = if builtins.pathExists ./ports.nix
    then import ./ports.nix else {};
  ...
in
rec {
  roomservice = rec {
    name = "roomservice";
    pkg = customPkgs.roomservicewrapper { inherit port; };
    dependsOn = {
      inherit rooms;
    };
    type = "process";
    portAssign = "private";
    port = portsConfiguration.ports.roomservice or 0;
  };

  ...

  stafftracker = rec {
    name = "stafftracker";
    pkg = customPkgs.stafftrackerwrapper { inherit port; };
    dependsOn = {
      inherit roomservice staffservice zipcodeservice;
    };
    type = "process";
    portAssign = "shared";
    port = portsConfiguration.ports.stafftracker or 0;
    baseURL = "/";
  };
}
```

In the above services model, each services declares a `portAssign` property that
can be `private` to indicate that a unique port should be assigned that applies
to machine where it has been deployed and `shared` to indicate a port that is
unique to the network.

Each service has a `port` property that contains the actual port assignment
value. This value is imported from the `ports.nix` expression and can be
automatically generated.

The following tool can be used to generate port assignments and to reuse
previous port assignments to prevent unnecessary redeployments:

    $ dydisnix-port-assign -s services.nix -i infrastructure.nix -d distribution.nix -p ports.nix > ports2.nix

The first three parameters refer to the Disnix service, infrastructure and
distribution models in which the services model is augmented with `portAssign`
properties. The last parameter `ports.nix` is a port specification expression
that has the following structure:

```nix
{
  ports = {
    roomservice = 8001;
    zipcodeservice = 3003;
  };
  portConfiguration = {
    globalConfig = {
      lastPort = 3003;
      minPort = 3000;
      maxPort = 4000;
      servicesToPorts = {
        stafftracker = 3002;
      };
    };
    targetConfigs = {
      test2 = {
        lastPort = 8001;
        minPort = 8000;
        maxPort = 9000;
        servicesToPorts = {
          roomservice = 8001;
        };
      };
    };
  };
}
```

The above configuration attribute set contains three properties:

* The `ports` attribute contains the actual port numbers that have been assigned
  to each service.
* The `portConfiguration` attribute contains port configuration settings for the
  network and each target machine. The `globalConfig` attribute defines a TCP
  port range with ports that must be unique to the network. Besides the port
  range it also stores the last assigned TCP port number and all global port
  reservations.
* The `targetConfigs` attribute contains port configuration settings and
  reservations for each target machine.

When running the port assigner, a new port assignment expression gets generated
that contains updated mappings for the services.

Dynamically deploying a system
------------------------------
The following command deploys a service-oriented system in which the
infrastructure is dynamically discovered, and the distribution dynamically
generated:

    $ dydisnix-env -s services.nix -a augment.nix -q qos.nix

When adding a ports parameter, it will also automatically assign ports as part
of the deployment process:

    $ dydisnix-env -s services.nix -a augment.nix -q qos.nix --ports ports.nix

Self adaptive deployment of a system
------------------------------------
We can also run a basic feedback loop regularly checking for changes and
redeploying the machine if any change has been detected:

    $ dydisnix-self-adapt -s services.nix -a augment.nix -q qos.nix

When adding a ports parameter, it will also automatically assign ports as part
of the deployment process:

    $ dydisnix-self-adapt -s services.nix -a augment.nix -q qos.nix --ports ports.nix

We can also preemtively take snapshots of all stateful services by providing the
`--snapshot` parameter, so that if any of the machines disappears, a service's
state can be restored when it is redeployed elsewhere:

    $ dydisnix-self-adapt -s services.nix -a augment.nix -q qos.nix --snapshot

Finally, it may also happen that a machine with an existing deployment
configuration gets added to a network. In order to be able to manage them, their
deployment configurations must be reconstructed. This can be done by adding the
`--reconstruct` parameter:

    $ dydisnix-self-adapt -s services.nix -a augment.nix -q qos.nix --reconstruct

License
=======
Disnix is free software; you can redistribute it and/or modify it under the terms
of the [GNU Lesser General Public License](http://www.gnu.org/licenses/lgpl.html)
as published by the [Free Software Foundation](http://www.fsf.org) either version
2.1 of the License, or (at your option) any later version. Disnix is distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.
