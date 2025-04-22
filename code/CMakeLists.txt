cmake_minimum_required(VERSION 3.10)
project(code)

# Spécifier le standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ajouter l'exécutable
add_executable(cod "cod.cpp")

# Lier les bibliothèques nécessaires (si applicable)
# target_link_libraries(cod stdc++fs) # Décommentez si nécessaire
