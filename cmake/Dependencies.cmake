# Third-party dependencies via FetchContent


include(FetchContent)

FetchContent_Declare(
    minhook
    GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
    GIT_TAG v1.3.4
)

FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v3.21.12
)

FetchContent_Declare(
    icecap_contracts
    GIT_REPOSITORY https://github.com/mora9715/icecap-contracts
    GIT_TAG main
)

FetchContent_MakeAvailable(minhook protobuf icecap_contracts)
