# eProsima Integration Services
![http://www.eprosima.com](https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcSd0PDlVz1U_7MgdTe0FRIWD0Jc9_YH-gGi0ZpLkr-qgCI6ZEoJZ5GBqQ)

*eProsima Integration Services* is a library and an utility based on *Fast RTPS* for making communication bridges between different systems, services and protocols. With *Integration Services* the user can create parametric communication bridges between applications. At the same time, it is able to perform some transformations over the messages such as customized routing, mapping between input and output attributes or data modification.

Some of the possibilities offered by *Integration Services* are:

-   Connections for jumping from topics which are running on different domains.
-   Adapters for mapping the attributes from types with different IDL definitions.
-   User-defined operations over the circulating messages.
-   Communication with others environments, as *ROS2*.

<p align="center"> <img src="docs/IS-main.png" alt="Default behaviour"/> </p>

<hr></hr>
#### Table Of Contents

[Installation](#installation)

[Configuration options in configuration file](#configuration-options-in-configuration-file)

[Steps to allow other protocols than RTPS](#steps-to-allow-other-protocols-than-rtps)

[Documentation](#documentation)

[Getting Help](#getting-help)
<hr></hr>

#### Installation

Before compiling *eProsima Integration Services* you need to have installed *Fast RTPS* as described in its [documentation](http://eprosima-fast-rtps.readthedocs.io/en/latest/binaries.html). For cloning this project execute:


    $ git clone --recursive https://github.com/eProsima/integration-services

Now, for compiling, if you are on Linux execute:

    $ mkdir build && cd build
    $ cmake ..
    $ make

If you are on Windows choose your version of Visual Studio:

    > mkdir build && cd build
    > cmake ..  -G "Visual Studio 14 2015 Win64"
    > cmake --build .

<hr></hr>

#### Configuration options in configuration file

The configuration files define *topic_types*, *profiles*, *bridges* and *connectors*.
The *topic_types* are the Topic Data Types that will be used by the participants. If these data types use Keys or you want to define how to instante them, *topic_types* allows to define *data types libraries*.
The *profiles* define participants, subscribers, publishers, etc, following the format used by **FastRTPS XML configuration files**.
The *bridges* are defined by the *bridge library* that implements the bridge and the definitions of the subscribers and publishers that we want to instantiate.
Finally, the *connectors* are just relationhips between subscribers and publishers, and optionally, transformation functions.

As reference, let's take this configuration file:

<p align="center"> <a href="docs/example_config.xml"> <img src="docs/Config.png" alt="Config file"/> </a> <font size="2">Click on the image to open the code.</font> </p>


In this file there are defined two RTPS *participants*, and a *bridge*. All of them have a subscriber and a publisher.
The relationships between *participants* and *subscribers*/*publishers* defined in the *profiles* section are stablished by each *connector*. This allows to share *subscribers*/*publishers* configurations between *participants*.
There are four connectors defined: *shapes_projection*, *shapes_stereo*, *shapes_protocol* and *protocol_shapes*.

There are several possible types of connectors depending of the kind of its participants.

- RTPS Bridge:

In this kind of connector, both participant are RTPS compliant, like *shapes_projection* and *shapes_stereo* in our example file.

<p align="center"> <img src="docs/RTPS-bridge.png" alt="RTPS Bridge"/> </p>

* RTPS -> Other protocol

This connector will communicate a RTPS environment with another protocol. Just like our *shapes_protocol* connector.

Your *Bridge Library* must define at least a publisher to your desired protocol and it is responsible to communicate with it and follow the ISPublisher interface. By default, the transformation function is applied after *on_received_data* method calls to the instance of ISBridge. If you want to change this behaviour you will need to override the complete data flow.

*Bridge_configuration* node can contain configuration information that *Bridge Library* must understand. ISManager will parse the *property* nodes of each element and will call the respective *create_* function of the library with a vector of pairs with the data contained.
If no *bridge_configuration* is provided, then your createBridge will be called with nullptr or an empty vector as parameter config.

*Transformation* library could be reused by your bridge library, with the same or another transformation function inside the same transformation library (an example of reusing the transformation library can be found on [FIROS2](https://github.com/eProsima/FIROS2/tree/master/examples/TIS_NGSIv2).
Of course, you can add built in transformation functions inside your *bridge library*.

<p align="center"> <img src="docs/IS-RTPS-to-Other.png" alt="RTPS to Other Bridge"/> </p>

* Other procotol -> RTPS

This is a similar case as the previous one, but in the other way, as in the connector *protocol_shapes* of our example.

The same logic applies in this connectors as in the RTPS -> Other protocol case, but in this case the RTPS participant is the publisher. An example of this can be found on [FIROS2](https://github.com/eProsima/FIROS2/tree/master/examples/helloworld_ros2).

<p align="center"> <img src="docs/IS-Other-to-RTPS.png" alt="Other to RTPS Bridge"/> </p>

* Bidirectional bridge (RTPS <--> Other protocol)

This case is not a connector, but the consecuence of set two connectors with the correct parameters. In our example the combination of *shapes_projection* and *shapes_stereo* is a bidirectional bridge, as well as, *shapes_protocol* and *protocol_shapes*.

A combination of both logics RTPS->Other and Other->RTPS applies here. The example [TIS_NGSIv2](https://github.com/eProsima/FIROS2/tree/master/examples/TIS_NGSIv2) of FIROS2 uses a bridge of this type.


<p align="center"> <img src="docs/IS-RTPS-Other.png" alt="RTPS-Other Bridge"/> </p>

<hr></hr>
### Steps to allow other protocols than RTPS

*Integration Services* has a builtin RTPS bridge, but you can specify any other procotol implementing your own libraries.

There are three kind of libraries that the user can implement, **Bridge Library**, **Transformation Library** and **Data Types Library**.


**Bridge Library**:

This library must export the following functions as defined in the *resource/templatebridgelib.cpp* file:

    ISBridge* create_bridge(const char* name,
    	std::vector<std::pair<std::string, std::string>> *configuration);

    ISSubscriber* create_subscriber(ISBridge* bridge, const char* name,
    	std::vector<std::pair<std::string, std::string>> *configuration);

    ISPublisher* create_publisher(ISBridge* bridge, const char* name,
    	std::vector<std::pair<std::string, std::string>> *configuration);

- The function *create_bridge* must return a pointer to an instance of a derived class of ISBridge, or nullptr if it has failed.

- The function *create_subscriber* must return a pointer to an instance of a derived class of ISSubscriber, or nullptr if it has failed.

- The function *create_publisher* must return a pointer to an instance of a derived class of ISPublisher, or nullptr if it has failed.

Integration Services will deallocate these objects from memory when the bridge is stopped.

Let's take a look to these interfaces:

	/** Base class for publishers. Must know how to write into the destination protocol */
	class ISPublisher
	{
	public:
	    ISPublisher(const std::string &name);
	    virtual ~ISPublisher() = default;
	    virtual bool publish(SerializedPayload_t *data);
	    virtual ISBridge* setBridge(ISBridge *bridge);
	};

	/** Base class for subscribers. Must know how to read from the origin protocol */
	class ISSubscriber
	{
	public:
	    ISSubscriber(const std::string &name);
	    virtual ~ISSubscriber() = default;
	    virtual void addBridge(ISBridge* bridge);
	    virtual void on_received_data(SerializedPayload_t* payload);
	};

	/**
	 * Base class for Bridges. All implementation must inherit from it.
	 */
	class ISBridge
	{
	public:
	    ISBridge(const std::string &name);
	    virtual ~ISBridge();
	    virtual void addSubscriber(ISSubscriber *sub);
	    virtual void addFunction(const std::string &sub, const std::string &fname, userf_t func);
	    virtual void addPublisher(const std::string &sub, const std::string &funcName, ISPublisher* pub);
	    virtual ISPublisher* removePublisher(ISPublisher* pub);
	    virtual void on_received_data(const ISSubscriber *sub, SerializedPayload_t *data);
	    virtual void onTerminate() override;
	};

When you implement your ISBridge derived class, you must take in account:

- Only ISPublisher::publish is mandatory to implement.
- When your subscriber receives data, you must call on_received_data function with the data properly converted into SerializedPayload_t.
- You can override the default behaviour, but isn't recommended in general. This behaviour follows this diagram:

<p align="center"> <img src="docs/flow.png" alt="Default behaviour"/> </p>

When the subscriber calls to its method *on_received_data*, it will call all the *bridges* it belongs, calling the method *on_received_data* of each bridge.
Then the bridges will apply each respective transformation functions to the data and will call the *publish* method of each of their publishers.
All this behaviour will only occurs with the declared connectors in the XML configuration file.


**Transformation Library**:

This *optional* library must implement the transformation functions for the received data.
These *Transformation Libraries* are indicated in each connector.
The *Bridge* connector will be configured with the function to call in each case.
There is a prototype in *resource/templatelib.cpp*:

	extern "C" void USER_LIB_EXPORT transform(
		SerializedPayload_t *serialized_input,
		SerializedPayload_t *serialized_output)


For both types of libraries, there are examples in the *examples* folder of [FIROS2](https://github.com/eProsima/FIROS2/tree/master/examples) project.

May be necessary generate data types from IDL files to communicate with *Fast-RTPS*.

The **config.xml** file must be adapted to each protocol. **ISManager** will provide the parsed *properties* node inside *bridge* node to the *create_bridge* function as a vector of pairs, as defined in the *Bridge Libraries*.
The same applies for each publisher and subscriber inside *bridge* node and its *property* nodes.

Your custom publisher/subscriber must inherit from ISPublisher/ISSubscriber respectively.

The responsability of how to instantiate your bridge, publisher and/or subscriber is on your *Bridge Library*, but remember that "RTPS" publisher and subscribers will be filled automatically by ISManager with the configuration from the *participant* node of the *config.xml*.

**Data Types Library**:

These *optional* libraries must implement a single function *GetTopicType* that will return a new instance of the desired data type. The library could support multiple types or be integrated in other libraries such as the *Transformation Library*.
There is a prototype in *resource/typelib.cpp*:

	extern "C" TopicDataType* SHAPES_LIB_EXPORT GetTopicType(const char *name)
	{
		if (strncmp(name, "MyType", 6) == 0)
		{
			return new MyType();
		}
		return nullptr;
	}

<hr></hr>

## Documentation

You can access the documentation online, which is hosted on [Read the Docs](http://eprosima-fast-rtps.readthedocs.io).

<hr></hr>
## Getting Help

If you need support you can reach us by mail at `support@eProsima.com` or by phone at `+34 91 804 34 48`.
