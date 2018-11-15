Dynamic Types example
=====================

In this example, we will communicate HelloWorld and Keys examples from FastRTPS, making use of a bridge with transformation.

Executing the *Integration Service* with the provided `dyn_dyn_config.xml <dyn_dyn_config.xml>`_ or
`static_static_config.xml <static_static_config.xml>`_ files in this folder will create a bridge between
the two examples and communicate both applications.

The HelloWorld example must be started as a publisher. The Keys examples as a subscriber.
You can modify the .xml files to change roles.

You will notice that there is no communication between both applications.
Run the *Integration Service* with one of the provided configuration files, and both applications will start to
communicate.

Note to Windows users:
^^^^^^^^^^^^^^^^^^^^^^

You must use `dyn_dyn_config_win.xml <dyn_dyn_config_win.xml>`_ or
`static_static_config_win.xml <static_static_config_win.xml>`_ configuration files instead.
By default the example will be compiled as Debug, so the files are configured in that way.
If you compile the examples as Release, you should modify the configuration files to match the output build folder.