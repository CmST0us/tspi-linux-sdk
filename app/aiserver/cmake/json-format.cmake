# additional target to perform jsonformat run, requires nlohmann_json_style

add_custom_target(
    jsonformat
    COMMAND ${CMAKE_SOURCE_DIR}/src/conf/nlohmann_json_style
    ${CMAKE_SOURCE_DIR}/src/conf
)
