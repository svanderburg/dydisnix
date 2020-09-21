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
* The *ID assigner* automatically assigns unique IDs (such as TCP ports) to
  services that require them

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

```bash
$ ./configure
$ make
$ make install
```

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

```bash
$ dydisnix-augment-infra -i infrastructure-discovered -a augment.nix
```

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

```bash
$ dydisnix-gendist -s services.nix -i infrastructure.nix -q qos.nix
```

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
* An approximation alogrithm for the minimum set cover problem (optimize costs
  when machines have a fixed price).
* An approximation algorithm for the multiway cut problem (try to reduce the
  amount of network links between machines, to make deployments more reliable)
* An approximation algorithm for the graph coloring problem (try to manifest
  all dependencies as network links between machines, with the minimum amount
  of machines).

Implementing custom filter functions
------------------------------------
In addition to using the deployment planning algorithms provided by the Dynamic
Disnix framework, it is also possible to make a catalog of custom filter
functions, such as:

```nix
{pkgs, referenceFilters}:

{
  clear = {distribution}:
    pkgs.lib.mapAttrs (serviceName: target: []);
}
```

A custom filter function catalog has the following structure:
* It defines a function that takes two arguments: `pkgs` refers to the Nixpkgs
  collection and `referenceFilters` refers to the set of reference filters
  included with Dynamic Disnix. The body defines an attribute set of custom
  filter functions.
* The `clear` function is a (dummy) example function that takes an existing
  distribution model and erases the targets for every service.

The `clear` filter function is implemented in the Nix expression language, that
is a domain-specific language less suitable for arbitrary programming tasks.
For more complicated tasks, it is also possible to invoke external executables
that generate Disnix distributions:

```nix
{pkgs, referenceFilters}:

{
  multiwaycut = {distribution}:
    import "${(pkgs.stdenv.mkDerivation {
      name = "distribution.nix";
      buildInputs = [ dydisnix ];
      buildCommand =
      ''
        dydisnix-multiwaycut \
          --distribution ${referenceFilters.generateDistributionXML distribution} \
          --xml \
          > $out
      '';
    })}";
}
```

The above example invokes the `dydisnix-multiwaycut` executable to calculate a
distribution by using an approximation algorithm for the multiway cut problem.
It uses the `referenceFilters.generateDistributionXML` function to convert the
distribution model to a standardized XML file so that it can be consumed by the
`dydisnix-multiwaycut` executable.

Invoking a custom filter function from a QoS model can be done by referring to
the `filters` parameter:

```nix
{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

filters.clear {
  distribution = initialDistribution;
}
```

The filters catalog expression can be specified by providing the `-F`
parameter to the appropriate command-line tools, such as `dydisnix-gendist` that
dynamically generates a distribution model:

```bash
$ dydisnix-gendist -s services.nix -i infrastructure.nix -q qos.nix -F customfilters.nix
```

Using the external distribution filters on the command-line
-----------------------------------------------------------
Apart from QoS models, it is also possible to use some filter algorithms
(that are implemented as external executables) directly from the command-line
for experimentation purposes.

External filter programs accept both Nix expression as well as XML
representations of input models, and can write Nix or XML representations of
filtered distributions to the standard output.

For example, the following command will apply the one-dimensional division
strategy on a set of input models:

```bash
$ dydisnix-divide --strategy greedy -s services.nix -i infrastructure.nix -d distribution.nix --service-property requireMem --target-property mem
```

The above command-line invocation reads services from the given service model
(`services.nix`), uses the target machines in the infrastructure model
(`infrastructure.nix`) considering all targets for each service in the
distribution model (`distribution.nix`) candidate targets.

It does a one-to-many mapping from the `requireMem` property of each service
onto to the `mem` property of each target.

It is also possible to use XML representations of all input models. Internally,
all external filter programs will work with XML representations of the input
models, converting them from Nix when necessary.

The Nix representations of the inputs can also be explicitly converted to XML by
running the `dydisnix-xml` command, such as:

```bash
$ dydisnix-xml -s services.nix
```

The above command-line instruction generates an XML representation of a service
model implemented in the Nix expression language.

Most tools can be instructed to accept XML versions of input models by passing
the `--xml` parameter. For example, the following command does exactly the same
as the previously shown `dydisnix-divide` invocation:

```bash
$ dydisnix-divide --xml --strategy greedy -s services.xml -i infrastructure.xml -d distribution.xml --service-property requireMem --target-property mem
```

By default, the filter programs will output a Nix expression representation of
a new distribution. It is also possible to use XML by providing the
`--output-xml` parameter:

```bash
$ dydisnix-divide --output-xml --xml --strategy greedy -s services.xml -i infrastructure.xml -d distribution.xml --service-property requireMem --target-property mem
```

ID assigner
-----------
Some service require unique IDs, for example to allow them to bind to an
unallocated TCP port or to use stable UIDs and GIDs when a service needs to run
as an unprivileged user.

The process of assigning unique IDs to services can be automated. To do this
you need to specify an ID resources model that has the following structure:

```nix
{
  ports = {
    min = 3000;
    max = 4000;
    scope = "global";
    step = 1;
  };

  uids = {
    min = 2000;
    max = 3000;
    scope = "global";
  };

  gids = {
    min = 2000;
    max = 3000;
    scope = "global";
  };
}
```

The above `idresources.nix` configuration file defines three kinds of numeric
ID resources:

* `ports` refers to a pool of unique TCP port numbers
* `uids` refers to a pool of unique user IDs (UIDs)
* `gids` refers to a pool of unique group IDs (GIDs)

Each resource defines the following configuration properties:

* `min` specifies the minimum allowed ID
* `max` specifies the maximum allowed ID
* `scope` defines the scope of the ID. `global` (the default value) means that
  the IDs should be globally unique for the entire system. `machine` says that
  an ID should be unique to the machine where a service is deployed to.
  In case the scope is `machine`, a service becomes target-specific and can
  only be deployed to one machine in the network only.
* `step` specifies the step size of the search algorithm. The default the value
  is 1, but can be increased to any number. A higher number is useful to auto
  assign port numbers to services that requires multiple port assignments in
  which port numbers are derived from a base port number.

To make it possible to automatically assign IDs to services, we can annotate
a service model as follows:

```nix
{distribution, system, pkgs}:

let
  ids = if builtins.pathExists ./ids.nix then (import ./ids.nix).ids else {};
in
rec {
  roomservice = rec {
    name = "roomservice";
    pkg = customPkgs.roomservicewrapper {
      port = ids.ports.roomservice;
      uid = ids.uids.roomservice;
      gid = ids.gids.roomservice;
    };
    dependsOn = {
      inherit rooms;
    };
    type = "process";
    requiresUniqueIdsFor = [ "ports" "uids" "groups" ];
  };

  ...

  stafftracker = rec {
    name = "stafftracker";
    pkg = customPkgs.stafftrackerwrapper {
      port = ids.ports.stafftracker;
      uid = ids.uids.stafftracker;
      gid = ids.gids.stafftracker;
    };
    dependsOn = {
      inherit roomservice staffservice zipcodeservice;
    };
    type = "process";
    baseURL = "/";
    requiresUniqueIdsFor = [ "ports" "uids" "groups" ];
  };
}
```

In the above partial service model, every service defines a
`requireUniqueIdsFor` property to tell the ID generator for which resources it
requires a unique ID. (The `requireUniqueIdsFor` property can be changed by
passing the `--service-property` parameter to the generator tool).

The generated IDs expression (`ids.nix`) is imported in the beginning of the
expression. To allow the services to use the unique IDs in the `ids` attribute
set, they are passed as parameters to each `pkg` constructor function.

With the following command, you can automatically generate an `ids.nix`
expression assigning unique IDs to every services that is deployed in the
distribution model that requires a unique ID:

```bash
$ dydisnix-id-assign -s services.nix -i infrastructure.nix -d distribution.nix --id-resources idresources.nix --output-file ids.nix
```

The output of the `ids.nix` expression may look as follows:

```nix
{
  ids = {
    ports = {
      roomservice = 3000;
      stafftracker = 3001;
    };

    uids = {
      roomservice = 2000;
      stafftracker = 2001;
    };

    gids = {
      roomservice = 2000;
      stafftracker = 2001;
    };
  };
  lastAssignments = {
    ports = 3001;
    uids = 2001;
    gids = 2001;
  };
}
```

The above Nix expression specifies for each resource type, a mapping from a
service to a unique ID (that might be globally unique or unique to the target
machine where the service is deployed to).

It also memorizes the last assigned IDs per resource (and optionally per target)
so that it will not reuse any previously assigned IDs, unless the limit has been
reached.

In addition to creating ID assignments from scratch, it is also possible to
update an existing `ids.nix` expression:

```bash
$ dydisnix-id-assign -s services.nix -i infrastructure.nix -d distribution.nix --id-resources idresources.nix --ids ids.nix --output-file ids.nix
```

The above command retains all previous ID assignments that are still valid, and
will only assign IDs to service that have none assigned yet. Retaining valid
assignments prevents unnecessary redeployments.

The above commands will only assign IDs to services that are deployed to target
machines. It is also possible to assign IDs to all services in a services model
regardless whether they are actually used or not:

```bash
$ dydisnix-id-assign -s services.nix --id-resources idresources.nix --output-file ids.nix
```

When assigning IDs to all services, you can only use resource types that work
on a global scope.

Dynamically deploying a system
------------------------------
The following command deploys a service-oriented system in which the
infrastructure is dynamically discovered, and the distribution dynamically
generated:

```bash
$ dydisnix-env -s services.nix -a augment.nix -q qos.nix
```

When adding the `--ids` parameter, it will also automatically assign unique IDs
as part of the deployment process:

```bash
$ dydisnix-env -s services.nix -a augment.nix -q qos.nix --id-resources idresources.nix --ids ids.nix
```

Self adaptive deployment of a system
------------------------------------
We can also run a basic feedback loop regularly checking for changes and
redeploying the machine if any change has been detected:

```bash
$ dydisnix-self-adapt -s services.nix -a augment.nix -q qos.nix
```

When adding the `--ids` parameter, it will also automatically assign unique IDs
as part of the deployment process:

```bash
$ dydisnix-self-adapt -s services.nix -a augment.nix -q qos.nix --id-resources idresources.nix --ids ids.nix
```

We can also preemtively take snapshots of all stateful services by providing the
`--snapshot` parameter, so that if any of the machines disappears, a service's
state can be restored when it is redeployed elsewhere:

```bash
$ dydisnix-self-adapt -s services.nix -a augment.nix -q qos.nix --snapshot
```

Finally, it may also happen that a machine with an existing deployment
configuration gets added to a network. In order to be able to manage them, their
deployment configurations must be reconstructed. This can be done by adding the
`--reconstruct` parameter:

```bash
$ dydisnix-self-adapt -s services.nix -a augment.nix -q qos.nix --reconstruct
```

Generating functional architecture documentation
------------------------------------------------
Another use case of the toolset is to use the model parsing infrastructure to
generate functional architecture documentation.

Running the following command will generate a HTML documentation catalog for the
provided services expression, using SVG images for the diagrams, storing the
output artefacts in the `out/` sub folder:

```bash
$ dydisnix-generate-services-docs -s services.nix -f svg --output-dir out
```

The catalog can be inspected by opening the root page in the output folder:

```bash
$ firefox out/index.html
```

For complex architectures consisting of many services, it is also possible to
group services (for example, by clustering services that implement a feature
group). Grouping can be done by annotating a service in a service Nix expression
(e.g. `services.nix`) with a `group` property:

```nix
{distribution, invDistribution, pkgs, system}:

let
  customPkgs = import ../top-level/all-packages.nix {
    inherit pkgs system;
  };

  groups = {
    homework = "Homework";
    literature = "Literature";
  };
in
{
  homeworkdb = {
    name = "homeworkdb";
    pkg = customPkgs.homeworkdb;
    type = "mysql-database";
    group = groups.homework;
    description = "Database backend of the Homework subsystem";
  };

  homework = {
    name = "homework";
    pkg = customPkgs.homework;
    dependsOn = {
      inherit usersdb homeworkdb;
    };
    type = "apache-webapplication";
    appName = "Homework";
    group = groups.homework;
    description = "Front-end of the Homework subsystem";
  };

  ...
}
```

In the above services model, the `homeworkdb` and `homework` service are grouped
together in a feature group called `Homework`. In the above example, this is a
means to group a database and a web application front-end into one "logical"
group" that makes more sense from a functional point of view.

When services are grouped, the documentation catalog generator will partition
the diagrams and their descriptions over multiple pages using a layered
organisation allowing you to zoom in from the highest abstraction layer to
deeper layers. There are multiple layers of sub groups possible, by using the
`/' symbol as a delimiter in a group identifier.

It may also be desired adjust certain kinds of properties for the generated
documentation catalog. These adjustments can be specified in a docs configuration
expression (e.g. `docs.nix`):

```nix
{
  groups = {
    Homework = "Homework subsystem";
    Literature = "Literature subsystem";
  };

  fields = [ "description" "type" ];

  descriptions = {
    type = "Type";
    description = "Description";
  };
}
```

The above configuration provides descriptions to the group identifiers and
specifies that the `type` and `description` attributes of every service should
be displayed in the overview.

By adding the `docs.nix` configuration as a command-line parameter, we can
adjust the documentation generation process with additional descriptions and
fields:

```bash
$ dydisnix-generate-services-docs -s services.nix --docs docs.nix -f svg --output-dir out
```

In addition to generating a full catalog, it is also possible to generate the
diagrams (using `dydisnix-visualize-services`) and descriptions separately
(using `dydisnix-document-services`). These tools can be used for showing the
details of a single layer, or in batch mode to generate artefacts for all
abstraction layers.

License
=======
Disnix is free software; you can redistribute it and/or modify it under the terms
of the [GNU Lesser General Public License](http://www.gnu.org/licenses/lgpl.html)
as published by the [Free Software Foundation](http://www.fsf.org) either version
2.1 of the License, or (at your option) any later version. Disnix is distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.
