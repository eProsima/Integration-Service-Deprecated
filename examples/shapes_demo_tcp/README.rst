Shapes Demo TCP example
=====================

In this example, we will communicate two shapesdemo over TCP, making use of a TCP bridge.

Executing the *Integration Services* with the provided `config.xml <config.xml>`_ file in this folder will create a bridge between the two shapesdemo and communicate both applications.

One shapesdemo will be configured with domain 1 and UDP. The other instance will use domain 0, and UDP as well.
Create in the shapesdemo instance on domain 1 one or more publishers.
Do the same with the instance on domain 0, but with subscribers instead.

You will notice that there is no communication between both applications.
Run the *Integration Services* with the provided `config.xml <config.xml>`_ file, and both applications will start to communicate.

You can modify the config.xml to allow communication between different subnetworks or behind a NAT.
