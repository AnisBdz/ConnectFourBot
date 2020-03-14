import sys

def read_file(filename):
    file = open(filename, mode='r')
    txt = file.read()
    file.close()
    return txt

def write_file(filename, data):
    f = open(filename, "w")
    f.write(data)
    f.close()

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

else:
    print('i don\'t know that command')
