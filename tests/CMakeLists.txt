add_executable(UnitTests
    core/TestProperty.cc
    core/TestPlayer.cc
    core/TestSpecialCards.cc
    core/TestGame.cc
)

target_link_libraries(UnitTests PRIVATE
    GameCore
    Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(UnitTests)