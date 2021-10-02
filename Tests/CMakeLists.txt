# ---- Tests ----

include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(googletest
        GIT_REPOSITORY "https://github.com/google/googletest.git"
        GIT_TAG e2239ee6043f73722e7aa812a459f54a28552929
        UPDATE_DISCONNECTED YES)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(Tests tests.cpp)
target_link_libraries(Tests PRIVATE DYN::DYN gtest_main)
add_test(NAME Tests COMMAND Tests)
