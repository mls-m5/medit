
add_executable(
    lsp-test-client
    test1.cpp
    )

target_link_libraries(
    lsp-test-client
    PUBLIC
    lsp-client
    )

# This would require all project including this project to have clangd
# installed
#add_test(COMMAND test-client)
#enable_testing()
