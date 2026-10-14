include(FetchContent)

# Build hidapi as a shared library
set(BUILD_SHARED_LIBS ${USE_SHARED_HIDAPI})

# Set hidapi output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

FetchContent_Declare(
    hidapi
    URL "${CMAKE_SOURCE_DIR}/third_party/hidapi-master.zip"
    # URL_HASH "SHA256=e0a2f8fdaddba3e63f7d47336a584ed416750d55083c0ee0a474e55adb6027d1"
    # DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Linux uses the HIDAPI implementation exported by TransportDLL.
    # Fetch only the headers to avoid linking a second, possibly different backend.
    FetchContent_GetProperties(hidapi)
    if(NOT hidapi_POPULATED)
        FetchContent_Populate(hidapi)
    endif()
    add_library(hidapi_headers INTERFACE)
    target_include_directories(hidapi_headers INTERFACE "${hidapi_SOURCE_DIR}/hidapi")
    add_library(hidapi::include ALIAS hidapi_headers)
else()
    FetchContent_MakeAvailable(hidapi)
endif()
