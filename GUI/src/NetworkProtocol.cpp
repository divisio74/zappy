#include "../include/NetworkProtocol.hpp"
#include <iostream>
#include <sstream>

static void addResourcePlayer(Player &player, int resourceIndex)
{
    switch (resourceIndex) {
        case 0: player._food++; break;
        case 1: player._linemate++; break;
        case 2: player._deraumere++; break;
        case 3: player._sibur++; break;
        case 4: player._mendiane++; break;
        case 5: player._phiras++; break;
        case 6: player._thystame++; break;
    }
}

static void removeResourcePlayer(Player &player, int resourceIndex)
{
    switch (resourceIndex) {
        case 0: player._food--; break;
        case 1: player._linemate--; break;
        case 2: player._deraumere--; break;
        case 3: player._sibur--; break;
        case 4: player._mendiane--; break;
        case 5: player._phiras--; break;
        case 6: player._thystame--; break;
    }
}

static void addResourceTile(Tile &tile, int resourceIndex)
{
    switch (resourceIndex) {
        case 0: tile.food++; break;
        case 1: tile.linemate++; break;
        case 2: tile.deraumere++; break;
        case 3: tile.sibur++; break;
        case 4: tile.mendiane++; break;
        case 5: tile.phiras++; break;
        case 6: tile.thystame++; break;
    }
}

static void removeResourceTile(Tile &tile, int resourceIndex)
{
    switch (resourceIndex) {
        case 0: tile.food--; break;
        case 1: tile.linemate--; break;
        case 2: tile.deraumere--; break;
        case 3: tile.sibur--; break;
        case 4: tile.mendiane--; break;
        case 5: tile.phiras--; break;
        case 6: tile.thystame--; break;
    }
}

void NetworkProtocol::poll(GameState &state)
{
    std::vector<std::string> lines = receive();
 
    for (const std::string &line : lines) {
        if (!line.empty())
            parseMessage(line, state);
    }
}


void NetworkProtocol::parseMessage(const std::string &line, GameState &state)
{
    std::istringstream words(line);
    std::string cmd;
    words >> cmd;

    state.lock();

    if (cmd == "msz") {
        int width = 0;
        int height = 0;
        words >> width >> height;
        state.setSize(width, height);
    }

    else if (cmd == "bct") {
        int x = 0;
        int y = 0;
        words >> x >> y;
        if (x >= 0 && x < state.getWidth() && y >= 0 && y < state.getHeight()) {
            Tile &t = state.getTile(x, y);
            words >> t.food >> t.linemate >> t.deraumere
                  >> t.sibur >> t.mendiane >> t.phiras >> t.thystame;
            t._resources.clear();
        }
    }

    else if (cmd == "tna") {
        std::string name;
        words >> name;
        state.addTeam(name);
    }

    else if (cmd == "pnw") {
        std::string idstr;
        int x = 0;
        int y = 0;
        int orientation = 0;
        int level = 0;
        std::string teamName;
        words >> idstr >> x >> y >> orientation >> level >> teamName;

        int id = std::stoi(idstr.substr(1));

        Player newPlayer;
        newPlayer._id = id;
        newPlayer._x = x;
        newPlayer._y = y;
        newPlayer._orientation = orientation;
        newPlayer._level = level;
        newPlayer._teamName = teamName;
        newPlayer._isAlive = true;
        newPlayer._animation = "idle";
        state.addPlayer(newPlayer);
        state._playerToUpt.push_back(newPlayer._id);
    }

    else if (cmd == "ppo") {
        std::string idstr;
        int x = 0;
        int y = 0;
        int orientation = 0;
        words >> idstr >> x >> y >> orientation;
        int id = std::stoi(idstr.substr(1));

        Player *player = state.findPlayer(id);
        if (player != nullptr) {
            bool hasMoved = player->_x != x || player->_y != y;
            player->_x = x;
            player->_y = y;
            player->_orientation = orientation;
            player->_animation = hasMoved ? "walk" : "idle";
            state._playerToUpt.push_back(player->_id);
        }
    }

    else if (cmd == "plv") {
        std::string idstr;
        int level = 0;

        words >> idstr >> level;
        int id = std::stoi(idstr.substr(1));

        Player *player = state.findPlayer(id);
        if (player != nullptr)
            player->_level = level;
    }

    else if (cmd == "pin") {
        std::string idstr;
        int x = 0;
        int y = 0;

        words >> idstr;
        int id = std::stoi(idstr.substr(1));
        Player *player = state.findPlayer(id);
        if (player != nullptr) {
            words >> x >> y;
            words >> player->_food >> player->_linemate >> player->_deraumere
                >> player->_sibur >> player->_mendiane >> player->_phiras
                >> player->_thystame;
        }
    }

    else if (cmd == "pex") {
        std::string idStr;
        words >> idStr;
        int id = std::stoi(idStr.substr(1));

        std::cout << "[expulsion] joueur " << id << "\n";
    }

    else if (cmd == "pbc") {
        std::string idstr;
        std::string message;
        words >> idstr;
        std::getline(words, message);

        int id = std::stoi(idstr.substr(1));
        Player *player = state.findPlayer(id);
        std::string teamName = idstr;
        if (player != nullptr)
            teamName = player->_teamName;
        state.addBroadcast(teamName, message);
}

    else if (cmd == "pic") {
        int x = 0;
        int y = 0;
        int level = 0;
        std::string idstr;
        int id = 0;

        words >> x >> y >> level;
        std::cout << "[incantation] debut en (" << x << "," << y
            << ") niveau " << level << "\n";
        
        while (words >> idstr) {
            id = std::stoi(idstr.substr(1));
            std::cout << id << " ";
        }
        std::cout << "\n";
    }

    else if (cmd == "pie") {
        int x = 0;
        int y = 0;
        std::string result;

        words >> x >> y >> result;
        std::cout << "[incantation] fin en (" << x << "," << y
            << ") resultat " << result << "\n";
    }

    else if (cmd == "pfk") {
        std::string idStr;

        words >> idStr;
        int id = std::stoi(idStr.substr(1));

        std::cout << "[ponte] joueur " << id << " vient de pondre un oeuf\n";
    }

    else if (cmd == "pdr") {
        std::string idsrtr;
        int resourceIndex = 0;

        words >> idsrtr >> resourceIndex;
        int id = std::stoi(idsrtr.substr(1));

        Player *player = state.findPlayer(id);
        if (player != nullptr) {
            removeResourcePlayer(*player, resourceIndex);
            Tile &t = state.getTile(player->_x, player->_y);
            addResourceTile(t, resourceIndex);
            t._resources.clear();
        }
    }

    else if (cmd == "pgt") {
        std::string idStr;
        int resourceIndex = 0;

        words >> idStr >> resourceIndex;
        int id = std::stoi(idStr.substr(1));
        Player *player = state.findPlayer(id);
        if (player != nullptr) {
            addResourcePlayer(*player, resourceIndex);
            Tile &t = state.getTile(player->_x, player->_y);
            removeResourceTile(t, resourceIndex);
            t._resources.clear();
            player->_animation = "pickup";
            state._playerToUpt.push_back(player->_id);
        }
    }

    else if (cmd == "pdi") {
        std::string idstr;

        words >> idstr;
        int id = std::stoi(idstr.substr(1));

        Player *player = state.findPlayer(id);
        if (player != nullptr) {
            player->_isAlive = false;
            state._playerToUpt.push_back(player->_id);
        }
    }

    else if (cmd == "enw") {
        std::string eggidstr;
        std::string playeridstr;
        int x = 0;
        int y = 0;

        words >> eggidstr >> playeridstr >> x >> y;

        Egg newEgg;
        newEgg._id = std::stoi(eggidstr.substr(1));
        newEgg._playerId = std::stoi(playeridstr.substr(1));
        newEgg._x = x;
        newEgg._y = y;
        newEgg._isAlive = true;
        state.addEgg(newEgg);
    }

    else if (cmd == "ebo") {
        std::string eggidstr;

        words >> eggidstr;
        int id = std::stoi(eggidstr.substr(1));
        state.removeEgg(id);
    }

    else if (cmd == "edi") {
        std::string eggidstr;

        words >> eggidstr;
        int id = std::stoi(eggidstr.substr(1));
        state.removeEgg(id);
    }

    else if (cmd == "sgt") {
        int t;
        words >> t;
        state.setTimeUnit(t);
    }

    else if (cmd == "sst") {
        int t;
        words >> t;
        state.setTimeUnit(t);
    }

    else if (cmd == "seg") {
        std::string winner;
        words >> winner;
        state.setGameOver(winner);

    }

    else if (cmd == "smg") {
        std::string msg;
        std::getline(words, msg);
        std::cout << "[serveur] " << msg << "\n";
    }

    else if (cmd == "suc") {
        std::cerr << "NetworkHandler: commande inconnue rejetée\n";
    }

    else if (cmd == "sbp") {
        std::cerr << "NetworkHandler: paramètre invalide\n";
    }

    state.unlock();
}
