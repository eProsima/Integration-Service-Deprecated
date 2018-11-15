Domain change example
=====================

In this example, we have generated a *Fast RTPS* example application using *fastrtpsgen* and then configured the publisher participant in the domain **0**, and the subscriber participant in the domain **5**.

This means that if a subscriber and a publisher are executed, there will be not communication between them, even having the same topic and type.

Executing the *Integration Service* with the provided `config.xml <config.xml>`_ file in this folder will create a bridge between the publisher (as writer) and the subscriber (as reader), and communicate both applications.
