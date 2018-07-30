Making a hello world example
============================

For this example we are going to use two elements:

- *Integration-Service* with the example configuration.
- *Fast RTPS* `HelloWorld example <https://github.com/eProsima/Fast-RTPS/tree/master/examples/C%2B%2B/HelloWorldExample>`_.

The goal will be to make a bridge from the *Fast RTPS* HelloWorld publisher that will write all received data to a file.

Types generation
----------------

*Fast RTPS* HelloWorld's *idl* of the example is located on the main directory.

Now using the *idl* file and the tool `fastrtpsgen <http://eprosima-fast-rtps.readthedocs.io/en/latest/geninfo.html>`_ we are going to generate the needed source files that contains the definition of the described type in the IDL:

.. code-block:: shell

    $ fastrtpsgen idl/HelloWorld.idl

From the `resource <../../resource>`_ directory, we are going to use the provided template for the *CMakeLists.txt* and *config.xml* files. In this point, our directory looks like it is showed on the following graph.

.. code-block:: shell

   examples
   └── helloworld
	   ├── CMakeLists.txt
       ├── config.xml
	   ├── templatebridgelib.cpp
	   ├── HelloWorld.cxx
	   ├── HelloWorld.h
	   ├── HelloWorldPubSubTypes.cxx
	   ├── HelloWorldPubSubTypes.h
       └── idl
	       ├── HelloWorld.idl


Bridge generation
-----------------

For our Bridge we only need to implement a custom Publisher.
We'll use the builtin RTPS Subscriber and a Generic Bridge.

We rename our *templatebridgelib.cpp* to *isfile.cpp*.
Now we can change *create_subscriber* to just return nullptr, as never will be called by Integration-Service.
*create_bridge* will return *new ISBridge(name)* instead.
Finally, *create_publisher* will return our "not yet implemented" FilePublisher: *new FilePublisher(name, config)*.

Let's create the class FilePublisher now. We need to create both .h and .cpp FilePublisher files.
Take as example the already existing files.
Our FilePublisher will read the Publisher's configuration (filename, format and append) and will open a *std::ofstream* applying the parsed parameters on each constructor.
Then will override the *publish* method to just deserialize the SerializedPayload and write its contens to the opened file.
On the destructor the file is closed.


CMakeLists modification
-----------------------

We need to adapt our CMakeLists file to compile our created files. You can see the result in the CMakeLists.txt file present in the root folder of the example.

Now, we have to compile the library using these commands on Linux:

.. code-block:: shell

    $ mkdir build && cd build
    $ cmake ..
    $ make


Or these other on Windows:

.. code-block:: shell

    > mkdir build && cd build
    > cmake .. -G "Visual Studio 14 2015 Win64"
    > cmake --build . --config Release


Publisher parameters
-------------------------------

The next step is to set the *config.xml* file with the specific parameters of our bridge:

.. code-block:: xml

    <is>
        <topic_types>
            <type name="HelloWorld">
                <participants>
                    <participant name="rtps"/>
                </participants>
            </type>
        </topic_types>

        <profiles>
            <participant profile_name="rtps">
                <rtps>
                    <builtin>
                        <domainId>0</domainId>
                    </builtin>
                </rtps>
            </participant>

            <subscriber profile_name="fastrtps_subscriber">
                <topic>
                    <name>HelloWorldTopic</name>
                    <dataType>HelloWorld</dataType>
                </topic>
                <historyMemoryPolicy>DYNAMIC</historyMemoryPolicy>
            </subscriber>
        </profiles>

        <bridge name="file">
            <library>build/libisfile.so</library> <!-- Path to the NGSIv2 library -->

            <publisher name="file_publisher">
                <property>
                    <name>filename</name>
                    <value>output</value>
                </property>
                <property>
                    <name>format</name>
                    <value>txt</value>
                </property>
                <property>
                    <name>append</name>
                    <value>true</value>
                </property>
            </publisher>
        </bridge>

        <connector name="dump_to_file">
            <subscriber participant_name="rtps" subscriber_name="fastrtps_subscriber"/>
            <publisher participant_name="file" publisher_name="file_publisher"/>
        </connector>
    </is>


Now, we only have to launch the HelloWorld example from FastRTPS. If this instructions has been succesfully followed we will see this content on the *output* file.

.. code-block:: shell

    HelloWorld : 1
    HelloWorld : 2
    HelloWorld : 3
    HelloWorld : 4
    HelloWorld : 5
    HelloWorld : 6
    HelloWorld : 7
    HelloWorld : 8
    HelloWorld : 9
    HelloWorld : 10

If you execute again the example, the same content will appear again for each execution. If you modify the *append* parameter to false, the file will be reset for each execution and the content will only appear once.