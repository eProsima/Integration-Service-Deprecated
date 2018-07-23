Shapes Demo TCP example
=====================

In this example, we will communicate two shapesdemo over TCP, making use of a TCP bridge.

Executing the *Integration Services* with the provided `config.xml <config.xml>`_ file in this folder will create a bridge between the two shapesdemo and communicate both applications.

One shapesdemo will be configured with domain 1 and UDP. The other instance will use domain 0, and UDP as well.
Create in the shapesdemo instance on domain 1 one or more publishers.
Do the same with the instance on domain 0, but with subscribers instead.

You will notice that there is no communication between both applications.
Run the *Integration Services* with the provided `config.xml <config.xml>`_ file, and both applications will start to communicate.

Another two configuration files are provided, `config_server.xml <config_server.xml>`_ and `config_client.xml <config_client.xml>`_. You can modify them to configure IP addresses and ports correctly. 
These config files will allow communicate two shapesdemo instances with the same configuration, but in different machines, including behind NAT if properly configured.
