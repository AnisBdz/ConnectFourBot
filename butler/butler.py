import sys
import os

def read_file(filename):
    file = open(filename, mode='r')
    txt = file.read()
    file.close()
    return txt

def write_file(filename, data):
    f = open(filename, "w")
    f.write(data)
    f.close()

def get_players():
    players = []
    for root, dirs, files in os.walk("../joueurs"):
        for filename in files:
            if "joueur_" in filename and ".h" in filename:
                players.append(filename[len("joueur_"):-len(".h")])
    return players

def regenerate():
    players = get_players()
    srcs = "\n".join(map(lambda e: "joueur_%s.cpp" % (e), players))
    headers = "\n".join(map(lambda e: "joueur_%s.h" % (e), players))

    cmakelists_file = read_file("CMakeLists.txt")
    cmakelists_file = cmakelists_file.replace("PLAYERS",'''
        set(SRCS
            joueur.cpp
            %s
        )

        set(HEADERS
            joueur.h
            %s
        )
    ''' % (srcs, headers))
    write_file("../joueurs/CMakeLists.txt", cmakelists_file)

    arbitre_h_file = read_file("arbitre.h")
    arbitre_h_file = arbitre_h_file.replace("ENUM_PLAYERS", "enum class player { %s };" % (",".join(map(lambda e : e.upper(), players))))
    arbitre_h_file = arbitre_h_file.replace("INCLUDE_PLAYERS", "\n".join(map(lambda e : "#include \"joueurs/joueur_%s.h\"" % (e), players) ))
    write_file("../arbitre.h", arbitre_h_file)

    arbitre_cpp_file = read_file("arbitre.cpp")
    arbitre_cpp_file = arbitre_cpp_file.replace("SWITCH_PLAYERS1", "\n".join(map(lambda e : '''
        case player::%s:
        _joueur1 = std::make_shared<%s>("%s",true);
        break;
    ''' % (e.upper(), e.capitalize(), e.capitalize()), players)))

    arbitre_cpp_file = arbitre_cpp_file.replace("SWITCH_PLAYERS2", "\n".join(map(lambda e : '''
        case player::%s:
          _joueur2 = std::make_shared<%s> ("%s",false);
          break;
    ''' % (e.upper(), e.capitalize(), e.capitalize()), players)))

    write_file("../arbitre.cpp", arbitre_cpp_file)
    os.system("cd ../build && cmake .. && make")

if len(sys.argv) < 2:
    print('Yo! tell me what to do!')
    print('butler <command> [args]')
    sys.exit()

command = sys.argv[1]


if command == 'generate_player':
    if len(sys.argv) != 3:
        print('nope')
        print('butler generate_player <player_name>')
        sys.exit()

    player_name = sys.argv[2]

    cpp_file = read_file('joueur.cpp')
    h_file   = read_file('joueur.h')

    cpp_file = cpp_file.replace("PLAYER", player_name.upper())
    cpp_file = cpp_file.replace("Player", player_name.lower().capitalize())
    cpp_file = cpp_file.replace("player", player_name.lower())

    h_file = h_file.replace("PLAYER", player_name.upper())
    h_file = h_file.replace("Player", player_name.lower().capitalize())
    h_file = h_file.replace("player", player_name.lower())

    write_file("../joueurs/joueur_%s.cpp" % (player_name), cpp_file)
    write_file("../joueurs/joueur_%s.h" % (player_name), h_file)

    regenerate()

elif command == 'regenerate':
    regenerate()
    
else:
    print('i don\'t know that command')
