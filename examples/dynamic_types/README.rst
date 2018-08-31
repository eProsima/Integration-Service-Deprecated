Dynamic Types example
=====================

In this example, we will communicate Helloworld and Keys examples from FastRTPS, making use of a bridge with transformation.

Executing the *Integration Services* with the provided `dyn_dyn_config.xml <dyn_dyn_config.xml>`_ or
`static_static_config.xml <static_static_config.xml>`_ files in this folder will create a bridge between
the two examples and communicate both applications.

Helloworld example must be started as publisher. Keys examples as subscriber.
You can modify the .xml files to change roles.

You will notice that there is no communication between both applications.
Run the *Integration Services* with one of the provided configuration files, and both applications will start to
communicate.
