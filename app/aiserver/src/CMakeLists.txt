set(CMAKE_DBUSXX_XML2CPP_TOOLS ${BR2_SDK_PATH}/bin/dbusxx-xml2cpp)
add_custom_command(OUTPUT dbus_meida_control_xml2cpp
                   COMMAND ${CMAKE_DBUSXX_XML2CPP_TOOLS}
                   dbus_media_control.xml
                   --proxy=dbus_media_control_proxy.h
                   --adaptor=dbus_media_control_adaptor.h
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbus/control
                   COMMENT "build dbus_meida_control xml2cpp head file"
)
add_custom_target(dbus_meida_control_xml2cppTask ALL DEPENDS dbus_meida_control_xml2cpp)

add_custom_command(OUTPUT dbus_dbserver_xml2cpp
                   COMMAND ${CMAKE_DBUSXX_XML2CPP_TOOLS}
                   dbus_dbserver.xml
                   --proxy=dbus_dbserver_proxy.h
                   --adaptor=dbus_dbserver_adaptor.h
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbus/database
                   COMMENT "build dbus_dbserver xml2cpp head file"
)
add_custom_target(dbus_dbserver_xml2cppTask ALL DEPENDS dbus_dbserver_xml2cpp)


add_custom_command(OUTPUT dbus_storage_manager_xml2cpp
                   COMMAND ${CMAKE_DBUSXX_XML2CPP_TOOLS}
                   dbus_storage_manager.xml
                   --proxy=dbus_storage_manager_proxy.h
                   --adaptor=dbus_storage_manager_adaptor.h
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbus/storage_manager
                   COMMENT "build dbus_storage_manager xml2cpp head file"
)
add_custom_target(dbus_storage_manager_xml2cppTask ALL DEPENDS dbus_storage_manager_xml2cpp)

set(AI_SERVER_INC
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/server/
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/thread/
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/osd/
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/drm/
    ${CMAKE_CURRENT_SOURCE_DIR}/task/
    ${CMAKE_CURRENT_SOURCE_DIR}/parse/
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/control/
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/database/
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/storage_manager/)

#     glib_2::glib_2
set(AISERVER_LIB
    dbus-1
    dbus_cpp::dbus_cpp
    ${CMAKE_THREAD_LIBS_INIT})

set(AI_SERVER_SRC aiserver.cpp ai_scene_director.cpp ai_feature_retriver.cpp ai_uvc_graph.cpp)
aux_source_directory(utils/thread AI_SERVER_SRC)
aux_source_directory(utils/drm    AI_SERVER_SRC)
aux_source_directory(task         AI_SERVER_SRC)
aux_source_directory(dbus         AI_SERVER_SRC)
aux_source_directory(dbus/control AI_SERVER_SRC)
aux_source_directory(dbus/database        AI_SERVER_SRC)
aux_source_directory(dbus/storage_manager AI_SERVER_SRC)

if (DEFINED MEDIASERVER_CONF_PREFIX)
add_definitions(-DMEDIASERVER_CONF_PREFIX=${MEDIASERVER_CONF_PREFIX})
endif()

option(DBSERVER_SUPPORT  "enbale dbserver" OFF)
if (${DBSERVER_SUPPORT})
    add_definitions(-DDBSERVER_SUPPORT)
    set(AISERVER_LIB ${AISERVER_LIB} IPCProtocol)
endif()

option(ENABLE_OSD_SERVER  "enbale osd server" OFF)
if (${ENABLE_OSD_SERVER_ON})
    find_package(FreeType2 REQUIRED)
    add_definitions(-DENABLE_OSD_SERVER)
    set(AISERVER_LIB ${AISERVER_LIB} Freetype_2::Freetype_2)
    aux_source_directory(utils/osd/ AI_SERVER_SRC)
endif()

option(ENABLE_MINILOGGER  "enbale minilogger" ON)
if (${ENABLE_MINILOGGER})
    find_package(MiniLogger REQUIRED)
    add_definitions(-DENABLE_MINILOGGER)
    set(AISERVER_LIB ${AISERVER_LIB} MiniLogger::MiniLogger)
endif()

option(ENABLE_SHM_SERVER  "enbale shm server" ON)
if (${ENABLE_SHM_SERVER})
    add_definitions(-DENABLE_SHM_SERVER)
    set(AI_SERVER_INC
        ${AI_SERVER_INC}
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/shmc/)
    aux_source_directory(utils/shmc/ AI_SERVER_SRC)
    find_package(Protobuf REQUIRED)
    if(PROTOBUF_FOUND)
        message(STATUS "protobuf library found")
        set(AISERVER_LIB ${AISERVER_LIB} protobuf-lite)
        include_directories(${PROTOBUF_INCLUDE_DIRS})
        include_directories(${CMAKE_CURRENT_BINARY_DIR})
        protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS 
                              ${CMAKE_CURRENT_SOURCE_DIR}/utils/shmc/nn_data.proto
                              ${CMAKE_CURRENT_SOURCE_DIR}/utils/shmc/uvc_data.proto)
        set(AI_SERVER_INC ${AI_SERVER_INC} ${PROTO_HDRS})
        set(AI_SERVER_SRC ${AI_SERVER_SRC} ${PROTO_SRCS} ${PROTO_HDRS})
    endif()
endif()

option(ENABLE_EXIV2_LIB  "enbale exiv2 library" OFF)
if (${ENABLE_EXIV2_LIB_ON})
    find_package(Exiv2 REQUIRED)
    add_definitions(-DENABLE_EXIV2_LIB)
    set(AISERVER_LIB ${AISERVER_LIB} Exiv2::Exiv2)
    set(AI_SERVER_INC ${AI_SERVER_INC} utils/exiv2/)
    aux_source_directory(utils/exiv2/ AI_SERVER_SRC)
endif()

option(USE_STASTERIA  "enable sensetime st_asteria" OFF)
if (${USE_STASTERIA})
    add_definitions(-DHAVE_STASTERIA)
    set(AISERVER_LIB ${AISERVER_LIB} stasteria)
    set(AI_SERVER_INC ${AI_SERVER_INC} task/sensetime/)
    #link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/../../sensetime/lib/libstdc++.so.6.0.22)
    aux_source_directory(task/sensetime/ ST_ASTERIA_SRC)
endif()

option(USE_ROCKFACE  "enbale rockface" OFF)
if (${USE_ROCKFACE})
    add_definitions(-DUSE_ROCKFACE)
endif()

option(USE_ROCKX  "enbale rockx" OFF)
if (${USE_ROCKX})
    add_definitions(-DUSE_ROCKX)
    add_definitions(-DHAVE_ROCKX)
    set(AISERVER_LIB ${AISERVER_LIB} rockx)
    set(AI_SERVER_INC ${AI_SERVER_INC} task/rockx/)
    aux_source_directory(task/rockx/ ST_ASTERIA_SRC)
endif()

# vendor & samples
set(VENDOR_STATIC vendor_static)
add_subdirectory(vendor)

set(AISERVER_LIB ${AISERVER_LIB} rockit -Wl,--whole-archive ${VENDOR_STATIC} -Wl,--no-whole-archive)

add_executable(aiserver ${AI_SERVER_SRC} ${ST_ASTERIA_SRC})
target_include_directories(aiserver PUBLIC ${AI_SERVER_INC})
target_link_libraries(aiserver ${AISERVER_LIB} PkgConfig::JSON-C)
add_dependencies(aiserver dbus_meida_control_xml2cppTask)
add_dependencies(aiserver dbus_dbserver_xml2cppTask)
add_dependencies(aiserver dbus_storage_manager_xml2cppTask)
install(TARGETS aiserver RUNTIME DESTINATION bin)
install(FILES dbus/rockchip.aiserver.control.conf DESTINATION share/dbus-1/system.d)

if (${ENABLE_OSD_SERVER})
    install(DIRECTORY resource/ DESTINATION share/aiserver FILES_MATCHING PATTERN "*.bmp")
    install(DIRECTORY resource/ DESTINATION share/aiserver FILES_MATCHING PATTERN "*.ttc")
    install(DIRECTORY resource/ DESTINATION share/aiserver FILES_MATCHING PATTERN "*.ttf")
endif()

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/aiserver.h DESTINATION include/aiserver)
