cmake_minimum_required(VERSION 3.0)

project(joueurs)

find_package(Threads)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)
add_compile_options(-Wall -Wpedantic)


        set(SRCS
            joueur.cpp
            joueur_bjorn.cpp
joueur_brutal_.cpp
joueur_floki.cpp
joueur_ivar.cpp
joueur_lagertha.cpp
joueur_random.cpp
        )

        set(HEADERS
            joueur.h
            joueur_bjorn.h
joueur_brutal_.h
joueur_floki.h
joueur_ivar.h
joueur_lagertha.h
joueur_random.h
        )
    

add_library(joueurs ${SRCS} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
