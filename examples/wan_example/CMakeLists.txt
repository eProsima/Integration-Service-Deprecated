project(wan_example)
cmake_minimum_required(VERSION 2.9)

install(FILES
    config_client.xml
    config_server.xml
    DESTINATION examples/${PROJECT_NAME}
    )