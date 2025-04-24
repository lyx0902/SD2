cmake_minimum_required(VERSION 3.30)
project("SD2")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# 添加源文件
set(LR1_SOURCES
        TaskResolution/Symbol.cpp
        TaskResolution/Production.cpp
        TaskResolution/LR1Item.cpp
        TaskResolution/SyntaxAnalyzer.cpp
        include/Symbol.h
        include/Production.h
        include/LR1Item.h
        include/SyntaxAnalyzer.h
        TaskResolution/Symbol.cpp
        TaskResolution/Production.cpp
        TaskResolution/LR1Item.cpp
        TaskResolution/SyntaxAnalyzer.cpp
        include/LexicalAnalyzer.h
        TaskResolution/LexicalAnalyzer.cpp
        include/LexicalAnalysis.h
        TaskResolution/LexicalAnalysis.cpp
)

# 编译可执行文件
add_executable(Task1 TaskResolution/Task1.cpp
        include/LexicalAnalysis.h
        TaskResolution/LexicalAnalysis.cpp)
add_executable(Task2 TaskResolution/Task2.cpp ${LR1_SOURCES})