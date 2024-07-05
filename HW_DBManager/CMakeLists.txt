cmake_minimum_required(VERSION 3.20) 

project(HW_DBMAnager) 

set (PostgreSQL_ADDITIONAL_VERSIONS "15")

add_executable(HW_DBMAnager main.cpp) 

add_subdirectory( "C:\\Users\\alexa\\Documents\\Netology\\Cpp_study\\libpqxx-7.9.1" libpqxx-build)
  
target_compile_features(HW_DBMAnager PRIVATE cxx_std_17) 
 
target_link_libraries(HW_DBMAnager PRIVATE pqxx)                  