Integration Services parameters and configuration
============================================

In this section are presented the main features of the *Integration Services* and information about how to configure them for creating user-customized communication bridges.


XML configuration file
----------------------

At the time of launching the *Integration Services*, it is needed to add the path of a XML configuration file. This XML has to contain all the data which defines the bridges to be created.

Each execution of the *Integration Services* permits to set up multiple independent bridges. At the same time, it is possible to launch more than one executions of the *Integration Services* simultaneously. In any case, if there exists an error in the definition of an individual bridge, this would not affect to the other of them.

The parameters of the XML configuration file are the following:

*   **rs**: label that contains a Integration Services valid configuration. It doesn't have to have an associated text but the bridge elements.

    *   **bridge**: element defining a simple bridge. There could be as "bridge" elements as the user needs. It doesn't have to have an associated text but the bridge configuration elements.

        *   **subscriber**

            *   participant: the name of the subscriber participant.
            *   domain: ID of the domain from data is received.
            *   topic: the specific topic to be subscripted.
            *   type: the name of the type to be received.
            *   partition: partition of the subscriber domain.

        *   **publisher**

            *   participant: the name of the publisher participant.
            *   domain: ID of the domain where data is published.
            *   topic: the specific topic to publish on.
            *   type: the name of the type to be published.
            *   partition: partition of the publisher domain.

        *   **transformation**: the file path with the transformation library binary.

If the communication goes from two topics which are using the same type, it is possible to perform an automatic mapping. For using it, the transformation field has to be empty. This is the case in which the user wants just rerout a message between different domains. However, if the bridge connects topics with different types, the user has to provide the needed functions to map or transform the message. In other cases, the user can aggregate its own transformation library.
In the `resource <resource>`_ folder there is a proposed template the user can fill with his application-specific configuration.


Transformation libraries
------------------------

As it was said before, there exists the option to define specific functions for making operations of routing, mapping, and message modification. This is a powerful feature that allow tasks like creating adapters between different types or even different enviroments, based on *Fast RTPS* or not. This libraries are compiled apart from the *Integration Services* and are loaded on execution time.

A simple transformation function would looks like this:

.. code-block:: c

   void transform(SerializedPayload_t *serialized_input, SerializedPayload_t *serialized_output){
       TypeA input_data;
       TypeAPubSubType input_pst;
       TypeB output_data;
       TypeBPubSubType output_pst;

       // Deserialization
       input_pst.deserialize(serialized_input, &input_data);

       // Custom routing
       output_data = input_data;

       // Serialization
       serialized_output->reserve(output_pst.m_typeSize);
       output_pst.serialize(&output_data, serialized_output);
   }

In this functions the user has to deserialize the incoming buffer, apply his conversion to the data, and then serialize it. The interfaces for each type can be created easily with the tool "fastrtpsgen", which is documented `here <http://eprosima-fast-rtps.readthedocs.io/en/latest/fastrtpsgen.html>`_

In the `resource <resource>`_ folder it could be found a proposed template which the user can use for implement his transformation, and a *CMakeLists.txt* example file to compile it.


Other applications
------------------

As an example of a real application of the *Integration Services*, *Firos2* is a *ROS2* package for communicating *Fast RTPS* and *ROS2* environments.
